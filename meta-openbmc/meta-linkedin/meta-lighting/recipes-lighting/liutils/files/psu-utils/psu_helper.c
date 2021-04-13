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
#include <math.h>
#include <openbmc/gpio.h>
#include "openbmc/pal.h"
#include "psu_helper.h"

int psu_2_i2cbus[] = {3, 3, 3, 3, 3, 3};

void psu_usage ()
{
    fprintf(stderr, "Usage: psu-eeprom|psu-util [psu unit 1 - 6]\n");
    return;
}

int psu_read(int fd, void *buf, size_t size, char *msg)
{
    if (read(fd, buf, size) != size) {
        perror(msg);
        return -1;
    }
    return 0;
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
 *  Read PSU sysattr
 *   return:
 *     -1:    read error
 *      0:    read successful
 */
int
psu_read_device(const char *device, int *value) {
  FILE *fp;
  int rc;
  char buf[8];

  fp = fopen(device, "r");
  if (!fp) {
    return -1; 
  }

  rc = fscanf(fp, "%s", buf);
  *value = atoi(buf);

  fclose(fp);

  if (rc != 1) {
    return -1;
  } else {
    return 0;
  }
}

int
psu_write_device(const char *device, int value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    return -1;
  }

  rc = fprintf(fp, "%d ", value);

  fclose(fp);

  if (rc != 1) {
    return -1;
  } else {
    return 0;
  }
}

/*
 * check if PSU is present
 *   return:
 *     1:    present
 *     0:    not present
 */
int  psu_is_present (uint16_t gpio)
{
    if (gpio_get(gpio) == GPIO_VALUE_LOW) {
        return 1;
    }
    else
        return 0;
    return 0;
}

/*
 * convert 2's complement to decimal
 */
int convert2compl_to_decimal(int data, int bits)
{
    int negative = (data & (1 << (bits-1))) != 0;
    int nativeInt;

    if (negative)
        nativeInt = data | ~((1 << (bits-1) + 1) - 1);
    else
        nativeInt = data;

    return nativeInt;
}

/*
 * real value for linear data format
 */
float psu_get_realvalue(int input)
{
    int     y;
    int     n;
    double  real_val;
    int     bits = 11;
    int     mask = 0x7FF;

    /* 
     * linear data format
     * real value X = Y*(2^N)
     *    Y: 11 bit 2's complement integer
     *    N: 5 bit 2's complement integer
     */
    y = convert2compl_to_decimal(input&mask, 11);
    n = convert2compl_to_decimal(input >> 11, 5);

    real_val = y*pow(2.0, n);
    return real_val;
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
