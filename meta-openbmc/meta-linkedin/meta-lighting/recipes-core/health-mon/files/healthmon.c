/*
 * healthmon
 *
 * Copyright 2018-present LinkedIn. All Rights Reserved.
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
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <sys/file.h>
#include <fcntl.h>
#include <openbmc/obmc-i2c.h>
#include <openbmc/gpio.h>
#include <facebook/powershelf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

#define    HEALTHMON_INTERVAL      10
#define    MAX_RETRIES             10
#define    BMC_POLL_WAIT_TIME      5

/*
 * Start to send GPIO low-high pulse after 60s
 */
#define    BMC_SEND_PULSE_WAIT     60
/*
 * 3 minute boot timeout:   9 GPIO LOW<->HIGH pulse (20s)
 */
#define    BMC_BOOT_TIMEOUT        9

/*
 * BMC HEARTBEAT GPIOs:
 *   HEARTBEAT_OUT_GPIO(C1)
 *   HEARTBEAT_IN_GPIO(Y0)
 */
#define BMC_HEARTBEAT_OUT_GPIO_NUM      17
#define BMC_HEARTBEAT_IN_GPIO_NUM       192

/*
 * BMC Identity GPIO(J2):
 */
#define BMC_ID_GPIO_NUM                 76

/*
 * Cached Data
 */
efuse_info_t efuse_info_cache[MAX_EFUSE_NUM];
psu_info_t   psu_info_cache[MAX_PSU_NUM];
psu_info_t   psu_info_mem[MAX_PSU_NUM];
fan_info_t   fan_info_cache;
int         *mmap_addr;

int          fd_mmap;
int          bmc_id;

/*
 * BMC_ID_GPIO_NUM - GPIO for BMC0 or BMC1
 */
int which_bmc ()
{
    return gpio_get(BMC_ID_GPIO_NUM);
}

void set_gpio_out_direction(uint8_t gpio_num);

/*
 * save efuse info to cached table
 */
int
save_efuse_info (efuse_info_t *efuse_info)
{
    if (!efuse_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    if ((efuse_info->efuse_num < 1) || efuse_info->efuse_num > MAX_EFUSE_NUM) {
       syslog(LOG_WARNING, "efuse%d is out of range(1-50)", efuse_info->efuse_num);
       return -1;
    }

    memcpy(&efuse_info_cache[efuse_info->efuse_num - 1], efuse_info, sizeof(efuse_info_t));
    return 0;
}

/*
 * save to info table
 */
int
save_psu_info (psu_info_t *psu_info)
{
    if (!psu_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    if ((psu_info->psu_num < 1) || psu_info->psu_num > MAX_PSU_NUM) {
       syslog(LOG_WARNING, "psu %d is out of range", psu_info->psu_num);
       return -1;
    }

    get_mmap_info((int*)&psu_info_mem, FILE_PSU, sizeof(psu_info_mem));

    for(int i=0; i < PSU_STATUS_MAX; i++) {
        psu_info->status_cntr[i] += psu_info_mem[psu_info->psu_num - 1].status_cntr[i];
    }

    memcpy(&psu_info_cache[psu_info->psu_num - 1], psu_info, sizeof(psu_info_t));
    return 0;
}

/*
 * save to info cache table
 */
int
save_fan_info (fan_info_t *fan_info)
{
    if (!fan_info) {
        syslog(LOG_WARNING, "Invalid input:  NULL \n");
        return -1;
    }

    memcpy(&fan_info_cache, fan_info, sizeof(fan_info_t));
    return 0;
}

/*
 * Write to mmap file
 */
int
write_mmap_file(uint8_t fd)
{
    uint8_t   retries = 5;
    int       done = 0, offset, len;

    for(int attempt; attempt < retries; ++attempt) {
        done = flock(fd, LOCK_EX | LOCK_NB);
        if (done != 0) {
            syslog(LOG_WARNING, "Failed to lock mmap file, errno: %d", errno);
            /*
             * lock held by another proc
             */
            sleep(1);
            continue;
        }
        else
            break;
    }

    if (done != 0) {
        syslog(LOG_WARNING, "Failed to lock mmap file\n");
        return -1;
    }

    /*
     * write fan info to the mmaped file
     */
    offset = get_file_offset(FILE_FAN);
    len = sizeof(fan_info_cache) + 1;
    memcpy(mmap_addr+offset, &fan_info_cache, len);

    /*
     * write psu info to the mmaped file
     */
    offset = get_file_offset(FILE_PSU);
    len = sizeof(psu_info_cache);
    memcpy(mmap_addr+offset, psu_info_cache, len);

    /*
     * write efuse info to the mmaped file
     */
    offset = get_file_offset(FILE_EFUSE);
    len = sizeof(efuse_info_cache);
    memcpy(mmap_addr + offset, efuse_info_cache, len);

    flock(fd, LOCK_UN);
    return 0;
}

void init_psus (int *mmap_addr)
{
    psu_info_t *psu_info;

    if (!mmap_addr)
       return;

    psu_info = (psu_info_t *) malloc(sizeof(psu_info_t));

    if (!psu_info)
        return;

    for (int i = 1; i < MAX_PSU_NUM + 1; i++) {
        memset(psu_info, 0, sizeof(psu_info_t));
        psu_info->psu_num = i;
        save_psu_info(psu_info);
    }

    free(psu_info);
}

int init_poll (int filesize)
{
    int mode = 0x0777;

    bmc_id = which_bmc();

    if (fd_mmap == -1) {
        syslog(LOG_WARNING, "failed to open mmap file");
        return -1;
    }

    /* go to the location corresponding to the last byte */
    if (lseek (fd_mmap, filesize - 1, SEEK_SET) == -1)
    {
        syslog(LOG_WARNING, "mmap file lseek error");
        return -1;
    }

    /* write a dummy byte at the last location */
    if (write (fd_mmap, "", 1) != 1)  {
        syslog(LOG_WARNING, "mmap file write error");
        return -1;
    }

    /* Now the file is ready to be mmapped.
     */
    mmap_addr = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mmap, 0);
    if (mmap_addr == MAP_FAILED) {
        syslog(LOG_WARNING, "failed to mmap()");
        return -1;
    }

    init_psus(mmap_addr);

    return 0;
}

/*
 *  Poll the Hardware and save to structures
 */
void poll_psu_info (int *mmap_addr, uint8_t fd)
{
    int        len;
    int        offset;
    psu_info_t *psu_info;

    if (!mmap_addr)
       return;

    psu_info = (psu_info_t *) malloc(sizeof(psu_info_t));

    if (!psu_info)
        return;

    /*
     * check PSU presense, AC input and all faults and warning
     */
    for (int i = 1; i < MAX_PSU_NUM + 1; i++) {
        memset(psu_info, 0, sizeof(psu_info_t));
        psu_info->psu_num = i;
        if (psu_poll_info(psu_info) == -1) {
           syslog(LOG_WARNING, "failed to poll psu%d info", i);
           continue;
        }

        save_psu_info(psu_info);

        /*
         * check PSU presense, AC input
         */
        if (psu_info->operation_state == PSU_NOT_PRESENT) {
            syslog(LOG_WARNING, "psu%d not present", i);
            continue;
        }
        else if (psu_info->operation_state == PSU_OFF) {
            syslog(LOG_WARNING, "psu%d is off", i);
            continue;
        }
        else if (psu_info->operation_state == PSU_NO_AC_INPUT) {
            syslog(LOG_WARNING, "psu%d has no ac input", i);
            continue;
        }
    }

    free(psu_info);
}

void check_efuse_temp(efuse_info_t *efuse_info)
{
    if(efuse_info->temperature > WARN_EFUSE_TEMP)
    {
        syslog(LOG_WARNING, "healthmon: efuse%d exceeded warning temperature %dC", efuse_info->efuse_num, WARN_EFUSE_TEMP);
    }

    if (efuse_info->temperature > MAX_EFUSE_TEMP && efuse_info->efuse_tripped == 0)
    {
        // turn efuse off
        syslog(LOG_WARNING, "healthmon: efuse%d exceeded max temperature. Turning off", efuse_info->efuse_num);
        if (efuse_operation(efuse_info->efuse_num, 0) == 0) {
            syslog(LOG_WARNING, "healthmon: eFuse%d has been turned off", efuse_info->efuse_num);
        }
         else {
            syslog(LOG_WARNING, " healthmon: eFuse%d failed to turn off", efuse_info->efuse_num);
        }
    }
}

void poll_efuse_info(int *mmap_addr, uint8_t fd)
{
    int          i2c_bus, selector_addr, len;
    int          offset;
    efuse_info_t *efuse_info;

    if (!mmap_addr) {
        syslog(LOG_WARNING, "healthmon: efuse mmap: NULL");
        return;
    }

    efuse_info = (efuse_info_t *) malloc(sizeof(efuse_info_t));
    if (!efuse_info) {
        syslog(LOG_WARNING, "healthmon: efuse memory alloc failed");
        return;
    }

    /*
     * check faults and warning
     */
    for (int i = 1; i < MAX_EFUSE_NUM + 1; i++) {
        memset(efuse_info, 0, sizeof(efuse_info_t));
        efuse_info->efuse_num = i;
        if (efuse_poll_info(efuse_info) == -1) {
            continue;
        }

        // check if max temp has been exceeded
        check_efuse_temp(efuse_info);

        save_efuse_info(efuse_info);
    }

    free(efuse_info);
    return;
}

/*
 * Poll FAN Hardware info
 * save hardware data into cached data table
 */
void poll_fan_info (int *mmap_addr, uint8_t fd)
{
    int    len, offset;
    fan_info_t *fan_info;
    fan_info = (fan_info_t *) malloc(sizeof(fan_info_t));

    if (!fan_info)
        return;

    memset(fan_info, 0, sizeof(fan_info_t));

    if (fan_poll_info(fan_info) == -1) {
       syslog(LOG_WARNING, "failed to poll fan info");
       free(fan_info);
       return;
    }

    save_fan_info(fan_info);

    free(fan_info);
}

/*
 * Set GPIO direction to OUT
 */
void set_gpio_out_direction(uint8_t gpio_num)
{
    gpio_st g;
    gpio_direction_en dir;

    /*
     * GPIO current direction
     */
    g.gs_gpio = gpio_num;
    gpio_current_direction(&g, &dir);

    if (dir == GPIO_DIRECTION_IN) {
        gpio_change_direction(&g, GPIO_DIRECTION_OUT);
    }

    return;
}

int do_poll(int timedout_flag, int fd)
{
    int    randomTime = 0, ret = 0;
    time_t endwait;
    time_t start = time(NULL);
    time_t seconds = HEALTHMON_INTERVAL;

    /*
     * set GPIO OUT to LOW
     */
    sleep(1);

    /*
     * Own the i2c buses, start transaction of HW access
     */
    sleep(BMC_POLL_WAIT_TIME);

    /*
     * Get HW data
     */
    poll_efuse_info(mmap_addr, fd);
    poll_psu_info(mmap_addr, fd);
    poll_fan_info(mmap_addr, fd);

    if (write_mmap_file(fd) == -1)  {
        syslog(LOG_WARNING, "failed to write to mmap");
    }

    /*
     * Done access HW, set GPIO OUT to HIGH
     */
    sleep(BMC_POLL_WAIT_TIME);
    return 0;
}

int main(int argc, char **argv) {
    int     filesize;
    struct  stat statbuf;
    int     retry = 3;
    int     boot_timeout_count  = BMC_BOOT_TIMEOUT;
    int     timedout_flag = 0;
    int     in_gpio_value_prev = -1;
    int     in_gpio_value_current;
    int     count = 0, timeout_count = 0;
    int     start_send_pulse = 0;
    int     gpio_pulse_count = 0;
    int     gpio_low_count = 0;
    int     gpio_value = GPIO_VALUE_HIGH;
    int     mode = 0x0777;

    filesize = sizeof(efuse_info_cache) + sizeof(psu_info_cache) + sizeof(fan_info_cache);

    /*
     * Open a file for writing efuse info.
     */
    fd_mmap = open(MMAP_FILEPATH, O_RDWR | O_CREAT | O_TRUNC, mode);

    /*
     * initialization
     */
    init_poll(filesize);

    if (bmc_id == 0)
    {
        in_gpio_value_prev = 0;
    }
    else
    {
        // do not run on BMC1
        return 0;
    }

    while (1) {
        sleep(1);
        /*
         * timeout check
         */
        do_poll(timedout_flag, fd_mmap);
    }

    /* Don't forget to free the mmapped memory
     */
    if (munmap(mmap_addr, filesize) == -1) {
        syslog(LOG_WARNING, "Error un-mmapping the efuse file");
    }

    /*
     * Un-mmaping doesn't close the file, so we still need to do that.
     */
    close(fd_mmap);
}
