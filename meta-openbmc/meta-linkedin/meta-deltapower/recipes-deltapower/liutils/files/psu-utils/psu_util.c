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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <facebook/powershelf.h>
#include <syslog.h>

void psu_show_info(psu_info_t *psu_info)
{
    if (!psu_info) {
        printf("Invalid input: NULL \n");
        return;
    }

    printf("info for PSU %d: \n", psu_info->psu_num);
    printf("operation_state: %s\n", psu_get_operation_state(psu_info->operation_state));
    printf("volt_input: %10.2f\n", psu_info->volt_input);
    printf("current_input: %10.2f\n", psu_info->current_input);
    printf("volt_out: %10.2f\n", psu_info->volt_out);
    printf("power_input: %10.2f\n", psu_info->power_input);
    printf("fan_speed: %d\n", psu_info->fan_speed);
    printf("temperature1: %d\n", psu_info->temperature[0]);
    printf("temperature2: %d\n", psu_info->temperature[1]);
    printf("temperature3: %d\n\n", psu_info->temperature[2]);
}

void psu_show_aggregate_info(psu_aggregate_info_t *psu_aggr_info)
{
    if (!psu_aggr_info) {
        printf("Invalid input: NULL \n");
        return;
    }

    printf("\n========= PSU Summary ======= \n");
    printf("total_power: %10.2f\n", psu_aggr_info->total_power);
    printf("total_current: %10.2f\n", psu_aggr_info->total_current);
    printf("side_b_power: %10.2f\n", psu_aggr_info->side_b_power);
    printf("side_a_power: %10.2f\n", psu_aggr_info->side_a_power);
}

int main(int argc, char **argv)
{
    int       psu, psu_idx;
    uint8_t   psu_start = 0, psu_end = 0;
    int       i2c_bus, i2c_addr;
    psu_info_t           psu_info[MAX_PSU_NUM];
    psu_aggregate_info_t psu_aggr_info;
    uint8_t   show_status = 0;
    uint8_t   poll_psu = 0;

    memset(psu_info, 0, MAX_PSU_NUM*sizeof(psu_info_t));

    if (argc < 2) {
        printf("\nPlease input PSU unit#\n");
        return -1;
    }

    if (!strcmp(argv[1], "all")) {
        /*
         * all PSUs: 1 - 6
         */
        psu_start = 1;
        psu_end = MAX_PSU_NUM + 1;
    }
    else if ((strlen(argv[1]) == 1) && isdigit(argv[1][0])) {
        /*
         * PSU unit#: 1 - 6
         */
        psu = (int) atoi(argv[1]);
        if ((psu > MAX_PSU_NUM) || (psu < 1)) {
            printf("\npsu# out of range\n");
            return -1;
        }
        psu_start = psu;
        psu_end = psu + 1;
    } else {
        printf("Wrong psu#%s\n", argv[1]);
        return -1;
    }

    if (argc == 3) {
        if (!strcmp(argv[2], "--status"))  {
            show_status = 1;
        }
        else if (!strcmp(argv[2], "--poll"))  {
            poll_psu = 1;
        }
    }

    /*
     * get psu informatin from memory mapped file
     */
    for (int i = psu_start; i < psu_end; i++) {
        psu_idx = i - 1;
        psu_info[psu_idx].psu_num = i;
        printf("PSU util psu# %d\n", i);
        if (poll_psu == 1) {
           psu_poll_info(&psu_info[psu_idx]);
           syslog(LOG_WARNING, "poll PSU%d", i);
        }
        else {
            psu_get_info(&psu_info[psu_idx]);
        }

        psu_show_info(&psu_info[psu_idx]);

        if (show_status)
            psu_show_status(&psu_info[psu_idx]);
    }

    /*
     * PSU aggregate info
     */
    if (!strcmp(argv[1], "all")) {
        psu_aggregate_info(&psu_aggr_info);
        psu_show_aggregate_info(&psu_aggr_info);
    }
}
