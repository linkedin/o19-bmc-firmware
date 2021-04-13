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
#include <errno.h>
#include <string.h>
#include <facebook/powershelf.h>


void fan_show_info(fan_info_t *fan_info)
{
    if (!fan_info) {
        printf("Invalid input: NULL \n");
        return;
    }

    for (int i = 0; i < MAX_FAN_NUM; i++) {
        printf("fan%d speed: %d RPM\n", i + 1, fan_info->fan_speed[i]);
    }

    for (int i = 0; i < MAX_TEMP_NUM; i++) {
        printf("temperature%i: %.2f C\n", i + 1, fan_info->temperature[i]);
    }
}

int main(int argc, char **argv)
{
    int             i2c_bus, i2c_addr;
    fan_info_t      *fan_info;

    fan_info = (fan_info_t *) malloc(sizeof(fan_info_t));

    if (!fan_info) {
        printf("Failed to alloc memory for fan_info\n");
        return -1;
    }

    if (argc < 1) {
        return -1;
    }

    /*
     * get fan informatin from memory mapped file
     */
    fan_get_info(fan_info);
    fan_show_info(fan_info);
    fan_show_status(fan_info);

    free(fan_info);

    return 0;
}
