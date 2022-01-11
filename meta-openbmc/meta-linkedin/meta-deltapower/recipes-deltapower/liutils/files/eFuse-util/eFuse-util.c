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
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <syslog.h>
#include <facebook/powershelf.h>

void usage()
{
    fprintf(stderr, "Usage:  eFuse-util all|[efuse unit 1 - 50] [--status|--reset|--on|--poll]\n");
    return;
}

void show_eFuse_info(efuse_info_t *efuse_info)
{
    if (!efuse_info) {
        printf("Invalid input: NULL \n");
        return;
    }
    printf("info for eFuse %d:\n", efuse_info->efuse_num);
    printf("state: %s\n", efuse_get_operation_state(efuse_info->state));
    printf("volt_input: %10.2f\n", efuse_info->volt_input);
    printf("current_input: %10.2f\n", efuse_info->current_input);
    printf("volt_out: %10.2f\n", efuse_info->volt_out);
    printf("power_input: %10.2f\n", efuse_info->power_input);
    printf("temperature: %d\n", efuse_info->temperature);
    printf("efuse_tripped: %d\n\n", efuse_info->efuse_tripped);
}

int main(int argc, char **argv)
{
    uint8_t      efuse, efuse_idx;
    int          i2c_bus, i2c_sel_addr;
    uint8_t      efuse_start = 0, efuse_end = 0;
    uint8_t      show_status = 0;
    uint8_t      reset_efuse = 0;
    uint8_t      on_efuse = 0;
    uint8_t      poll_efuse = 0;
    uint8_t      off_efuse = 0;
    int          ret;

    efuse_info_t *efuse_info;

    efuse_info = (efuse_info_t *) malloc(sizeof(efuse_info_t));

    if (!efuse_info) {
        printf("Failed to alloc memory for efuse_info\n");
        return -1;
    }

    if (argc < 2 || argc > 3) {
        usage();
        return -1;
    }

    if (!strcmp(argv[1], "all")) {
        efuse_start = 1;
        efuse_end = MAX_EFUSE_NUM + 1;
    }
    else if (((strlen(argv[1]) == 1) && isdigit(argv[1][0])) ||
             ((strlen(argv[1]) == 2) && isdigit(argv[1][0]) && isdigit(argv[1][1]))) {
        efuse = (int) atoi(argv[1]);
        if ((efuse > MAX_EFUSE_NUM) || (efuse < 1)) {
            printf("\neFuse# out of range\n");
            usage();
            return -1;
        }
        efuse_start = efuse;
        efuse_end = efuse + 1;
    } else {
        usage();
        return -1;
    }

    if (argc == 3) {
        if (!strcmp(argv[2], "--status"))  {
            show_status = 1;
        }
        else if (!strcmp(argv[2], "--reset"))  {
            reset_efuse = 1;
        }
        else if (!strcmp(argv[2], "--on"))  {
            on_efuse = 1;
        }
        else if (!strcmp(argv[2], "--poll"))  {
            poll_efuse = 1;
        }
        else if (!strcmp(argv[2], "--off"))  {
            off_efuse = 1;
        }
    }

    /*
     * get efuse informatin from memory mapped file
     */
    efuse_get_mmap_info();

    for (int i = efuse_start; i < efuse_end; i++) {
        memset(efuse_info, 0, sizeof(efuse_info_t));
        efuse_info->efuse_num = i;

        if (reset_efuse || on_efuse || off_efuse)
            poll_efuse = 1;

        if (poll_efuse == 1) {
            if (efuse_poll_info(efuse_info) != 0)
                syslog(LOG_WARNING, "poll failed: efuse#%d", i);
        }
        else {
            if (efuse_get_info(efuse_info) != 0)
                syslog(LOG_WARNING, "read failed: efuse#%d", i);
        }

        show_eFuse_info(efuse_info);
        if (show_status)
            efuse_show_status(efuse_info);
        else if (reset_efuse) {
            /*
             * reset efuse if it is tripped or vout issue
             */
            ret = efuse_reset(efuse_info->efuse_num);

            switch (ret) {
            case OP_RESET_ERROR:
                printf("efuse%d not tripped, can't be reset \n", efuse_info->efuse_num);
                syslog(LOG_WARNING, "efuse%d not tripped, can't be reset", efuse_info->efuse_num);
                break;
            case OP_OFF_ERROR:
                printf("efuse%d failed to turn off\n", efuse_info->efuse_num);
                syslog(LOG_WARNING, "efuse%d failed to turn off", efuse_info->efuse_num);
                break;
            case OP_ON_ERROR:
                printf("efuse%d failed to turn on\n", efuse_info->efuse_num);
                syslog(LOG_WARNING, "efuse%d failed to turn on", efuse_info->efuse_num);
                break;
            case OP_NO_ERROR:
                printf("efuse%d reset success\n", efuse_info->efuse_num);
                break;
            default:
                break;
            }
        }
        else if (on_efuse) {
            /*
             * turn on efuse
             */
             if (efuse_operation(efuse_info->efuse_num, 1) == 0) {
                 printf("eFuse%d is turned on\n", efuse_info->efuse_num);
             }
             else {
                 printf("eFuse%d failed to turn on\n", efuse_info->efuse_num);
                 syslog(LOG_WARNING, "eFuse%d failed to turn on", efuse_info->efuse_num);
             }
        }
        else if (off_efuse) {
            /*
             * turn off efuse
             */
             if (efuse_operation(efuse_info->efuse_num, 0) == 0) {
                 syslog(LOG_WARNING, "eFuse%d has been turned off", efuse_info->efuse_num);
                 printf("eFuse%d is turned off\n", efuse_info->efuse_num);
             }
             else {
                 printf("eFuse%d failed to turn off\n", efuse_info->efuse_num);
                 syslog(LOG_WARNING, "eFuse%d failed to turn off", efuse_info->efuse_num);
             }
        }
    }

    free(efuse_info);
}
