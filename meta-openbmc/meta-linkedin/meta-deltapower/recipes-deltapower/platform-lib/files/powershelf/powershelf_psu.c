/*
 * Copyright 2014-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <openbmc/gpio.h>
#include <openbmc/obmc-i2c.h>
#include "powershelf.h"
#include <syslog.h>
#include <sys/mman.h>

#ifndef PSU_BUSES
#define PSU_BUSES 12, \
                  1, \
                  2, \
                  3, \
                  6, \
                  7
#endif

#ifndef PSU_ADDRS
#define PSU_ADDRS 64, \
                  64, \
                  64, \
                  64, \
                  64, \
                  64,
#endif

#ifndef PSU_SELECTOR_ADDRS
#define PSU_SELECTOR_ADDRS 0x71, \
                           0x72, \
                           0x73, \
                           0x74, \
                           0x75, \
                           0x76,
#endif

int       i2c_buses[] = {PSU_BUSES};
int       i2c_addrs[] = {PSU_ADDRS};
int       i2c_selector_addrs[] = {PSU_SELECTOR_ADDRS};

/*
 * GPIOs for PSU presence
 */
uint8_t psu_gpio [] = {
  41,    /* "GPIOF1", psu1 */
  193,   /* "GPIOY1", psu2 */
  42,    /* "GPIOF2", psu3 */
  194,   /* "GPIOY2", psu4 */
  44,    /* "GPIOF4", psu5 */
  53,    /* "GPIOG5", psu6 */
};

/*
 * GPIOs for PSU AC_OK
 *   F5, F6, F7, L5, L6, L7
 */
uint8_t psu_gpio_ac_ok [] = {
  45,    /* "GPIOF5", psu1 */
  46,    /* "GPIOF6", psu2 */
  47,    /* "GPIOF7", psu3 */
  93,    /* "GPIOL5", psu4 */
  94,    /* "GPIOL6", psu5 */
  95,    /* "GPIOL7", psu6 */
};

#define BMC_HEARTBEAT_OUT_GPIO      17
#define BMC_HEARTBEAT_IN_GPIO       192

status_info_t psu_status_table [] = {
    {PSU_VOUT_OV_FAULT,   STATUS_VOUT_ADDR,        BIT_VOUT_OV_FAULT,      "vout_overvolt_fault"},
    {PSU_VOUT_UV_FAULT,   STATUS_VOUT_ADDR,        BIT_VOUT_UV_FAULT,      "vout_undervolt_fault"},
    {PSU_IOUT_OC_FAULT,   STATUS_IOUT_ADDR,        BIT_IOUT_OC_FAULT,      "current_overcurrent_fault"},
    {PSU_IOUT_OC_WARN,    STATUS_IOUT_ADDR,        BIT_IOUT_OC_WARN,       "current_overcurrent_warn"},
    {PSU_VIN_OV_FAULT,    STATUS_INPUT_ADDR,       BIT_VIN_OV_FAULT,       "vin_overvolt_fault"},
    {PSU_VIN_UV_FAULT,    STATUS_INPUT_ADDR,       BIT_VIN_UV_WARN,        "vin_undervolt_fault"},
    {PSU_VIN_LOW_OFF,     STATUS_INPUT_ADDR,       BIT_VIN_LOW_OFF,        "psu_off_undervolt"},
    {PSU_OVER_TEMP_FAULT, STATUS_TEMPERATURE_ADDR, BIT_TEMP_OT_FAULT,      "overtemperature_fault"},
    {PSU_OVER_TEMP_WARN,  STATUS_TEMPERATURE_ADDR, BIT_TEMP_OT_WARN,       "overtemperature_warning"},
    {PSU_FAN1_FAULT,      STATUS_FAN_ADDR,         BIT_FAN1_FAULT,         "fan_fault"},
    {PSU_FAN1_OVER_RIDE,  STATUS_FAN_ADDR,         BIT_FAN1_OVER_RIDE,     "fan_override"},
};

psu_info_t psu_info_cache[MAX_PSU_NUM];

int psu_2_i2cbus[] = {12, 1, 2, 3, 6, 7};

void psu_usage ()
{
    fprintf(stderr, "Usage: psu-eeprom|psu-util [psu unit 1 - 6]\n");
    return;
}

void
reset_stats (psu_info_t *psu_info)
{
    uint8_t num_status = 0;

    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return;
    }

    if ((psu_info->psu_num < 1) || psu_info->psu_num > MAX_PSU_NUM) {
       return;
    }

    num_status = sizeof(psu_status_table)/sizeof(status_info_t);

    reset_psu_mmap_stats(psu_info->psu_num-1, psu_status_table, num_status);
}


void psu_show_status(psu_info_t *psu_info)
{
    uint8_t  val, num_status = 0;

    if (!psu_info) {
        printf("Invalid input: info for PSU NULL \n");
        return;
    }

    num_status = sizeof(psu_status_table)/sizeof(status_info_t);

    /*
     * check the status bitmap
     */
    for (int i = 0; i < num_status; i++) {
        printf("%s: %d\n", psu_status_table[i].status_desc, psu_info->status_cntr[psu_status_table[i].status]);
    }
    printf("\n");
}

int psu_to_i2cbus (int psu)
{
    if ((psu > MAX_PSU_NUM) || (psu < 1)) {
        fprintf(stderr, "invalid PSU number(1 - 6): %d\n", psu);
        return -1;
    }

    return psu_2_i2cbus[psu-1];
}

/*
 * check if PSU is present
 *   return:
 *     1:    present
 *     0:    not present
 */
int  psu_is_present (uint8_t psu)
{
    uint8_t   gpio_num;

    gpio_num = psu_gpio[psu];
    if (gpio_get(gpio_num) == GPIO_VALUE_LOW) {
        return 1;
    }
    else
        return 0;
}

/*
 * check if PSU AC input is ok
 *   return:
 *     1:    present
 *     0:    not present
 */
int  psu_is_ac_ok (uint8_t psu)
{
    uint8_t   gpio_num;

    gpio_num = psu_gpio_ac_ok[psu];
    if (gpio_get(gpio_num) == GPIO_VALUE_LOW) {
        return 1;
    }
    else
        return 0;
}

/*
 * real value for linear mode, voltage output
 */
float psu_get_realvalue_vout(int vout_mode, int vout)
{
    int     y;
    int     n;
    double  real_val;
    int     mask = 0x7FF;

    /*
     * linear data format
     * real value X = Y*(2^N)
     *    Y: 16 bit integer
     *    N: 5 bit 2's complement integer
     */
    y = vout;
    n = convert2compl_to_decimal(vout_mode, 5);

    real_val = y*pow(2.0, n);
    return real_val;
}

char* psu_get_operation_state(psu_operation_state_t state)
{
    switch(state) {
    case PSU_NOT_PRESENT:
        return "not present";
    case PSU_ON:
        return "on";
    case PSU_OFF:
        return "off";
    case PSU_NO_AC_INPUT:
        return "no ac input";
    case PSU_OTHER:
    default:
        return "other";
    }
}

bool is_psu_powerok(char* dir)
{
    char filename[DEVICE_FILENAME_SIZE];
    int  status_word;

    snprintf(filename, DEVICE_FILENAME_SIZE, "%s/status_word", dir);

    /*
     * read STATUS_POWER_GOOD & STATUS_INPUT
     */
    if (((status_word & STATUS_POWER_GOOD_MASK) == 0) &&
        ((status_word & STATUS_INPUT_MASK) == 0)) {
        return true;
    }
    else
        return false;
}

float get_psu_vout(char* dir)
{
    char filename[DEVICE_FILENAME_SIZE];
    int  vout_mode;
    int  vout_val;
    float  vout;

    snprintf(filename, DEVICE_FILENAME_SIZE, "%s/vout_mode", dir);
    if (i2c_read_device(filename, &vout_mode) != 0) {
        syslog(LOG_WARNING, "vout: failed");
        return -1;
    }

    snprintf(filename, DEVICE_FILENAME_SIZE, "%s/vout", dir);
    if (i2c_read_device(filename, &vout_val) != 0) {
        syslog(LOG_WARNING, "vout: failed\n");
        return -1;
    }

    vout = psu_get_realvalue_vout(vout_mode, vout_val);
    return vout;
}

int psu_poll_info(psu_info_t *psu_info)
{
    char      psu_file[DEVICE_FILENAME_SIZE];
    char      dir[DEVICE_FILENAME_SIZE];
    uint32_t  val;
    int       status_vout;
    int       status_iout;
    int       status_vin;
    int       i2c_bus, i2c_addr, selector_addr;
    int       read_status = -1;
    int       psu;
    int       i2c_buses[] = {PSU_BUSES};
    int       i2c_addrs[] = {PSU_ADDRS};
    float     vin,  iin, vout, pin;

    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    if (psu_info->psu_num < 1 || psu_info->psu_num > MAX_PSU_NUM) {
        syslog(LOG_WARNING, "Invalid input: PSU number %d,  range: 1 - 6 %d \n", psu_info->psu_num);
        return -1;
    }

    psu = psu_info->psu_num - 1;

    i2c_bus = i2c_buses[psu];
    i2c_addr = i2c_addrs[psu];
    selector_addr = i2c_selector_addrs[psu];

    i2c_master_selector_access(i2c_bus, selector_addr);

    snprintf(dir, DEVICE_FILENAME_SIZE,
             "/sys/class/i2c-adapter/i2c-%d/%d-00%x", i2c_bus, i2c_bus, i2c_addr);

    /*
     * PSU number: 1 - 6
     */
    psu_info->i2c_bus = i2c_bus;
    psu_info->i2c_addr = i2c_addr;

    /*
     * Get the GPIO pin for PSU presence
     */
    if (psu_is_present(psu) == 0) {
        psu_info->operation_state = PSU_NOT_PRESENT;
        syslog(LOG_WARNING, "PSU%d not present", psu_info->psu_num);
        /*
         * PSU is not present
         */
        return -1;
    }

    /*
     * Check PSU AC Input OK
     */
    if (psu_is_ac_ok(psu) == 0) {
        psu_info->operation_state = PSU_NO_AC_INPUT;
        syslog(LOG_WARNING, "PSU%d AC input not ok", psu_info->psu_num);
        /*
         * PSU is no AC input
         */
        return -1;
    }

    if (read_value_linear(dir, "vin", &vin) == 0) {
        psu_info->volt_input = vin;
    }

    if (read_value_linear(dir, "iin", &iin) == 0) {
        psu_info->current_input = iin;
    }

    if (read_value_linear(dir, "pin", &pin) == 0) {
        psu_info->power_input = pin;
    }

    psu_info->volt_out = get_psu_vout(dir);

    /*
     * PSU operation
     */
    snprintf(psu_file, DEVICE_FILENAME_SIZE, "%s/operation", dir);
    if (i2c_read_device(psu_file, &val) != 0) {
        return -1;
    }

    /*
     * PSU is on or off
     */
    if (val == DEVICE_OPERATION_OFF) {
        psu_info->operation_state = PSU_OFF;
    }
    else if (val == DEVICE_OPERATION_ON) {
        psu_info->operation_state = PSU_ON;
    }
    else
        psu_info->operation_state = PSU_OTHER;

    /*
     * PSU fan speed
     */
    snprintf(psu_file, DEVICE_FILENAME_SIZE, "%s/fan1_speed", dir);
    if (i2c_read_device(psu_file, &val) != 0) {
        syslog(LOG_WARNING, "fan_speed: read failed\n");
    }
    else {
        psu_info->fan_speed = val;
    }

    /*
     * PSU temperatures
     */
    snprintf(psu_file, DEVICE_FILENAME_SIZE, "%s/temperature1", dir);
    if (i2c_read_device(psu_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature1: read failed\n");
    }
    else {
        psu_info->temperature[0] = val;
    }

    snprintf(psu_file, DEVICE_FILENAME_SIZE, "%s/temperature2", dir);
    if (i2c_read_device(psu_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature2: read failed\n");
    }
    else {
        psu_info->temperature[1] = val;
    }

    snprintf(psu_file, DEVICE_FILENAME_SIZE, "%s/temperature3", dir);
    if (i2c_read_device(psu_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature3: read failed\n", psu_file);
    }
    else {
        psu_info->temperature[2] = val;
    }

    /*
     * get PSU status: faults and warnings
     */
    read_status = psu_get_status(psu_info);
    if (read_status == 0) {
        psu_clear_status(psu_info);
    }

    return 0;
}

int
psu_get_info (psu_info_t *psu_info)
{
    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    if ((psu_info->psu_num < 1) || psu_info->psu_num > MAX_PSU_NUM) {
       return -1;
    }

    get_mmap_info((int*)&psu_info_cache, FILE_PSU, sizeof(psu_info_cache));
    memcpy(psu_info, &psu_info_cache[psu_info->psu_num - 1], sizeof(psu_info_t));
    return 0;
}

int psu_save_info(psu_info_t *psu_info)
{
    if (!psu_info) {
        printf("Invalid input:  NULL \n");
        return -1;
    }

    memcpy(psu_info, &psu_info_cache[psu_info->psu_num - 1], sizeof(psu_info_t));
}

int psu_get_status(psu_info_t *psu_info)
{
    int val, fd, retry, num_faults;
    int status = -1;

    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return status;
    }

    num_faults = sizeof(psu_status_table) / sizeof(psu_status_table[0]);

    fd = i2c_open(psu_info->i2c_bus, psu_info->i2c_addr);
    if (fd < 0) {
        syslog(LOG_WARNING, "PSU%d failed to open i2c\n", psu_info->psu_num);
        return status;
    }

    for (int i = 0; i < num_faults; i++) {
        val = i2c_smbus_read_byte_data(fd, psu_status_table[i].status_i2c_addr);
        retry = 0;
        while ((retry < MAX_RETRIES) && (val < 0)) {
            usleep(1000);
            val = i2c_smbus_read_byte_data(fd, psu_status_table[i].status_i2c_addr);
            if (val < 0)
                retry++;
            else
                break;
        }

        if (val == -1) {
            syslog(LOG_WARNING, "PSU%d %s read failed\n",
                   psu_info->psu_num, psu_status_table[i].status_desc);

            // even if one read fails, discard current read for psu
            for (int fault = 0; i < num_faults ; fault++) {
                psu_info->status_cntr[psu_status_table[i].status] = 0;
            }

            close(fd);
            return status;
        }
        else {
            if (val & psu_status_table[i].bitmask) {
                syslog(LOG_WARNING, "psu%d %s", psu_info->psu_num, psu_status_table[i].status_desc);
                psu_info->status_cntr[psu_status_table[i].status] += 1;
                status = 0;
            }
        }
    }

    close(fd);
    return status;
}

int psu_clear_status(psu_info_t *psu_info)
{
    int val, fd, retry;

    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    fd = i2c_open(psu_info->i2c_bus, psu_info->i2c_addr);
    if (fd < 0) {
        syslog(LOG_WARNING, "PSU%d failed to open for write i2c\n", psu_info->psu_num);
        return -1;
    }

    psu_clear_status_info_t psu_clr_stat;
    psu_clr_stat.i2c_addr = psu_info->i2c_addr << 1;
    psu_clr_stat.reg = CLEAR_FAULTS_REG;

    uint8_t crc = crc8(&psu_clr_stat, sizeof(psu_clear_status_info_t));

    val = i2c_smbus_write_byte_data(fd, CLEAR_FAULTS_REG, crc);
    retry = 0;
    while ((retry < MAX_RETRIES) && (val < 0)) {
        usleep(1000);
        val = i2c_smbus_write_byte_data(fd, CLEAR_FAULTS_REG, crc);
        if (val < 0)
            retry++;
        else
            break;
    }

    if (val != 0) {
        syslog(LOG_WARNING, "PSU%d clear status failed \n", psu_info->psu_num);
    }
    else {
        syslog(LOG_WARNING, "PSU%d clear status success \n", psu_info->psu_num);
    }

    close(fd);
    return 0;
}

int psu_aggregate_info(psu_aggregate_info_t *psu_aggr_info)
{
    if (!psu_aggr_info) {
        syslog(LOG_WARNING, "Invalid Input:  psu_aggr_info is NULL \n");
        return -1;
    }

    for (int i = 0; i < MAX_PSU_NUM_ASIDE; i++) {
        /*
         *  Side A total power
         */
        psu_aggr_info->side_a_power += psu_info_cache[i].power_input;
        psu_aggr_info->total_power += psu_info_cache[i].power_input;
        psu_aggr_info->total_current += psu_info_cache[i].current_input;
    }

    for (int i = MAX_PSU_NUM_ASIDE; i < MAX_PSU_NUM; i++) {
        /*
         *  Side B total power
         */
        psu_aggr_info->side_b_power += psu_info_cache[i].power_input;
        psu_aggr_info->total_power += psu_info_cache[i].power_input;
        psu_aggr_info->total_current += psu_info_cache[i].current_input;
    }

    return 0;
}
