/*
 * Copyright 2018-present Linkedin. All Rights Reserved.
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <syslog.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <openbmc/gpio.h>
#include <openbmc/obmc-i2c.h>
#include "powershelf.h"
#include <sys/mman.h>

#ifndef EFUSE_BUSES
#define EFUSE_BUSES 0,0,0,0,0,0,0,0,0,0,0,0, \
                    0,0,0,0,0,0,0,0,0,0,0,0, \
                    4,4,4,4,4,4,4,4,4,4,4,4, \
                    4,4,4,4,4,4,4,4,4,4,4,4, \
                    4,4
#endif

#ifndef EFUSE_ADDRS
/*
 * HW P2 version
 */
#define EFUSE_ADDRS 0x12,0x13,0x10,0x11,0x46,0x47,0x45, \
                  0x44,0x43,0x42,0x41,0x40,0x14,0x15,0x16,0x17,0x50, \
                  0x51,0x53,0x52,0x55,0x54,0x57,0x56, \
                  0x12,0x13,0x10,0x11,0x46,0x47,0x45, \
                  0x44,0x43,0x42,0x41,0x40,0x14,0x15,0x16,0x17,0x50, \
                  0x51,0x53,0x52,0x55,0x54,0x57,0x56, \
                  0x59, 0x5a
#endif

#define      VIN_REG         0x8b
#define      VOUT_REG        0x88

efuse_info_t efuse_info_cache[MAX_EFUSE_NUM];

status_info_t efuse_status_table [] = {
    /* status word */
    {EFUSE_CML_FAULT, STATUS_WORD_ADDR, BIT_CML_FAULT, "status_word cml error"},
    {EFUSE_TEMPERATURE_WARN_FAULT, STATUS_WORD_ADDR,
     BIT_TEMPERATURE_WARN_FAULT, "temperature fault or warning"},
    {EFUSE_UNIT_OFF, STATUS_WORD_ADDR, BIT_UNIT_OFF, "MOSFET switched off"},
    {EFUSE_POWER_GOOD_NEGATED, STATUS_WORD_ADDR, BIT_POWER_GOOD_NEGATED, "status_word power not good"},
    {EFUSE_MFR_SPECIFIC_ERROR, STATUS_WORD_ADDR, BIT_MFR_SPECIFIC_ERROR, "manufacture specific error"},
    {EFUSE_VIN_IIN_FAULT, STATUS_WORD_ADDR, BIT_VIN_IIN_FAULT, "status_word vin or iin fault"},
    {EFUSE_VOUT_FAULT_WARN, STATUS_WORD_ADDR, BIT_VOUT_FAULT_WARN, "status_word vout fault or warn"},

    /* status vout */
    {EFUSE_VOUT_UV_WARN, STATUS_VOUT_ADDR, BIT_VOUT_UV_WARN, "vout_undervolt_warn"},

    /* status_input */
    {EFUSE_VIN_OV_FAULT, STATUS_INPUT_ADDR,  BIT_VIN_OV_FAULT, "vin_overvolt_fault"},
    {EFUSE_VIN_OV_WARN,  STATUS_INPUT_ADDR,  BIT_VIN_OV_WARN, "vin_overvolt_warn"},
    {EFUSE_VIN_UV_FAULT, STATUS_INPUT_ADDR,  BIT_VIN_UV_FAULT, "vin_undervolt_fault"},
    {EFUSE_VIN_UV_WARN,  STATUS_INPUT_ADDR,  BIT_VIN_UV_WARN, "vin_undervolt_warn"},
    {EFUSE_IIN_OC_FAULT, STATUS_INPUT_ADDR,  BIT_IIN_OC_FAULT, "current_overcurrent_fault"},
    {EFUSE_IIN_OC_WARN,  STATUS_INPUT_ADDR,  BIT_IIN_OC_WARN, "current_overcurrent_warn"},
    {EFUSE_PIN_OP_WARN,  STATUS_INPUT_ADDR,  BIT_PIN_OP_WARN, "powerin_overpower_warn"},

    /* status_temperature */
    {EFUSE_TEMP_OT_FAULT, STATUS_TEMPERATURE_ADDR, BIT_TEMP_OT_FAULT, "overtemperature_fault"},
    {EFUSE_TEMP_OT_WARN,  STATUS_TEMPERATURE_ADDR, BIT_TEMP_OT_WARN, "overtemperature_warn"},

    /* mfr status */
    {EFUSE_CIRCUIT_BREAKER_FAULT, STATUS_MFR_SPECIFIC, BIT_CIRCUIT_BREAKER_FAULT, "circuit_breaker_fault"},
    {EFUSE_MOSFET_FAULT,  STATUS_MFR_SPECIFIC, BIT_MOSFET_FAULT, "ext_mosfet_fault"},
};

int efuse_i2c_selector_addr(uint8_t bus)
{
    if(bus == I2C_MASTER_EFUSE_BUS_0)
        return I2C_MASTER_SELETOR_ADDR_0;
    else if (bus == I2C_MASTER_EFUSE_BUS_4)
        return I2C_MASTER_SELETOR_ADDR_4;
    else
        return -1;
}

int efuse_to_i2cbus(uint8_t efuse)
{
    int       i2c_buses[] = {EFUSE_BUSES};

    if ((efuse < 1) || efuse > MAX_EFUSE_NUM) {
       return -1;
    }

    return i2c_buses[efuse - 1];
}

int efuse_install_driver(uint8_t i2c_bus, uint8_t i2c_addr)
{
    char      cmd[DEVICE_FILENAME_SIZE];

    snprintf(cmd, DEVICE_FILENAME_SIZE,
             "echo 0x%x > /sys/class/i2c-dev/i2c-%d/device/delete_device", i2c_addr, i2c_bus);
    system(cmd);
    snprintf(cmd, DEVICE_FILENAME_SIZE,
             "echo lm25066 0x%x > /sys/class/i2c-dev/i2c-%d/device/new_device", i2c_addr, i2c_bus);
    system(cmd);
    return 0;
}

int efuse_device_file(uint8_t efuse, char* dev_file)
{
    char      eFuse_file[DEVICE_FILENAME_SIZE];
    char      dir[DEVICE_FILENAME_SIZE];
    char      eFuse_dir[DEVICE_FILENAME_SIZE];
    char      hwmon[DEVICE_FILENAME_SIZE];
    char      cmd[DEVICE_FILENAME_SIZE];
    char      info_dir[DEVICE_FILENAME_SIZE];
    int       i2c_addrs[] = {EFUSE_ADDRS};
    int       i2c_bus, i2c_addr;
    FILE*     in;
    struct    stat sb;
    int       ret, retry;

    i2c_bus = efuse_to_i2cbus(efuse);
    i2c_addr = i2c_addrs[efuse-1];

    /*
     * device directory
     */
    snprintf(dir, DEVICE_FILENAME_SIZE,
             "/sys/class/i2c-adapter/i2c-%d/%d-00%x", i2c_bus, i2c_bus, i2c_addr);

    /*
     * eFuse sysfs directory
     */
    snprintf(eFuse_dir, DEVICE_FILENAME_SIZE, "%s/hwmon", dir);

    ret = stat(eFuse_dir, &sb);
    while ((retry < 10) && (ret != 0)) {
        usleep(10000);
        syslog(LOG_WARNING, "efuse%d failed to open hwmon/ retry: %d", efuse, retry);
        efuse_install_driver(i2c_bus, i2c_addr);
        ret = stat(eFuse_dir, &sb);
        if (ret != 0)
           retry++;
        else
            break;
    }

    if (ret == 0 && S_ISDIR(sb.st_mode))
    {
        snprintf(cmd, DEVICE_FILENAME_SIZE, "ls %s", eFuse_dir);
    }
    else
    {
        syslog(LOG_WARNING, "efuse%d failed to open hwmon/\n", efuse);
        return -1;
    }

    in=popen(cmd, "r");

    if (in == NULL) {
        syslog(LOG_WARNING, "efuse%d failed to open hwmon/\n", efuse);
        return -1;
    }

    fgets(hwmon, 255, in);
    hwmon[strlen(hwmon) - 1] = 0;
    pclose(in);

    snprintf(dev_file, DEVICE_FILENAME_SIZE, "%s/hwmon/%s", dir, hwmon);

    return 0;
}

int
efuse_poll_info (efuse_info_t *efuse_info)
{
    int       val, in;
    char      eFuse_file[DEVICE_FILENAME_SIZE];
    char      dir[DEVICE_FILENAME_SIZE];
    float     vin=0.0, vout1=0.0, iin=0.0, pin=0.0, temp1=0.0;
    uint32_t  curr_val = 0;
    int       i2c_addrs[] = {EFUSE_ADDRS};
    int       efuse_idx;
    int       i2c_bus, i2c_addr, selector_addr;
    int       ret = 0;

    if (!efuse_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    if ((efuse_info->efuse_num < 1) || efuse_info->efuse_num > MAX_EFUSE_NUM) {
       syslog(LOG_WARNING, "efuse number %d out of range", efuse_info->efuse_num);
       return -1;
    }

    efuse_idx = efuse_info->efuse_num - 1;
    i2c_bus = efuse_to_i2cbus(efuse_info->efuse_num);
    i2c_addr = i2c_addrs[efuse_idx];

    efuse_info->i2c_bus = i2c_bus;
    efuse_info->i2c_addr = i2c_addr;

    selector_addr = efuse_i2c_selector_addr(i2c_bus);
    i2c_master_selector_access(i2c_bus, selector_addr);

    /*
     * device directory
     */
    snprintf(dir, DEVICE_FILENAME_SIZE,
             "/sys/class/i2c-adapter/i2c-%d/%d-00%x", i2c_bus, i2c_bus, i2c_addr);

    if (efuse_device_file(efuse_info->efuse_num, dir) == -1) {
        syslog(LOG_WARNING, "efuse%d failed to find the hwmon path", efuse_info->efuse_num);
        return -1;
    }

    if (i2c_read_device_float(dir, "in1_input", &vin) != 0) {
        syslog(LOG_WARNING, "efuse%d failed to read vin", efuse_info->efuse_num);
    }
    efuse_info->volt_input = vin;

    /*
     * vout1
     */
    if (i2c_read_device_float(dir, "in3_input", &vout1) != 0) {
        syslog(LOG_WARNING, "efuse%d failed to read vout", efuse_info->efuse_num);
    }
    efuse_info->volt_out = vout1;

    /*
     * pin - power
     */
    if (i2c_read_device_float(dir, "power1_input", &pin) != 0) {
        syslog(LOG_WARNING, "efuse%d failed to read power_input", efuse_info->efuse_num);
    }
    efuse_info->power_input = pin;

    /*
     * temp1
     */
    if (i2c_read_device_float(dir, "temp1_input", &temp1) != 0) {
        syslog(LOG_WARNING, "efuse%d failed to read temperature", efuse_info->efuse_num);
        return -1;
    }
    efuse_info->temperature = temp1;

    /*
     * iin - current
     */
    if (i2c_read_device_float(dir, "curr1_input", &iin) != 0) {
        syslog(LOG_WARNING, "efuse%d failed to read current", efuse_info->efuse_num);
    }
    efuse_info->current_input = iin;

    /*
     * eFuse status: faults and warnings
     */
    ret = efuse_get_status(efuse_info);

    return ret;
}

void
efuse_get_mmap_info ()
{
    get_mmap_info((int*)&efuse_info_cache[0], FILE_EFUSE, sizeof(efuse_info_cache));
    return;
}

int
efuse_get_info (efuse_info_t *efuse_info)
{
    if ((efuse_info->efuse_num < 1) || efuse_info->efuse_num > MAX_EFUSE_NUM) {
       return -1;
    }

    memcpy(efuse_info, &efuse_info_cache[efuse_info->efuse_num - 1], sizeof(efuse_info_t));
    return 0;
}

void efuse_show_status(efuse_info_t *efuse_info)
{
    uint8_t num_status = 0;

    if (!efuse_info) {
        syslog(LOG_WARNING, "Invalid input: NULL \n");
        return;
    }

    num_status = sizeof(efuse_status_table)/sizeof(status_info_t);

    /*
     * check the status bitmap
     */
    for (int i = 0; i < num_status; i++) {
        if (efuse_info->efuse_status & (1 << efuse_status_table[i].status)) {
            printf("%s: yes\n", efuse_status_table[i].status_desc);
        }
        else {
            printf("%s: no\n", efuse_status_table[i].status_desc);
        }
    }
    printf("\n");
}

char* efuse_get_operation_state(efuse_operation_state_t state)
{
    switch(state) {
    case EFUSE_ON:
        return "on";
    case EFUSE_OFF:
        return "off";
    default:
        return "unknown";
    }
}

int efuse_get_status(efuse_info_t *efuse_info)
{
    int    fd, val, retry, num_faults;
    int    i2c_addrs[] = {EFUSE_ADDRS};
    int    efuse_idx;
    int    i2c_bus, i2c_addr;

    if (!efuse_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    efuse_idx = efuse_info->efuse_num - 1;
    i2c_bus = efuse_to_i2cbus(efuse_info->efuse_num);
    i2c_addr = i2c_addrs[efuse_idx];

    efuse_info->i2c_bus = i2c_bus;
    efuse_info->i2c_addr = i2c_addr;

    efuse_info->efuse_status = 0;

    fd = i2c_open(i2c_bus, i2c_addr);
    if (fd < 0) {
        syslog(LOG_WARNING, "efuse%d Fail to open i2c", efuse_info->efuse_num);
        return -1;
    }

    /*
     * read operation register
     */
    val = i2c_smbus_read_byte_data(fd, OPERATION_REG);
    while ((retry < MAX_RETRIES) && (val < 0)) {
        usleep(10000);
        val = i2c_smbus_read_byte_data(fd, OPERATION_REG);
        if (val < 0)
           retry++;
        else
            break;
    }

    if (val < 0) {
        syslog(LOG_WARNING, "efuse%d failed to read operation register", efuse_info->efuse_num);
        close(fd);
        return -1;
    }
    else {
       if (val == DEVICE_OPERATION_ON) {
            efuse_info->state = EFUSE_ON;
        }
        else if (val == DEVICE_OPERATION_OFF) {
            efuse_info->state = EFUSE_OFF;
        }
        else
            efuse_info->state = EFUSE_UNKNOWN;
    }

    num_faults = sizeof(efuse_status_table) / sizeof(efuse_status_table[0]);

    for (int i = 0; i < num_faults; i++) {
        val = i2c_smbus_read_byte_data(fd, efuse_status_table[i].status_i2c_addr);
        retry = 0;
        while ((retry < MAX_RETRIES) && (val < 0)) {
            usleep(1000);
            val = i2c_smbus_read_byte_data(fd, efuse_status_table[i].status_i2c_addr);
            if (val < 0)
                retry++;
            else
                break;
        }

        if (val >= 0) {
           if (val & efuse_status_table[i].bitmask) {
               syslog(LOG_WARNING, "efuse%d %s", efuse_info->efuse_num, efuse_status_table[i].status_desc);
               efuse_info->efuse_status |= 1 << efuse_status_table[i].status;
            }
        }
        else {
            syslog(LOG_WARNING, "efuse%d failed to read 0x%x", efuse_info->efuse_num, efuse_status_table[i].status_i2c_addr);
            close(fd);
            return -1;
        }
    }

    /*
     * eFuse trip state
     */
    if (efuse_info->efuse_status & BIT_IIN_OC_FAULT) {
         efuse_info->efuse_tripped = 1;
         syslog(LOG_WARNING, "efuse%d efuse tripped\n", efuse_info->efuse_num);
    }
    else
        efuse_info->efuse_tripped = 0;

    close(fd);
    return 0;
}

/*
 * op:   1  on:    DEVICE_OPERATION_ON
 *       0  off:   DEVICE_OPERATION_OFF
 */
int efuse_operation (uint8_t efuse_num, uint8_t op)
{
    int       fd, ret, retry = 0;
    int       i2c_addrs[] = {EFUSE_ADDRS};
    int       i2c_bus, i2c_addr, selector_addr;
    int       op_value;

    if ((efuse_num < 1) || efuse_num > MAX_EFUSE_NUM) {
       return -1;
    }

    i2c_bus = efuse_to_i2cbus(efuse_num);
    i2c_addr = i2c_addrs[efuse_num - 1];

    selector_addr = efuse_i2c_selector_addr(i2c_bus);
    i2c_master_selector_access(i2c_bus, selector_addr);

    fd = i2c_open(i2c_bus, i2c_addr);
    if (fd < 0) {
        syslog(LOG_WARNING, "efuse%d Fail to open i2c", efuse_num);
        close(fd);
        return -1;
    }

    if (op == 1)
        op_value = DEVICE_OPERATION_ON;

    else if (op == 0)
        op_value = DEVICE_OPERATION_OFF;
    else {
        syslog(LOG_WARNING, "invalid op value");
        close(fd);
        return -1;
    }

    /*
     * Write to operation register
     */
    ret = i2c_smbus_write_byte_data(fd, OPERATION_REG, op_value);
    if (ret < 0) {
        syslog(LOG_WARNING, "efuse%d Fail to write %d to operation register", efuse_num, op_value);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int efuse_reset(uint8_t efuse_num)
{
    char         eFuse_file[DEVICE_FILENAME_SIZE];
    char         dir[DEVICE_FILENAME_SIZE];
    efuse_info_t *efuse_info;
    int          fd, ret, retry = 0;

    efuse_info = malloc(sizeof(efuse_info_t));
    if (!efuse_info) {
        syslog(LOG_WARNING, "efuse memory alloc failed");
        return -1;
    }

    efuse_info->efuse_num = efuse_num;

    /*
     * eFuse status: faults and warning
     */
    efuse_poll_info(efuse_info);

    if ((efuse_info->efuse_tripped == 1) ||
        (efuse_info->volt_out < 1.0)) {
        /*
         * Write "off"
         */
        if (efuse_operation(efuse_num, 0) == -1) {
            syslog(LOG_WARNING, "Failed to turn off efuse%d for reset", efuse_num);
            return OP_OFF_ERROR;
        }

        /*
         * Write "on"
         */
        if (efuse_operation(efuse_num, 1) == -1) {
            syslog(LOG_WARNING, "Failed to turn on efuse%d for reset", efuse_num);
            return OP_ON_ERROR;
        }
    }
    else {
        syslog(LOG_WARNING, "efuse%d is normal, can't be reset", efuse_num);
        return OP_RESET_ERROR;
    }

    return OP_NO_ERROR;
}
