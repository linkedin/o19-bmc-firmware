/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
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

#ifndef __YELLOWSUB_SENSOR_H__
#define __YELLOWSUB_SENSOR_H__

#include <stdbool.h>
#include <openbmc/ipmi.h>
#include <openbmc/ipmb.h>
#include <openbmc/obmc-pal.h>
#include <facebook/bic.h>
#include <facebook/yellowsub_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SDR_LEN           64
#define MAX_SENSOR_NUM        0xFF
#define MAX_SENSOR_THRESHOLD  8
#define MAX_RETRIES_SDR_INIT  30
#define THERMAL_CONSTANT      255
#define ERR_NOT_READY         -2

#define FAN_TACH_RPM         "fan%d_speed"
#define FAN_REAR_TACH_RPM    "fan%d_speed2"
/*
 * speed of the 2nd fan in the fan module
 */
#define FAN_1_TACH_RPM         "fan%d_1_speed"
#define FAN_MAX_RPM          23000.0
#define FAN_MIN_RPM          2300.0
#define FAN_FULL_SPEED       96

#define MAX_I2C_NUM    13

enum {
  FAN0 = 1,
  FAN1 = 2,
  FAN2 = 3,
  FAN3 = 4,
  FAN0_1 = 5,
  FAN1_1 = 6,
  FAN2_1 = 7,
  FAN3_1 = 8,
};

typedef struct _sensor_info_t {
  bool valid;
  sdr_full_t sdr;
} sensor_info_t;

// Sensors under Bridge IC
enum {
  BIC_SENSOR_BDE_TEMP = 0x01,
  BIC_SENSOR_SOC_TEMP = 0x05,
  BIC_SENSOR_PCH_TEMP = 0x08,
  BIC_SENSOR_SOC_THERM_MARGIN = 0x09,
  BIC_SENSOR_SYSTEM_STATUS = 0x10, //Discrete
  BIC_SENSOR_SPS_FW_HLTH = 0x17, //Event-only
  BIC_SENSOR_INA230_POWER = 0x29,
  BIC_SENSOR_INA230_VOL = 0x2A,
  BIC_SENSOR_POST_ERR = 0x2B, //Event-only
  BIC_SENSOR_SOC_PACKAGE_PWR = 0x2C,
  BIC_SENSOR_SOC_TJMAX = 0x30,
  BIC_SENSOR_POWER_THRESH_EVENT = 0x3B, //Event-only
  BIC_SENSOR_MACHINE_CHK_ERR = 0x40, //Event-only
  BIC_SENSOR_PCIE_ERR = 0x41, //Event-only
  BIC_SENSOR_OTHER_IIO_ERR = 0x43, //Event-only
  BIC_SENSOR_PROC_HOT_EXT = 0x51, //Event-only
  BIC_SENSOR_POWER_ERR = 0x56, //Event-only
  BIC_SENSOR_MEM_ECC_ERR = 0x63, //Event-only
  BIC_SENSOR_PROC_FAIL = 0x65, //Discrete
  BIC_SENSOR_SYS_BOOT_STAT = 0x7E, //Discrete
  BIC_SENSOR_CPU_DIMM_HOT = 0xB3, //Discrete
  BIC_SENSOR_SOC_DIMMA0_TEMP = 0xB4,
  BIC_SENSOR_SOC_DIMMA1_TEMP = 0xB5,
  BIC_SENSOR_SOC_DIMMB0_TEMP = 0xB6,
  BIC_SENSOR_SOC_DIMMB1_TEMP = 0xB7,
  BIC_SENSOR_P3V3_MB = 0xD0,
  BIC_SENSOR_P12V_MB = 0xD2,
  BIC_SENSOR_P1V05_PCH = 0xD3,
  BIC_SENSOR_P3V3_STBY_MB = 0xD5,
  BIC_SENSOR_P5V_STBY_MB = 0xD6,
  BIC_SENSOR_PV_BAT = 0xD7,
  BIC_SENSOR_PVDDR = 0xD8,
  BIC_SENSOR_PVCC_GBE = 0xD9,
  BIC_SENSOR_CAT_ERR = 0xEB, //Event-only
};

// Sensors on Main board
enum {
  MB_SENSOR_INLET_TEMP = 0x81,
  MB_SENSOR_OUTLET_TEMP = 0x80,
  MB_SENSOR_MEZZ_TEMP = 0x82,
  MB_SENSOR_ZONE1_TEMP = 0x83,
  MB_SENSOR_ZONE2_TEMP = 0x84,
  MB_SENSOR_ZONE3_TEMP = 0x85,
  MB_SENSOR_FAN0_TACH = 0x46,
  MB_SENSOR_FAN1_TACH = 0x47,
  MB_SENSOR_FAN2_TACH = 0x48,
  MB_SENSOR_FAN3_TACH = 0x49,
  MB_SENSOR_AIR_FLOW = 0x4A,
  MB_SENSOR_VDD5V_AUX = 0xE0,
  MB_SENSOR_VDD1_2V_STBY = 0xE1,
  MB_SENSOR_VDD1_5V_STBY = 0xE2,
  MB_SENSOR_VDD2_5V_STBY = 0xE3,
  MB_SENSOR_VDD5V_STBY = 0xE4,
  MB_SENSOR_VDD3_3V_STBY = 0xE5,
  MB_SENSOR_VDD12V_SNS_1 = 0xE7,
  MB_SENSOR_HSC_IN_VOLT = 0xC0,
  MB_SENSOR_HSC_OUT_CURR = 0xC1,
  MB_SENSOR_HSC_TEMP = 0xC2,
  MB_SENSOR_HSC_IN_POWER = 0xC3,
  MB_SENSOR_FAN0_1_TACH = 0x4B,
  MB_SENSOR_FAN1_1_TACH = 0x4C,
  MB_SENSOR_FAN2_1_TACH = 0x4D,
  MB_SENSOR_FAN3_1_TACH = 0x4E,
  MB_SENSOR_NVME0_CPU0 = 0x51,
  MB_SENSOR_NVME1_CPU0 = 0x52,
  MB_SENSOR_NVME0_CPU1 = 0x53,
  MB_SENSOR_NVME1_CPU1 = 0x54,
  MB_SENSOR_NVME0_CPU2 = 0x55,
  MB_SENSOR_NVME1_CPU2 = 0x56,
  MB_SENSOR_NVME0_CPU3 = 0x57,
  MB_SENSOR_NVME1_CPU3 = 0x58,
};

enum{
  MEZZ_SENSOR_TEMP = 0x82,
};
extern const uint8_t bic_sensor_list[];

extern const uint8_t bic_discrete_list[];

extern const uint8_t mb_sensor_list[];

extern const uint8_t nic_sensor_list[];

//extern float spb_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];

//extern float nic_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1];

extern size_t bic_sensor_cnt;

extern size_t bic_discrete_cnt;

extern size_t mb_sensor_cnt;

extern size_t nic_sensor_cnt;

int yellowsub_sensor_read(uint8_t fru, uint8_t sensor_num, void *value);
int yellowsub_sensor_name(uint8_t fru, uint8_t sensor_num, char *name);
int yellowsub_sensor_units(uint8_t fru, uint8_t sensor_num, char *units);
int yellowsub_sensor_sdr_path(uint8_t fru, char *path);
int yellowsub_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, float *value);
int yellowsub_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo);
int read_fan_value(const int fan, const char *device, float *value);
int yellowsub_mux_sel_channel(uint8_t i2c_bus, uint8_t mux_addr, uint8_t channel);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __YELLOWSUB_SENSOR_H__ */
