/*
 * yellowsub-sensors
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
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
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdint.h>
#include <pthread.h>
#include <facebook/bic.h>
#include <openbmc/ipmi.h>
#include <facebook/yellowsub_sensor.h>

int
main(int argc, char **argv) {
  int value;
  float fvalue;
  uint8_t slot_id;

  slot_id = atoi(argv[1]);

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_INLET_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_INLET_TEMP\n");
  } else {
    printf("MB_SENSOR_INLET_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_OUTLET_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_OUTLET_TEMP\n");
  } else {
    printf("MB_SENSOR_OUTLET_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_ZONE1_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_ZONE1_TEMP\n");
  } else {
    printf("MB_SENSOR_ZONE1_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_ZONE2_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_ZONE2_TEMP\n");
  } else {
    printf("MB_SENSOR_ZONE2_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_ZONE3_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_ZONE3_TEMP\n");
  } else {
    printf("MB_SENSOR_ZONE3_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN0_TACH, &value)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN0_TACH\n");
  } else {
    printf("MB_SENSOR_FAN0_TACH: %d rpm\n", value);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN1_TACH, &value)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN1_TACH\n");
  } else {
    printf("MB_SENSOR_FAN1_TACH: %d rpm\n", value);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN2_TACH, &value)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN2_TACH\n");
  } else {
    printf("MB_SENSOR_FAN2_TACH: %d rpm\n", value);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN3_TACH, &value)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN3_TACH\n");
  } else {
    printf("MB_SENSOR_FAN3_TACH: %d rpm\n", value);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD5V_AUX, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD5V_AUX\n");
  } else {
    printf("MB_SENSOR_VDD5V_AUX: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD1_2V_STBY, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD1_2V_STBY\n");
  } else {
    printf("MB_SENSOR_VDD1_2V_STBY: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD1_5V_STBY, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD1_5V_STBY\n");
  } else {
    printf("MB_SENSOR_VDD1_5V_STBY: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD2_5V_STBY, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD2_5V_STBY\n");
  } else {
    printf("MB_SENSOR_VDD2_5V_STBY: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD5V_STBY, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD5V_STBY\n");
  } else {
    printf("MB_SENSOR_VDD5V_STBY: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD3_3V_STBY, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD3_3V_STBY\n");
  } else {
    printf("MB_SENSOR_VDD3_3V_STBY: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_VDD12V_SNS_1, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_VDD12V_SNS_1\n");
  } else {
    printf("MB_SENSOR_VDD12V_SNS_1: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_HSC_IN_VOLT, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_HSC_IN_VOLT\n");
  } else {
    printf("MB_SENSOR_HSC_IN_VOLT: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_HSC_OUT_CURR, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_HSC_OUT_CURR\n");
  } else {
    printf("MB_SENSOR_HSC_OUT_CURR: %.2f Amps\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_HSC_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_HSC_TEMP\n");
  } else {
    printf("MB_SENSOR_P3V3: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_HSC_IN_POWER, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_HSC_IN_POWER\n");
  } else {
    printf("MB_SENSOR_HSC_IN_POWER: %.2f Watts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN0_1_TACH, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN0_1_TACH\n");
  } else {
    printf("MB_SENSOR_FAN0_1_TACH: %.2f rpm\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN1_1_TACH, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN1_1_TACH\n");
  } else {
    printf("MB_SENSOR_FAN1_1_TACH: %.2f rpm\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN2_1_TACH, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN2_1_TACH\n");
  } else {
    printf("MB_SENSOR_FAN2_1_TACH: %.2f rpm\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, MB_SENSOR_FAN3_1_TACH, &fvalue)) {
    printf("yellowsub_sensor_read failed: MB_SENSOR_FAN3_1_TACH\n");
  } else {
    printf("MB_SENSOR_FAN3_1_TACH: %.2f rpm\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_TEMP\n");
  } else {
    printf("BIC_SENSOR_SOC_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_BDE_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_BDE_TEMP\n");
  } else {
    printf("BIC_SENSOR_MB_INLET_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_PCH_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_PCH_TEMP\n");
  } else {
    printf("BIC_SENSOR_PCH_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_THERM_MARGIN, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_THERM_MARGIN\n");
  } else {
    printf("BIC_SENSOR_SOC_THERM_MARGIN: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_TJMAX, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_TJMAX\n");
  } else {
    printf("BIC_SENSOR_SOC_TJMAX: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_DIMMA0_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_DIMMA0_TEMP\n");
  } else {
    printf("BIC_SENSOR_SOC_DIMMA0_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_DIMMA1_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_DIMMA1_TEMP\n");
  } else {
    printf("BIC_SENSOR_SOC_DIMMA1_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_DIMMB0_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_DIMMB0_TEMP\n");
  } else {
    printf("BIC_SENSOR_SOC_DIMMB0_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_DIMMB1_TEMP, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_DIMMB1_TEMP\n");
  } else {
    printf("BIC_SENSOR_SOC_DIMMB1_TEMP: %.2f C\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_P3V3_MB, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_P3V3_MB\n");
  } else {
    printf("BIC_SENSOR_P3V3_MB: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_P12V_MB, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_P12V_MB\n");
  } else {
    printf("BIC_SENSOR_P12V_MB: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_P1V05_PCH, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_P1V05_PCH\n");
  } else {
    printf("BIC_SENSOR_P1V05_PCH: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_P3V3_STBY_MB, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_P3V3_STBY_MB\n");
  } else {
    printf("BIC_SENSOR_P3V3_STBY_MB: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_P5V_STBY_MB, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_P5V_STBY_MB\n");
  } else {
    printf("BIC_SENSOR_P5V_STBY_MB: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_PV_BAT, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_PV_BAT\n");
  } else {
    printf("BIC_SENSOR_PV_BAT: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_PVDDR, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_PVDDR\n");
  } else {
    printf("BIC_SENSOR_PVDDR: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_PVCC_GBE, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_PVCC_GBE\n");
  } else {
    printf("BIC_SENSOR_PVCC_GBE: %.2f Volts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_INA230_POWER, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_INA230_POWER\n");
  } else {
    printf("BIC_SENSOR_INA230_POWER: %.2f Watts\n", fvalue);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SOC_PACKAGE_PWR, &fvalue)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SOC_PACKAGE_PWR\n");
  } else {
    printf("BIC_SENSOR_SOC_PACKAGE_PWR: %.2f Watts\n", fvalue);
  }

  // Discrete Sensors
  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SYSTEM_STATUS, &value)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SYSTEM_STATUS\n");
  } else {
    printf("BIC_SENSOR_SYSTEM_STATUS: 0x%X\n", value);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_PROC_FAIL, &value)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_PROC_FAIL\n");
  } else {
    printf("BIC_SENSOR_PROC_FAIL: 0x%X\n", value);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_SYS_BOOT_STAT, &value)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_SYS_BOOT_STAT\n");
  } else {
    printf("BIC_SENSOR_SYS_BOOT_STAT: 0x%X\n", value);
  }

  if (yellowsub_sensor_read(slot_id, BIC_SENSOR_CPU_DIMM_HOT, &value)) {
    printf("yellowsub_sensor_read failed: BIC_SENSOR_CPU_DIMM_HOT\n");
  } else {
    printf("BIC_SENSOR_CPU_DIMM_HOT: 0x%X\n", value);
  }

  return 0;
}
