/*
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include "psu_eeprom.h"
#include "psu_helper.h"

int parse_psu_eeprom (char* buf, psu_eeprom_t* psu_eeprom)
{
    char   *ptr = buf;
    memset(psu_eeprom, 0, sizeof(psu_eeprom));

    strncpy(psu_eeprom->checksum[0], ptr+CHECKSUM1_ADDR, 1);
    strncpy(psu_eeprom->checksum[1], ptr+CHECKSUM2_ADDR, 1);
    strncpy(psu_eeprom->checksum[2], ptr+CHECKSUM3_ADDR, 1);
    strncpy(psu_eeprom->checksum[3], ptr+CHECKSUM4_ADDR, 1);
    strncpy(psu_eeprom->checksum[4], ptr+CHECKSUM5_ADDR, 1);
    strncpy(psu_eeprom->checksum[5], ptr+CHECKSUM6_ADDR, 1);
    strncpy(psu_eeprom->checksum[6], ptr+CHECKSUM7_ADDR, 1);
    strncpy(psu_eeprom->checksum[7], ptr+CHECKSUM8_ADDR, 1);
    strncpy(psu_eeprom->manuf_name, ptr+MANUFNAME_ADDR, MANUFNAME_SIZE);
    strncpy(psu_eeprom->product_name, ptr+PRODUCTNAME_ADDR, PRODUCTNAME_SIZE);
    strncpy(psu_eeprom->modelno, ptr+MODELNO_ADDR, MODELNO_SIZE);
    strncpy(psu_eeprom->product_version, ptr+PRODUCTVERSION_ADDR, PRODUCTVERSION_SIZE);
    strncpy(psu_eeprom->product_serial, ptr+PRODUCTSERIAL_ADDR, PRODUCTSERIAL_SIZE);

    psu_eeprom->manuf_name[MANUFNAME_SIZE-1] = 0;
    psu_eeprom->product_name[PRODUCTNAME_SIZE-1] = 0;
    psu_eeprom->modelno[MODELNO_SIZE-1] = 0;
    psu_eeprom->product_version[PRODUCTVERSION_SIZE-1] = 0;
    psu_eeprom->product_serial[PRODUCTSERIAL_SIZE-1] = 0;

    return 0;
}

int main(int argc, char **argv)
{
    int fd;
    int psu = 1;
    int ret_val;
    int i2cbus;
    char filename[FILENAME_SIZE];
    char buf[PSU_EEPROM_SIZE];

    psu_eeprom_t psu_eeprom;

    if (argc < 2) {
        printf("\nPlease input PSU unit#\n");
        psu_usage();
        return 0;
    }

    psu = (int) atoi(argv[1]);
    if ((psu > 6) || (psu < 1)) {
        printf("\nPSU unit# out of range\n");
        psu_usage();
        return 0;
    }

    /*
     * check if PSU present
     */
    memset(buf, 0, PSU_EEPROM_SIZE);
    i2cbus = psu_to_i2cbus(psu);
    if ((i2cbus < MIN_I2C_BUS_NUM) || (i2cbus > MAX_I2C_BUS_NUM)) {
      fprintf(stderr, "Invalid i2c bus %d for PSU%d\n", i2cbus, psu);
      exit(1);
    }

    snprintf(filename, FILENAME_SIZE, "/sys/class/i2c-adapter/i2c-%d/%d-0050/eeprom", i2cbus, i2cbus);

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "fail to open file for %d\n", psu);
      exit(1);
    }

    ret_val = psu_read(fd, buf, PSU_EEPROM_SIZE, "read PSU eeprom");
    if (ret_val < 0) {
      fprintf(stderr, "PSU%d not present\n", psu);
      exit(1);
    }

    parse_psu_eeprom(buf, &psu_eeprom);

    printf("\n===================================\n");

    printf("checksum:  %s\n", psu_eeprom.checksum);
    printf("manufactor name:  %s\n", psu_eeprom.manuf_name);
    printf("product name:  %s\n", psu_eeprom.product_name);
    printf("modelno:  %s\n", psu_eeprom.modelno);
    printf("product_version:  %s\n", psu_eeprom.product_version);
    printf("product serial:  %s\n", psu_eeprom.product_serial);

    close(fd);
    return 0;
}
