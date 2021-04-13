/*
 * Copyright 2019-present LinkedIn. All Rights Reserved.
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

fan_info_t fan_info_cache;

status_info_t fan_status_table [] = {
    {FAN_VIN_UV_FAULT,    STATUS_WORD_ADDR,  FAN_BIT_VIN_UV_FAULT,    "vin_undervolt_fault"},
    {FAN_OVER_TEMP_WARN,  STATUS_WORD_ADDR,  FAN_BIT_TEMP_WARNING,    "overtemperature_warning"},
    {FAN_FAN1_FAULT,      STATUS_FAN_ADDR,   FAN_BIT_SPEED_FAIL,      "fan_fault"},
    {FAN_FAN1_WARN,       STATUS_FAN_ADDR,   FAN_BIT_SPEED_WARNING,   "fan_warning"},
};

void fan_show_status(fan_info_t *fan_info)
{
    uint8_t    num_status = 0;

    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input NULL \n");
        return;
    }

    num_status = sizeof(fan_status_table)/sizeof(status_info_t);

    /*
     * check the status bitmap
     */
    for (int i = 0; i < num_status; i++) {
        if (fan_info->fan_status & (1 << fan_status_table[i].status)) {
            printf("%s: yes\n", fan_status_table[i].status_desc);
        }
        else {
            printf("%s: no\n", fan_status_table[i].status_desc);
        }
    }

    if (fan_info->fan_status == 0) {
        printf("fan_status: ok\n");
    }
}

int fan_poll_info(fan_info_t *fan_info)
{
    char      fan_file[DEVICE_FILENAME_SIZE];
    char      dir[DEVICE_FILENAME_SIZE];
    uint32_t  val;
    int       status_vin;
    int       i2c_bus, i2c_addr, selector_addr;
    float     vin;

    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    i2c_bus = FAN_BUS;
    i2c_addr = FAN_CTRL_ADDR;
    selector_addr = FAN_MUX_ADDR;

    i2c_master_selector_access(i2c_bus, selector_addr);

    snprintf(dir, DEVICE_FILENAME_SIZE,
             "/sys/class/i2c-adapter/i2c-%d/%d-00%x", i2c_bus, i2c_bus, i2c_addr);

    fan_info->i2c_bus = i2c_bus;
    fan_info->i2c_addr = i2c_addr;

    /*
     * 12V fan supply voltage in millivolts
     */
    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/vin", dir);
    if (i2c_read_device(fan_file, &val)) {
        syslog(LOG_WARNING, "fan read failed: vin\n");
    } else {
        fan_info->volt_input = (float) val/1000.0;
    }

    /*
     * fan speed
     */
    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/fan1_speed", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "fan_speed: read failed\n");
    }
    else {
        fan_info->fan_speed[0] = val;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/fan2_speed", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "fan_speed: read failed\n");
    }
    else {
        fan_info->fan_speed[1] = val;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/fan3_speed", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "fan_speed: read failed\n");
    }
    else {
        fan_info->fan_speed[2] = val;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/fan4_speed", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "fan_speed: read failed\n");
    }
    else {
        fan_info->fan_speed[3] = val;
    }

    /*
     * temperatures
     */
    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/temperature1", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature1: read failed\n");
    }
    else {
        fan_info->temperature[0] = (float) val/10.0;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/temperature2", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature2: read failed\n");
    }
    else {
        fan_info->temperature[1] = (float) val/10.0;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/temperature3", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature3: read failed\n", fan_file);
    }
    else {
        fan_info->temperature[2] = (float) val/10.0;
    }

    snprintf(fan_file, DEVICE_FILENAME_SIZE, "%s/temperature4", dir);
    if (i2c_read_device(fan_file, &val) != 0) {
        syslog(LOG_WARNING, "temperature3: read failed\n", fan_file);
    }
    else {
        fan_info->temperature[3] = (float) val/10.0;
    }

    fan_get_status(fan_info);
    return 0;
}

int
fan_get_info (fan_info_t *fan_info)
{
    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    get_mmap_info((int*)&fan_info_cache, FILE_FAN, sizeof(fan_info_cache));
    memcpy(fan_info, &fan_info_cache, sizeof(fan_info_t));
    return 0;
}

int fan_get_status(fan_info_t *fan_info)
{
    int  val, fd, retry, num_faults = 0;

    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    fan_info->fan_status = 0;
    num_faults = sizeof(fan_status_table) / sizeof(fan_status_table[0]);

    fd = i2c_open(fan_info->i2c_bus, fan_info->i2c_addr);
    if (fd < 0) {
        syslog(LOG_WARNING, "fan  failed to open i2c\n");
        return -1;
    }

    for (int i = 0; i < num_faults; i++) {
        val = i2c_smbus_read_byte_data(fd, fan_status_table[i].status_i2c_addr);
        retry = 0;
        while ((retry < MAX_RETRIES) && (val < 0)) {
            usleep(1000);
            val = i2c_smbus_read_byte_data(fd, fan_status_table[i].status_i2c_addr);
            if (val < 0)
                retry++;
            else
                break;
        }

        if (val == -1) {
            syslog(LOG_WARNING, "fan controller %s read failed\n",
                   fan_status_table[i].status_desc);
        }
        else {
            if (val & fan_status_table[i].bitmask) {
                syslog(LOG_WARNING, "Fan controller %s", fan_status_table[i].status_desc);
                fan_info->fan_status |= 1 << fan_status_table[i].status;
            }
        }
    }

    close(fd);
    return 0;
}
