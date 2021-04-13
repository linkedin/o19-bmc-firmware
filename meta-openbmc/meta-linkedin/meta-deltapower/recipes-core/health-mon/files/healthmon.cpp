/*
 * healthmon
 *
 * Copyright 2018-present LinkedIn. All Rights Reserved.
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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#define    PSU_NUM                 6
#define    EFUSE_NUM               50
#define    PSU_SLAVE               0x40
#define    CMD_SIZE                40
#define    DEVICE_FILE_SIZE        120
#define    HEALTHMON_INTERVAL      30

/*
 * Fan1_2 and Fan3_4 status
 *       HI BYTE: (Fan 1 or fan3) / LO BYTE: (Fan 2 or Fan 4)
 *          Bit 0: FAN FAIL
 *          Bit 1: FAN SPEED WARNING
*/
#define    FAN_HIBYTE_SPEED_WARNING    1<<9
#define    FAN_HIBYTE_SPEED_FAIL       1<<8
#define    FAN_LOWBYTE_SPEED_WARNING   1<<2
#define    FAN_LOWBYTE_SPEED_FAIL      0x1

/* Fan status word
 *    Bit 2: Temperature warning
 *    Bit 11: Fan warning
 *    Bit 15: VOUT fault
 */
#define    FAN_TEMP_WARNING            1<<2
#define    FAN_WARNING                 1<<11
#define    FAN_VOUT_FAULT              1<<15

/*
 * PSU status_temperature
 *   bit 7:    Overtemperature fault
 *   bit 6:    Overtemperature warning
 */
#define    PSU_OVERTEMP_FAULT              1<<7
#define    PSU_OVERTEMP_WARNING            1<<6

/*
 * PSU status_fan1_2:
 *   bit 7:     fan1 fault
 *   bit 3:     fan1 over-ride
 */
#define    PSU_FAN1_FAULT                  1<<7
#define    PSU_FAN1_OVERRIDE               1<<3

/*
 * PSU status_vout:
 *   bit 7:    PS is shut down for OV fault
 *   bit 4:    PS is shut down for UV fault
 */
#define    PSU_VOUT_OV_FAULT               1<<7
#define    PSU_VOUT_UV_FAULT               1<<4

/*
 * PSU status_iout:
 *   bit 7:    PS is shut down for OC fault
 *   bit 5:    PS Output over current warning
 */
#define    PSU_IOUT_OC_FALUT               1<<7
#define    PSU_IOUT_OC_WARNING             1<<5

/*
 * PSU status_vin:
 *   bit 7:    input over voltage fault
 *   bit 3:    VIN_LOW_OFF - PS is off for UV
 */
#define    PSU_VIN_OV_FAULT               1<<7
#define    PSU_VIN_UV_FAULT               1<<3

/*
 * PSU status_word
 *    bit 11:  POWER_GOOD signal is negated
 *    bit 10:  fan or airflow fault or warning
 *    bit 6:   PS is off
 *    bit 5:   PS VOUT Over Voltage fault
 *    bit 4:   PS VC Over Current fault
 *    bit 3:   PS Input under-voltage fault
 *    bit 2:   Temperature fault or warning
 */
#define    PSU_POWER_GOOD_NEG             1<<11
#define    PSU_FAN_FAULT_WARNING          1<<10
#define    PSU_IS_OFF                     1<<6
#define    PSU_OVER_VOLT_FAULT            1<<5
#define    PSU_OVER_CURRENT_FALUT         1<<4
#define    PSU_UNDER_VOLT_FAULT           1<<3
#define    PSU_TEMP_FAULT_WARNING         1<<2

#define    FAN_BUS                 8
#define    FAN_MUX_ADDR            0x7c
#define    FAN_DIR  "/sys/class/i2c-adapter/i2c-%d/%d-0030/"
#define    PSU_DIR  "/sys/class/i2c-adapter/i2c-%d/%d-0040/"

uint8_t   psu_i2cbus[] = {12, 1, 2, 3, 6, 7};
uint8_t   psu_i2caddr[] = {0x71, 0x72, 0x73, 0x74, 0x75, 0x76};

bool verbose = false;

int set_i2c_master(uint8_t i2c_bus,   uint8_t i2c_addr)
{
    char     cmd[CMD_SIZE];

    snprintf(cmd, CMD_SIZE, "/usr/local/bin/set_i2c_mselector.sh %d %x", i2c_bus, i2c_addr);
    system(cmd);

    return 0;
}

/*
 * We need to open the device each time to read a value
 */
int read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
//    syslog(LOG_INFO, "failed to open device %s", device);
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);

  if (rc != 1) {
//    syslog(LOG_INFO, "failed to read device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

int fan_temp_okay() {
    int   value;
    char  full_name[DEVICE_FILE_SIZE];
    char  device_name[DEVICE_FILE_SIZE];

    snprintf(device_name, DEVICE_FILE_SIZE, FAN_DIR, FAN_BUS, FAN_BUS);

    /*
     * I2C master selector
     */
    set_i2c_master(FAN_BUS, FAN_MUX_ADDR);

    /*
     * STATUS_WORD:
     */
    snprintf(full_name, DEVICE_FILE_SIZE, "%s/status_word", device_name);
    syslog(LOG_DEBUG, "fan status word: %s: STATUS_WORD\n", full_name);

    if (read_device(full_name, &value)) {
        syslog(LOG_DEBUG, "fan read failed: STATUS_WORD\n");
    } else {
        if (value & FAN_TEMP_WARNING) {
            syslog(LOG_CRIT, "Temperature Warning!\n");
        }

        if (value & FAN_WARNING) {
            syslog(LOG_CRIT, "Fan Warning!\n");
        }

        if (value & FAN_VOUT_FAULT) {
            syslog(LOG_CRIT, "Fancontrol VOUT Fault!\n");
        }
    }

    /*
     *    STATUS_FANS_1_2:
     *       HI BYTE: Fan 1 / LO BYTE: Fan 2
     *          Bit 0: FAN FAIL
     *          Bit 1: FAN SPEED WARNING
     */
    snprintf(full_name, DEVICE_FILE_SIZE, "%s/fan1_2_status", device_name);
    if (read_device(full_name, &value)) {
        syslog(LOG_WARNING, "fan read failed: fan1_2_status\n");
    } else {
        if (value & FAN_HIBYTE_SPEED_WARNING) {
            syslog(LOG_CRIT, "Fan1 speed Warning!\n");
        }
        if (value & FAN_HIBYTE_SPEED_FAIL) {
            syslog(LOG_CRIT, "Fan1 fail!\n");
        }
        if (value & FAN_LOWBYTE_SPEED_WARNING) {
            syslog(LOG_CRIT, "Fan2 speed Warning!\n");
        }
        if (value & FAN_LOWBYTE_SPEED_FAIL) {
            syslog(LOG_CRIT, "Fan2 fail!\n");
        }
    }

    /*
     *    STATUS_FANS_3_4:
     *       HI BYTE: Fan 3 / LO BYTE: Fan 4
     *          Bit 0: FAN FAIL
     *          Bit 1: FAN SPEED WARNING
     */
    snprintf(full_name, DEVICE_FILE_SIZE, "%s/fan3_4_status", device_name);
    if (read_device(full_name, &value)) {
        syslog(LOG_WARNING, "fan read failed: fan3_4_status\n");
    } else {
        if (value & FAN_HIBYTE_SPEED_WARNING) {
            syslog(LOG_CRIT, "Fan3 speed Warning!\n");
        }
        if (value & FAN_HIBYTE_SPEED_FAIL) {
            syslog(LOG_CRIT, "Fan3 fail!\n");
        }
        if (value & FAN_LOWBYTE_SPEED_WARNING) {
            syslog(LOG_CRIT, "Fan4 speed Warning!\n");
        }
        if (value & FAN_LOWBYTE_SPEED_FAIL) {
            syslog(LOG_CRIT, "Fan4 fail!\n");
        }
    }

    return 0;
}

int psu_okay() {
    int     status_vout;
    int     status_iout;
    int     status_vin;
    int     status_temp;
    int     status_fan1_2;
    int     status_word;
    int     i2c_bus;
    int     i2c_sel_addr;
    char    dir[DEVICE_FILE_SIZE];
    char    psu_file[DEVICE_FILE_SIZE];

    for (int i = 0; i < PSU_NUM; i++) {
        i2c_bus = psu_i2cbus[i];
        i2c_sel_addr =  psu_i2caddr[i];

        /*
         * I2C master selector
         */
        set_i2c_master(i2c_bus, i2c_sel_addr);
        snprintf(dir, DEVICE_FILE_SIZE, PSU_DIR,
                 i2c_bus, i2c_bus);

       /*
        * PSU status_temperature
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_temperature", dir);
       if (read_device(psu_file, &status_temp) != 0) {
          syslog(LOG_WARNING, "PSU not present or failed to read PSU%d\n", i+1);
          continue;
       }
       else {
           if (status_temp & PSU_OVERTEMP_FAULT) {
               syslog(LOG_WARNING, "PSU%d Over temperature Fault!\n", i+1);
           }
           if (status_temp & PSU_OVERTEMP_WARNING) {
               syslog(LOG_WARNING, "PSU%d Over temperature Warning!\n", i+1);
           }
       }

       /*
        * PSU status_fan1_2:
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_fan1_2", dir);
       if (read_device(psu_file, &status_fan1_2) != 0) {
           printf("failed to read status_fan1_2 for PSU%d\n", i+1);
           return -1;
       }
       else {
           if (status_fan1_2 & PSU_FAN1_FAULT) {
               syslog(LOG_WARNING, "PSU%d fan 1 fault! \n", i+1);
           }
           if (status_fan1_2 & PSU_FAN1_OVERRIDE) {
               printf("PSU%d fan 1 Over-ride by system!\n", i+1);
           }
       }

       /*
        * PSU status_vout:
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_vout", dir);
       if (read_device(psu_file, &status_vout) != 0) {
           syslog(LOG_WARNING, "failed to read status_vout for PSU%d\n", i+1);
           continue;
       }
       else {
           if (status_vout & PSU_VOUT_OV_FAULT) {
               syslog(LOG_WARNING, "PSU%d PS is shut down for OV fault\n", i+1);
           }
           if (status_vout & PSU_VOUT_UV_FAULT) {
               syslog(LOG_WARNING, "PSU%d PS is shut down for UV fault\n", i+1);
           }
       }

       /*
        * PSU status_iout:
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_iout", dir);
       if (read_device(psu_file, &status_iout) != 0) {
           syslog(LOG_WARNING, "failed to read status_iout for PSU%d\n", i+1);
           continue;
       }
       else {
           if (status_iout & PSU_IOUT_OC_FALUT) {
               syslog(LOG_WARNING, "PSU%d PS is shut down for OV fault\n", i+1);
           }
           if (status_iout & PSU_IOUT_OC_WARNING) {
               syslog(LOG_WARNING, "PSU%d Output Overcurrent warning\n", i+1);
           }
       }

       /*
        * PSU status_vin:
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_input", dir);
       if (read_device(psu_file, &status_vin) != 0) {
           syslog(LOG_WARNING, "failed to read status_vin for PSU%d\n", i+1);
           continue;
       }
       else {
           if (status_iout & PSU_VIN_OV_FAULT) {
               syslog(LOG_WARNING, "PSU%d input Over Voltage fault\n", i+1);
           }
           if (status_iout & PSU_VIN_UV_FAULT) {
               syslog(LOG_WARNING, "PSU%d PSU is off for UV\n", i+1);
           }
       }

       /*
        * PSU status_word
        */
       snprintf(psu_file, DEVICE_FILE_SIZE, "%s/status_word", dir);
       if (read_device(psu_file, &status_word) != 0) {
           syslog(LOG_WARNING, "failed to read status_word for PSU%d\n", i+1);
           continue;
       }
       else {
           if (status_word & PSU_TEMP_FAULT_WARNING)
               syslog(LOG_CRIT, "PSU%d temperature fault or warning\n", i+1);

           if (status_word & PSU_UNDER_VOLT_FAULT)
               syslog(LOG_CRIT, "PSU%d input undervoltage fault\n", i+1);

           if (status_word & PSU_OVER_CURRENT_FALUT)
               syslog(LOG_CRIT, "PSU%d output overcurrent fault\n", i+1);

           if (status_word & PSU_OVER_VOLT_FAULT)
               syslog(LOG_CRIT, "PSU%d output overvoltage fault \n", i+1);

           if (status_word & PSU_IS_OFF)
               syslog(LOG_CRIT, "PSU%d this unit is off\n", i+1);

           if (status_word & PSU_FAN_FAULT_WARNING)
               syslog(LOG_CRIT, "PSU%d fan or airflow fault or warning\n", i+1);

           if (status_word & PSU_POWER_GOOD_NEG)
               syslog(LOG_CRIT, "PSU%d POWER_GOOD signal is negated\n", i+1);
        } 
    }

    return 0;
}

#if 0
int efuses_okay() {
    set_i2c_master(0, 0x79);
    set_i2c_master(4, 0x7a);

    for (int i = 0; i < EFUSE_NUM; i++){

    }

    return 0;
}
#endif

/* 
 * shut down on receipt of a signal 
 */
void healthmon_interrupt(int sig)
{
  syslog(LOG_WARNING, "Shutting down healthmon on signal %s", strsignal(sig));
  exit(3);
}

int main(int argc, char **argv) {
  struct sigaction sa;

  sa.sa_handler = healthmon_interrupt;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);

  // Start writing to syslog as early as possible for diag purposes.
  daemon(1, 0);

  sleep(5);  /* Give the fans time to come up to speed */

  while (1) {

    sleep(HEALTHMON_INTERVAL);
    fan_temp_okay();
    psu_okay();
  }
}
