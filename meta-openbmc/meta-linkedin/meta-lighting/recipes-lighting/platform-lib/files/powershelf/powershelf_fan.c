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

#define FAN_ADDR 0x18, 0x19, 0x1a, 0x2c \

fan_info_t fan_info_cache;

void fan_show_status(fan_info_t *fan_info)
{
    int vinUndervoltFault = 0;
    int fanFault = 0;
    int overtempWarn = 0;
    int fanWarn = 0;
    int fanStatus = 1;

    for(int fan = 0 ; fan < 4 ; fan++)
    {
        if (fan_info->fan_status[fan].fanFault == 1)
        {
            fanFault = 1;
            fanStatus = 0;
            fanWarn = 1;
        }
        if (fan_info->fan_status[fan].tempCritAlarm == 1)
        {
            overtempWarn = 1;
            fanWarn = 1;
        }
        if (fan_info->fan_status[fan].tempMaxAlarm == 1)
        {
            overtempWarn = 1;
            fanWarn = 1;
        }
    }

    printf("vin_undervolt_fault: ");
    if(vinUndervoltFault == 1)
        printf("yes\n");
    else
        printf("no\n");

    printf("overtemperature_warning: ");
    if(overtempWarn == 1)
        printf("yes\n");
    else
        printf("no\n");

    printf("fan_fault: ");
    if(fanFault == 1)
        printf("yes\n");
    else
        printf("no\n");

    printf("fan_warning: ");
    if(fanWarn == 1)
        printf("yes\n");
    else
        printf("no\n");

    if(fanStatus == 1)
        printf("fan_status: ok\n");
}

int fan_device_file(uint8_t fan, char* dev_file)
{
    char      dir[DEVICE_FILENAME_SIZE];
    char      fan_dir[DEVICE_FILENAME_SIZE];
    char      hwmon[DEVICE_FILENAME_SIZE];
    char      cmd[DEVICE_FILENAME_SIZE];
    int       i2c_addrs[] = {FAN_ADDR};
    int       i2c_bus, i2c_addr;
    FILE*     in;
    struct    stat sb;
    int       ret;

    i2c_bus = 8;
    i2c_addr = i2c_addrs[fan];
    /*
     * device directory
     */
    snprintf(dir, DEVICE_FILENAME_SIZE,
             "/sys/class/i2c-adapter/i2c-%d/%d-00%x", i2c_bus, i2c_bus, i2c_addr);

    /*
     * eFuse sysfs directory
     */
    snprintf(fan_dir, DEVICE_FILENAME_SIZE, "%s/hwmon", dir);

    ret = stat(fan_dir, &sb);
    if (ret != 0)
    {
        syslog(LOG_WARNING, "fan%d failed to open hwmon/", fan);
        return -1;
    }

    if (ret == 0 && S_ISDIR(sb.st_mode))
    {
        snprintf(cmd, DEVICE_FILENAME_SIZE, "ls %s", fan_dir);
    }
    else
    {
        syslog(LOG_WARNING, "fan%d failed to open hwmon/\n", fan);
        return -1;
    }

    in=popen(cmd, "r");

    if (in == NULL) {
        syslog(LOG_WARNING, "fan%d failed to open hwmon/\n", fan);
        return -1;
    }

    fgets(hwmon, 255, in);
    hwmon[strlen(hwmon) - 1] = 0;
    pclose(in);

    snprintf(dev_file, DEVICE_FILENAME_SIZE, "%s/hwmon/%s", dir, hwmon);

    return 0;
}


int fan_poll_info(fan_info_t *fan_info)
{
    char      dir[DEVICE_FILENAME_SIZE];
    char      fan_file[DEVICE_FILENAME_SIZE];
    uint32_t  val;
    int       status_vin;
    int       i2c_bus, i2c_addr, selector_addr;
    float     vin;
    int       status;

    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    i2c_master_selector_access(FAN_BUS, FAN_MUX_ADDR);

    for(int fan_num = 0 ; fan_num < 4 ; fan_num++)
    {
        status = fan_device_file(fan_num, fan_file);
        if(status)
        {
            return -1;
        }

        memset(&fan_info->fan_status[fan_num], 0, sizeof(fan_status_t));

        snprintf(dir, DEVICE_FILENAME_SIZE, "%s/fan1_input", fan_file);
        if (i2c_read_device(dir, &val) != 0) {
            syslog(LOG_WARNING, "fan_speed: read failed\n");
        }
        else {
            fan_info->fan_speed[fan_num] = val;

            if(val < 100)
            {
                syslog(LOG_WARNING, "fan%d fault", fan_num);
                fan_info->fan_status[fan_num].fanFault = 1;
            }
        }

        /*
         * temperatures
         */
        snprintf(dir, DEVICE_FILENAME_SIZE, "%s/temp1_input", fan_file);
        if (i2c_read_device(dir, &val) != 0) {
            syslog(LOG_WARNING, "temperature1: read failed\n");
        }
        else {
            int temp = val/1000.0;
            fan_info->temperature[fan_num] = temp;
            if(temp > 80)
            {
                syslog(LOG_WARNING, "fan%d critical temperature alarm", fan_num+1);
                fan_info->fan_status[fan_num].tempCritAlarm = 1;
            }
            if(temp > 60)
            {
                syslog(LOG_WARNING, "fan%d max temperature alarm", fan_num+1);
                fan_info->fan_status[fan_num].tempMaxAlarm = 1;
            }
        }
    }

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
