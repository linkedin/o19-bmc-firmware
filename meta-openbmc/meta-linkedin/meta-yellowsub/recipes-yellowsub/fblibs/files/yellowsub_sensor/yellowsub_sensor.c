/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This file contains code to support IPMI2.0 Specificaton available @
 * http://www.intel.com/content/www/us/en/servers/ipmi/ipmi-specifications.html
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
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <dirent.h>
#include <unistd.h>
#include <openbmc/obmc-i2c.h>
#include "yellowsub_sensor.h"

#define LARGEST_DEVICE_NAME 120

#define GPIO_VAL "/sys/class/gpio/gpio%d/value"

#define I2C_BUS_1_DIR "/sys/class/i2c-adapter/i2c-1/"
#define I2C_BUS_3_DIR "/sys/class/i2c-adapter/i2c-3/"
#define I2C_BUS_9_DIR "/sys/class/i2c-adapter/i2c-9/"
#define I2C_BUS_11_DIR "/sys/class/i2c-adapter/i2c-11/"
#define I2C_BUS_10_DIR "/sys/class/i2c-adapter/i2c-10/"
#define I2C_BUS_2_DIR "/sys/class/i2c-adapter/i2c-2/2-003a"

#define TACH_DIR "/sys/class/i2c-adapter/i2c-12/12-0031"

#define MB_INLET_TEMP_DEVICE I2C_BUS_3_DIR "3-0049"
#define MB_OUTLET_TEMP_DEVICE I2C_BUS_3_DIR "3-004c"
#define MB_ZONE1_TEMP_DEVICE I2C_BUS_3_DIR "3-004a"
#define MB_ZONE2_TEMP_DEVICE I2C_BUS_3_DIR "3-0048"
#define MB_ZONE3_TEMP_DEVICE I2C_BUS_3_DIR "3-004b"

#define MEZZ_TEMP_DEVICE I2C_BUS_1_DIR "1-001f"

#define HSC_DEVICE I2C_BUS_11_DIR "11-0010"

#define VMON_VALUE "in%d_input"
#define HSC_IN_VOLT "in1_input"
#define HSC_OUT_CURR "curr1_input"
#define HSC_TEMP "temp1_input"
#define HSC_IN_POWER "power1_input"

#define UNIT_DIV 1000
#define FAN_FACTOR 150

#define I2C_DEV_NIC "/dev/i2c-1"
#define I2C_NIC_ADDR 0x1f
#define I2C_NIC_SENSOR_TEMP_REG 0x02

#define BIC_SENSOR_READ_NA 0x20

#define MAX_SENSOR_NUM 0xFF
#define ALL_BYTES 0xFF
#define LAST_REC_ID 0xFFFF

#define YELLOWSUB_SDR_PATH "/tmp/sdr_%s.bin"

/*
 * NVMe MUXes
 */
#define I2C_DEV_NVME                "/dev/i2c-10"
#define NVME_I2C_BUS_NUM            10
#define NVME_MUX_NUM                4
#define  NVME_I2C_MGMT_DATA_ADDR    0x6a
#define  NVME_I2C_TEMP_OFFSET       0x3
#define  NVME0_TEMP_UCR_THRESH      70
#define  NVME_I2C_INTF_ADDR         0x1b
#define  NVME_TEMP_ADDR             0x5

uint8_t  nvme_mux_addr[] = {0x70, 0x71, 0x72, 0x73};

typedef enum {
    NVME_CHANNEL_DISABLE = 0,
    NVME_CHANNEL1 = 1,
    NVME_CHANNEL2 = 2,
} nvme_channel;

// List of BIC sensors to be monitored
const uint8_t bic_sensor_list[] = {
  /* Threshold sensors */
  BIC_SENSOR_BDE_TEMP,
  BIC_SENSOR_SOC_TEMP,
  BIC_SENSOR_PCH_TEMP,
  BIC_SENSOR_SOC_THERM_MARGIN,
  BIC_SENSOR_INA230_POWER,
  BIC_SENSOR_INA230_VOL,
  BIC_SENSOR_SOC_PACKAGE_PWR,
  BIC_SENSOR_SOC_TJMAX,
  BIC_SENSOR_SOC_DIMMA0_TEMP,
  BIC_SENSOR_SOC_DIMMA1_TEMP,
  BIC_SENSOR_SOC_DIMMB0_TEMP,
  BIC_SENSOR_SOC_DIMMB1_TEMP,
  BIC_SENSOR_P3V3_MB,
  BIC_SENSOR_P12V_MB,
  BIC_SENSOR_P1V05_PCH,
  BIC_SENSOR_P3V3_STBY_MB,
  BIC_SENSOR_P5V_STBY_MB,
  BIC_SENSOR_PV_BAT,
  BIC_SENSOR_PVDDR,
  BIC_SENSOR_PVCC_GBE,
};

const uint8_t bic_discrete_list[] = {
  /* Discrete sensors */
  BIC_SENSOR_SYSTEM_STATUS,
  BIC_SENSOR_CPU_DIMM_HOT,
};

// List of MB sensors to be monitored
const uint8_t mb_sensor_list[] = {
  MB_SENSOR_INLET_TEMP,
  MB_SENSOR_OUTLET_TEMP,
  MB_SENSOR_ZONE1_TEMP,
  MB_SENSOR_ZONE2_TEMP,
  MB_SENSOR_ZONE3_TEMP,
  MB_SENSOR_FAN0_TACH,
  MB_SENSOR_FAN1_TACH,
  MB_SENSOR_FAN2_TACH,
  MB_SENSOR_FAN3_TACH,
  MB_SENSOR_VDD5V_AUX,
  MB_SENSOR_VDD1_2V_STBY,
  MB_SENSOR_VDD1_5V_STBY,
  MB_SENSOR_VDD2_5V_STBY,
  MB_SENSOR_VDD5V_STBY,
  MB_SENSOR_VDD3_3V_STBY,
  MB_SENSOR_HSC_IN_VOLT,
  MB_SENSOR_HSC_OUT_CURR,
  MB_SENSOR_HSC_IN_POWER,
  MB_SENSOR_FAN0_1_TACH,
  MB_SENSOR_FAN1_1_TACH,
  MB_SENSOR_FAN2_1_TACH,
  MB_SENSOR_FAN3_1_TACH,
  MB_SENSOR_NVME0_CPU0,
  MB_SENSOR_NVME1_CPU0,
  MB_SENSOR_NVME0_CPU1,
  MB_SENSOR_NVME1_CPU1,
  MB_SENSOR_NVME0_CPU2,
  MB_SENSOR_NVME1_CPU2,
  MB_SENSOR_NVME0_CPU3,
  MB_SENSOR_NVME1_CPU3,
};

// List of NIC sensors to be monitored
const uint8_t nic_sensor_list[] = {
  MEZZ_SENSOR_TEMP,
};

float mb_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1] = {0};
float nic_sensor_threshold[MAX_SENSOR_NUM][MAX_SENSOR_THRESHOLD + 1] = {0};

static void
sensor_thresh_array_init() {
  static bool init_done = false;

  if (init_done)
    return;

  mb_sensor_threshold[MB_SENSOR_INLET_TEMP][UCR_THRESH] = 40;
  mb_sensor_threshold[MB_SENSOR_OUTLET_TEMP][UCR_THRESH] = 70;
  mb_sensor_threshold[MB_SENSOR_ZONE1_TEMP][UCR_THRESH] = 70;
  mb_sensor_threshold[MB_SENSOR_ZONE2_TEMP][UCR_THRESH] = 70;
  mb_sensor_threshold[MB_SENSOR_ZONE3_TEMP][UCR_THRESH] = 70;
  mb_sensor_threshold[MB_SENSOR_FAN0_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN0_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN1_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN1_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN2_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN2_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN3_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN3_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_VDD5V_AUX][UCR_THRESH] = 5.493;
  mb_sensor_threshold[MB_SENSOR_VDD5V_AUX][LCR_THRESH] = 4.501;
  mb_sensor_threshold[MB_SENSOR_VDD1_2V_STBY][UCR_THRESH] = 1.236;
  mb_sensor_threshold[MB_SENSOR_VDD1_2V_STBY][LCR_THRESH] = 1.164;
  mb_sensor_threshold[MB_SENSOR_VDD1_5V_STBY][UCR_THRESH] = 1.545;
  mb_sensor_threshold[MB_SENSOR_VDD1_5V_STBY][LCR_THRESH] = 1.455;
  mb_sensor_threshold[MB_SENSOR_VDD2_5V_STBY][UCR_THRESH] = 2.575;
  mb_sensor_threshold[MB_SENSOR_VDD2_5V_STBY][LCR_THRESH] = 2.425;
  mb_sensor_threshold[MB_SENSOR_VDD5V_STBY][UCR_THRESH] = 5.15;
  mb_sensor_threshold[MB_SENSOR_VDD5V_STBY][LCR_THRESH] = 4.85;
  mb_sensor_threshold[MB_SENSOR_VDD3_3V_STBY][UCR_THRESH] = 3.45;
  mb_sensor_threshold[MB_SENSOR_VDD3_3V_STBY][LCR_THRESH] = 3.20;
  mb_sensor_threshold[MB_SENSOR_VDD12V_SNS_1][UCR_THRESH] = 12.36;
  mb_sensor_threshold[MB_SENSOR_VDD12V_SNS_1][LCR_THRESH] = 11.64;
  mb_sensor_threshold[MB_SENSOR_HSC_IN_VOLT][UCR_THRESH] = 13.2;
  mb_sensor_threshold[MB_SENSOR_HSC_IN_VOLT][LCR_THRESH] = 10.8;
  mb_sensor_threshold[MB_SENSOR_HSC_OUT_CURR][UCR_THRESH] = 47.705;
  mb_sensor_threshold[MB_SENSOR_HSC_TEMP][UCR_THRESH] = 120;
  mb_sensor_threshold[MB_SENSOR_HSC_IN_POWER][UCR_THRESH] = 525;
  mb_sensor_threshold[MB_SENSOR_FAN0_1_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN0_1_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN1_1_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN1_1_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN2_1_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN2_1_TACH][LCR_THRESH] = 500;
  mb_sensor_threshold[MB_SENSOR_FAN3_1_TACH][UCR_THRESH] = 25300;
  mb_sensor_threshold[MB_SENSOR_FAN3_1_TACH][LCR_THRESH] = 500;

  mb_sensor_threshold[MB_SENSOR_NVME0_CPU0][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME1_CPU0][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME0_CPU1][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME1_CPU1][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME0_CPU2][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME1_CPU2][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME0_CPU3][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;
  mb_sensor_threshold[MB_SENSOR_NVME1_CPU3][UCR_THRESH] = NVME0_TEMP_UCR_THRESH;

  nic_sensor_threshold[MEZZ_SENSOR_TEMP][UCR_THRESH] = 95;

  init_done = true;
}

size_t bic_sensor_cnt = sizeof(bic_sensor_list)/sizeof(uint8_t);

size_t bic_discrete_cnt = sizeof(bic_discrete_list)/sizeof(uint8_t);

size_t mb_sensor_cnt = sizeof(mb_sensor_list)/sizeof(uint8_t);

size_t nic_sensor_cnt = sizeof(nic_sensor_list)/sizeof(uint8_t);

enum {
  VMON_PIN0 = 0,
  VMON_PIN1,
  VMON_PIN2,
  VMON_PIN3,
  VMON_PIN4,
  VMON_PIN5,
  VMON_PIN6,
  VMON_PIN7,
  VMON_PIN8,
  VMON_PIN9,
};

static sensor_info_t g_sinfo[MAX_NUM_FRUS][MAX_SENSOR_NUM] = {0};

static int
read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;

#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);

  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  } else {
    return 0;
  }
}

static int
read_device_float(const char *device, float *value) {
  FILE *fp;
  int rc;
  char tmp[10];

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  }

  *value = atof(tmp);

  return 0;
}

static int
get_current_dir(const char *device, char *dir_name) {
  char full_name[LARGEST_DEVICE_NAME + 1];
  DIR *dir = NULL;
  struct dirent *ent;

  snprintf(full_name, sizeof(full_name), "%s/hwmon", device);
  dir = opendir(full_name);
  if (dir == NULL) {
    goto close_dir_out;
  }
  while ((ent = readdir(dir)) != NULL) {
    if (strstr(ent->d_name, "hwmon")) {
      // found the correct 'hwmon??' directory
      snprintf(dir_name, sizeof(full_name), "%s/hwmon/%s/",
      device, ent->d_name);
      goto close_dir_out;
    }
  }

close_dir_out:
  if (dir != NULL) {
    if (closedir(dir)) {
      syslog(LOG_ERR, "%s closedir failed, errno=%s\n",
              __FUNCTION__, strerror(errno));
    }
  }
  return 0;
}

int
yellowsub_mux_sel_channel(uint8_t i2c_bus, uint8_t mux_addr, uint8_t channel) {
  int ret;
  int dev, retry;
  char bus[LARGEST_DEVICE_NAME] = "";

  if(i2c_bus > MAX_I2C_NUM) {
    syslog(LOG_ERR, "%s: Wrong I2C bus number\n", __func__);
    return -1;
  }

  snprintf(bus, LARGEST_DEVICE_NAME, "/dev/i2c-%d", i2c_bus);

  dev = open(bus, O_RDWR);
  if (dev < 0) {
    syslog(LOG_ERR, "%s: open() failed", __func__);
    return -1;
  }

  /* Assign the i2c device address */
  ret = ioctl(dev, I2C_SLAVE, mux_addr);
  if (ret < 0) {
    syslog(LOG_ERR, "%s: ioctl() assigning i2c addr failed", __func__);
    close(dev);
    return -1;
  }

  ret = i2c_smbus_write_byte(dev, channel);
  retry = 0;
  while ((retry < 5) && (ret < 0)) {
    usleep(10000);
    ret = i2c_smbus_write_byte(dev, channel);
    if (ret < 0)
      retry++;
    else
      break;
  }

  if (ret < 0) {
    close(dev);
    syslog(LOG_ERR, "%s: i2c_smbus_write_byte failed", __func__);
    return -1;
  }

  close(dev);

  return 0;
}

void
yellowsub_set_nvme_channel(uint8_t nvme_senId)
{
    uint16_t   i2c_addr;
    uint8_t    nvme_channel;

    /*
     * close all channels
     */
    for (int i = 0; i < NVME_MUX_NUM; i++) {
        yellowsub_mux_sel_channel(NVME_I2C_BUS_NUM,
                                  nvme_mux_addr[i], NVME_CHANNEL_DISABLE);
    }

    /*
     * Enable NVMe channel
     */
    switch(nvme_senId) {
    case MB_SENSOR_NVME0_CPU0:
        i2c_addr = nvme_mux_addr[0];
        nvme_channel = 1;
        break;
    case MB_SENSOR_NVME1_CPU0:
        i2c_addr = nvme_mux_addr[0];
        nvme_channel = 2;
        break;
    case MB_SENSOR_NVME0_CPU1:
        i2c_addr = nvme_mux_addr[1];
        nvme_channel = 1;
        break;
    case MB_SENSOR_NVME1_CPU1:
        i2c_addr = nvme_mux_addr[1];
        nvme_channel = 2;
        break;
    case MB_SENSOR_NVME0_CPU2:
        i2c_addr = nvme_mux_addr[2];
        nvme_channel = 1;
        break;
    case MB_SENSOR_NVME1_CPU2:
        i2c_addr = nvme_mux_addr[2];
        nvme_channel = 2;
        break;
    case MB_SENSOR_NVME0_CPU3:
        i2c_addr = nvme_mux_addr[3];
        nvme_channel = 1;
        break;
    case MB_SENSOR_NVME1_CPU3:
        i2c_addr = nvme_mux_addr[3];
        nvme_channel = 2;
        break;
    default:
        break;
    }

    /*
     *  Enable the NMVe on one i2c MUX, and set the channel
     */
    yellowsub_mux_sel_channel(NVME_I2C_BUS_NUM,
                              i2c_addr, nvme_channel);
}

static int
read_temp_value(char *device, uint8_t addr, uint8_t type, float *value) {
    int dev,fan;
    int ret,rc;
    int32_t res;
    float rpm_avg = 0, rpm_sum = 0;
    float rpm_val;
    int retry = 0;

    dev = open(device, O_RDWR);
    if (dev < 0) {
        syslog(LOG_ERR, "read_temp_value: open() failed");
        return -1;
    }

    /* Assign the i2c device address */
    ret = ioctl(dev, I2C_SLAVE, addr);
    if (ret < 0) {
        syslog(LOG_ERR, "read_temp_value: ioctl() assigning i2c addr failed");
        close(dev);
        return -1;
    }

    /* Read the Temperature Register result based on whether it is internal or external sensor */
    res = i2c_smbus_read_byte_data(dev, type);
    retry = 0;
    while ((retry < 5) && (res < 0)) {
        usleep(10000);
        res = i2c_smbus_read_byte_data(dev, type);
        if (res < 0)
            retry++;
        else
            break;
    }

    if (res < 0) {
        syslog(LOG_ERR, "read_temp_value: read i2c failed");
        close(dev);
        return -1;
    }

    *value = (float) res;
    close(dev);
    return 0;
}

static int
read_temp_attr(const char *device, const char *attr, float *value) {
  char full_dir_name[LARGEST_DEVICE_NAME + 1];
  char dir_name[LARGEST_DEVICE_NAME + 1];
  int tmp;

  // Get current working directory
  if (get_current_dir(device, dir_name))
  {
    return -1;
  }
  snprintf(
      full_dir_name, LARGEST_DEVICE_NAME, "%s/%s", dir_name, attr);


  if (read_device(full_dir_name, &tmp)) {
     return -1;
  }

  *value = ((float)tmp)/UNIT_DIV;

  return 0;
}

static int
read_temp(const char *device, float *value) {
  return read_temp_attr(device, "temp1_input", value);
}

int
read_fan_value(const int fan, const char *device, float *value) {
  FILE *fp;
  int rc;
  char tmp[10];
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int  fan_value;

  snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", TACH_DIR, device_name);

  fp = fopen(full_name, "r");
  if (!fp) {
    int err = errno;
#ifdef DEBUG
    syslog(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1) {
#ifdef DEBUG
    syslog(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  }

  fan_value = (int)strtol(tmp, NULL, 0);
  *value = fan_value * FAN_FACTOR;

  return 0;
}

static int
read_vmon_value(const int pin, const char *device, float *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int  vmon_value;
  int  status;

  snprintf(device_name, LARGEST_DEVICE_NAME, device, pin);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", I2C_BUS_2_DIR, device_name);
  status = read_device(full_name, &vmon_value);

  *value = (float) vmon_value /1000.0;

  return status;
}

/*
static int
read_adc_value(const int pin, const char *device, float *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];

  snprintf(device_name, LARGEST_DEVICE_NAME, device, pin);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", ADC_DIR, device_name);
  return read_device_float(full_name, value);
}
*/

static int
read_hsc_value(const char* attr, const char *device, float *value) {
  char full_dir_name[LARGEST_DEVICE_NAME];
  char dir_name[LARGEST_DEVICE_NAME + 1];
  int tmp;

  // Get current working directory
  if (get_current_dir(device, dir_name))
  {
    return -1;
  }
  snprintf(
      full_dir_name, LARGEST_DEVICE_NAME, "%s/%s", dir_name, attr);

  if(read_device(full_dir_name, &tmp)) {
    return -1;
  }

  *value = ((float) tmp)/UNIT_DIV;

  return 0;
}

static int
read_nic_temp(const char* device, float *value) {
    return read_temp_attr(device, "temp2_input", value);
}

static int
read_nvme_value(const int nvme, float *value)
{
   int res;

   res = read_temp_value(I2C_DEV_NVME, NVME_I2C_INTF_ADDR, NVME_TEMP_ADDR, value);

   if (res < 0) {
       /*
        * Additional Temperature sensor info provided via SMBus command at 0x6A
        */
       res = read_temp_value(I2C_DEV_NVME, NVME_I2C_MGMT_DATA_ADDR, NVME_I2C_TEMP_OFFSET, value);
   }

   return res;
}

static int
bic_read_sensor_wrapper(uint8_t fru, uint8_t sensor_num, bool discrete,
    void *value) {

  int ret;
  sdr_full_t *sdr;
  ipmi_sensor_reading_t sensor;

  ret = bic_read_sensor(fru, sensor_num, &sensor);
  if (ret) {
    return ret;
  }

  if (sensor.flags & BIC_SENSOR_READ_NA) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_read_sensor_wrapper: Reading Not Available");
    syslog(LOG_ERR, "bic_read_sensor_wrapper: sensor_num: 0x%X, flag: 0x%X",
        sensor_num, sensor.flags);
#endif
    return -1;
  }

  if (discrete) {
    *(float *) value = (float) sensor.status;
    return 0;
  }

  sdr = &g_sinfo[fru-1][sensor_num].sdr;

  // If the SDR is not type1, no need for conversion
  if (sdr->type !=1) {
    *(float *) value = sensor.value;
    return 0;
  }

  // y = (mx + b * 10^b_exp) * 10^r_exp
  uint8_t x;
  uint8_t m_lsb, m_msb, m;
  uint8_t b_lsb, b_msb, b;
  int8_t b_exp, r_exp;

  x = sensor.value;

  m_lsb = sdr->m_val;
  m_msb = sdr->m_tolerance >> 6;
  m = (m_msb << 8) | m_lsb;

  b_lsb = sdr->b_val;
  b_msb = sdr->b_accuracy >> 6;
  b = (b_msb << 8) | b_lsb;

  // exponents are 2's complement 4-bit number
  b_exp = sdr->rb_exp & 0xF;
  if (b_exp > 7) {
    b_exp = (~b_exp + 1) & 0xF;
    b_exp = -b_exp;
  }
  r_exp = (sdr->rb_exp >> 4) & 0xF;
  if (r_exp > 7) {
    r_exp = (~r_exp + 1) & 0xF;
    r_exp = -r_exp;
  }

  //printf("m:%d, x:%d, b:%d, b_exp:%d, r_exp:%d\n", m, x, b, b_exp, r_exp);

  * (float *) value = ((m * x) + (b * pow(10, b_exp))) * (pow(10, r_exp));

  if ((sensor_num == BIC_SENSOR_SOC_THERM_MARGIN) && (* (float *) value > 0)) {
   * (float *) value -= (float) THERMAL_CONSTANT;
  }

  return 0;
}

int
yellowsub_sensor_sdr_path(uint8_t fru, char *path) {

char fru_name[16] = {0};

switch(fru) {
  case FRU_SLOT1:
    sprintf(fru_name, "%s", "slot1");
    break;
  case FRU_SLOT2:
    sprintf(fru_name, "%s", "slot2");
    break;
  case FRU_SLOT3:
    sprintf(fru_name, "%s", "slot3");
    break;
  case FRU_SLOT4:
    sprintf(fru_name, "%s", "slot4");
    break;
  case FRU_MB:
    sprintf(fru_name, "%s", "mb");
    break;
  case FRU_NIC:
    sprintf(fru_name, "%s", "nic");
    break;
  default:
#ifdef DEBUG
    syslog(LOG_WARNING, "yellowsub_sensor_sdr_path: Wrong Slot ID\n");
#endif
    return -1;
}

sprintf(path, YELLOWSUB_SDR_PATH, fru_name);

if (access(path, F_OK) == -1) {
  return -1;
}

return 0;
}

/* Populates all sensor_info_t struct using the path to SDR dump */
int
sdr_init(char *path, sensor_info_t *sinfo) {
int fd;
uint8_t buf[MAX_SDR_LEN] = {0};
uint8_t bytes_rd = 0;
uint8_t snr_num = 0;
sdr_full_t *sdr;

while (access(path, F_OK) == -1) {
  sleep(5);
}

fd = open(path, O_RDONLY);
if (fd < 0) {
  syslog(LOG_ERR, "sdr_init: open failed for %s\n", path);
  return -1;
}

while ((bytes_rd = read(fd, buf, sizeof(sdr_full_t))) > 0) {
  if (bytes_rd != sizeof(sdr_full_t)) {
    syslog(LOG_ERR, "sdr_init: read returns %d bytes\n", bytes_rd);
    return -1;
  }

  sdr = (sdr_full_t *) buf;
  snr_num = sdr->sensor_num;
  sinfo[snr_num].valid = true;
  memcpy(&sinfo[snr_num].sdr, sdr, sizeof(sdr_full_t));
}

return 0;
}

int
yellowsub_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo) {
  int fd;
  uint8_t buf[MAX_SDR_LEN] = {0};
  uint8_t bytes_rd = 0;
  uint8_t sn = 0;
  char path[64] = {0};

  switch(fru) {
    case FRU_SLOT1:
    case FRU_SLOT2:
    case FRU_SLOT3:
    case FRU_SLOT4:
      if (yellowsub_sensor_sdr_path(fru, path) < 0) {
#ifdef DEBUG
        syslog(LOG_WARNING, "yellowsub_sensor_sdr_init: get_fru_sdr_path failed\n");
#endif
        return ERR_NOT_READY;
      }

      if (sdr_init(path, sinfo) < 0) {
#ifdef DEBUG
        syslog(LOG_ERR, "yellowsub_sensor_sdr_init: sdr_init failed for FRU %d", fru);
#endif
      }
      break;

    case FRU_MB:
    case FRU_NIC:
      return -1;
      break;
  }

  return 0;
}

static int
yellowsub_sdr_init(uint8_t fru) {

  static bool init_done[MAX_NUM_FRUS] = {false};

  if (!init_done[fru - 1]) {

    sensor_info_t *sinfo = g_sinfo[fru-1];

    if (yellowsub_sensor_sdr_init(fru, sinfo) < 0)
      return ERR_NOT_READY;

    init_done[fru - 1] = true;
  }

  return 0;
}

static bool
is_server_prsnt(uint8_t fru) {
  /*
   * For Yellowsub, 4 host present, no GPIO PINs.
   */
  return 1;
}

/* Get the units for the sensor */
int
yellowsub_sensor_units(uint8_t fru, uint8_t sensor_num, char *units) {
  uint8_t op, modifier;
  sensor_info_t *sinfo;

    if (is_server_prsnt(fru) && (yellowsub_sdr_init(fru) != 0) &&
        (fru != FRU_MB) && (fru != FRU_NIC)) {
      syslog(LOG_ERR, "yellowsub_sensor_units failed for FRU %d", fru);
      return -1;
    }

  switch(fru) {
    case FRU_SLOT1:
    case FRU_SLOT2:
    case FRU_SLOT3:
    case FRU_SLOT4:
      sprintf(units, "");
      break;

    case FRU_MB:
      switch(sensor_num) {
        case MB_SENSOR_INLET_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_OUTLET_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_ZONE1_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_ZONE2_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_ZONE3_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_FAN0_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN1_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN2_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN3_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN0_1_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN1_1_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN2_1_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_FAN3_1_TACH:
          sprintf(units, "RPM");
          break;
        case MB_SENSOR_VDD5V_AUX:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_VDD1_2V_STBY:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_VDD1_5V_STBY:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_VDD2_5V_STBY:
        case MB_SENSOR_VDD5V_STBY:
        case MB_SENSOR_VDD3_3V_STBY:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_VDD12V_SNS_1:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_HSC_IN_VOLT:
          sprintf(units, "Volts");
          break;
        case MB_SENSOR_HSC_OUT_CURR:
          sprintf(units, "Amps");
          break;
        case MB_SENSOR_HSC_TEMP:
          sprintf(units, "C");
          break;
        case MB_SENSOR_HSC_IN_POWER:
          sprintf(units, "Watts");
          break;
        case MB_SENSOR_NVME0_CPU0:
        case MB_SENSOR_NVME1_CPU0:
        case MB_SENSOR_NVME0_CPU1:
        case MB_SENSOR_NVME1_CPU1:
        case MB_SENSOR_NVME0_CPU2:
        case MB_SENSOR_NVME1_CPU2:
        case MB_SENSOR_NVME0_CPU3:
        case MB_SENSOR_NVME1_CPU3:
          sprintf(units, "C");
          break;
      }
      break;
    case FRU_NIC:
      switch(sensor_num) {
        case MEZZ_SENSOR_TEMP:
          sprintf(units, "C");
          break;
      }
      break;
  }
  return 0;
}

int
yellowsub_sensor_threshold(uint8_t fru, uint8_t sensor_num, uint8_t thresh, float *value) {

  sensor_thresh_array_init();

  switch(fru) {
    case FRU_SLOT1:
    case FRU_SLOT2:
    case FRU_SLOT3:
    case FRU_SLOT4:
      break;
    case FRU_MB:
      *value = mb_sensor_threshold[sensor_num][thresh];
      break;
    case FRU_NIC:
      *value = nic_sensor_threshold[sensor_num][thresh];
      break;
  }
  return 0;
}

/* Get the name for the sensor */
int
yellowsub_sensor_name(uint8_t fru, uint8_t sensor_num, char *name) {

  switch(fru) {
    case FRU_SLOT1:
    case FRU_SLOT2:
    case FRU_SLOT3:
    case FRU_SLOT4:
      switch(sensor_num) {
        case BIC_SENSOR_SYSTEM_STATUS:
          sprintf(name, "SYSTEM_STATUS");
          break;
        case BIC_SENSOR_SYS_BOOT_STAT:
          sprintf(name, "SYS_BOOT_STAT");
          break;
        case BIC_SENSOR_CPU_DIMM_HOT:
          sprintf(name, "CPU_DIMM_HOT");
          break;
        case BIC_SENSOR_PROC_FAIL:
          sprintf(name, "PROC_FAIL");
          break;
        default:
          sprintf(name, "");
          break;
      }
      break;

    case FRU_MB:
      switch(sensor_num) {
        case MB_SENSOR_INLET_TEMP:
          sprintf(name, "MB_INLET_TEMP");
          break;
        case MB_SENSOR_OUTLET_TEMP:
          sprintf(name, "MB_OUTLET_TEMP");
          break;
        case MB_SENSOR_ZONE1_TEMP:
          sprintf(name, "MB_ZONE1_TEMP");
          break;
        case MB_SENSOR_ZONE2_TEMP:
          sprintf(name, "MB_ZONE2_TEMP");
          break;
        case MB_SENSOR_ZONE3_TEMP:
          sprintf(name, "MB_ZONE3_TEMP");
          break;
        case MB_SENSOR_FAN0_TACH:
          sprintf(name, "MB_FAN0_TACH");
          break;
        case MB_SENSOR_FAN1_TACH:
          sprintf(name, "MB_FAN1_TACH");
          break;
        case MB_SENSOR_FAN2_TACH:
          sprintf(name, "MB_FAN2_TACH");
          break;
        case MB_SENSOR_FAN3_TACH:
          sprintf(name, "MB_FAN3_TACH");
          break;
        case MB_SENSOR_AIR_FLOW:
          sprintf(name, "MB_AIR_FLOW");
          break;
	case MB_SENSOR_VDD5V_AUX:
          sprintf(name, "MB_VDD5V_AUX");
          break;
	case MB_SENSOR_VDD1_2V_STBY :
          sprintf(name, "MB_VDD1_2V_STBY");
          break;
	case MB_SENSOR_VDD1_5V_STBY:
          sprintf(name, "MB_VDD1_5V_STBY");
          break;
	case MB_SENSOR_VDD5V_STBY:
          sprintf(name, "MB_VDD5V_STBY");
          break;
        case MB_SENSOR_VDD3_3V_STBY:
          sprintf(name, "MB_VDD3_3V_STBY");
          break;
        case MB_SENSOR_VDD12V_SNS_1:
          sprintf(name, "MB_VDD12V_SNS_1");
          break;
        case MB_SENSOR_VDD2_5V_STBY:
          sprintf(name, "MB_VDD2_5V_STBY");
          break;
        case MB_SENSOR_HSC_IN_VOLT:
          sprintf(name, "MB_HSC_IN_VOLT");
          break;
        case MB_SENSOR_HSC_OUT_CURR:
          sprintf(name, "MB_HSC_OUT_CURR");
          break;
        case MB_SENSOR_HSC_TEMP:
          sprintf(name, "MB_HSC_TEMP");
          break;
        case MB_SENSOR_HSC_IN_POWER:
          sprintf(name, "MB_HSC_IN_POWER");
          break;
        case MB_SENSOR_FAN0_1_TACH:
          sprintf(name, "MB_FAN0_REAR_TACH");
          break;
        case MB_SENSOR_FAN1_1_TACH:
          sprintf(name, "MB_FAN1_REAR_TACH");
          break;
        case MB_SENSOR_FAN2_1_TACH:
          sprintf(name, "MB_FAN2_REAR_TACH");
          break;
        case MB_SENSOR_FAN3_1_TACH:
          sprintf(name, "MB_FAN3_REAR_TACH");
          break;
        case MB_SENSOR_NVME0_CPU0:
          sprintf(name, "MB_NVME0_CPU0");
          break;
        case MB_SENSOR_NVME1_CPU0:
          sprintf(name, "MB_NVME1_CPU0");
          break;
        case MB_SENSOR_NVME0_CPU1:
          sprintf(name, "MB_NVME0_CPU1");
          break;
        case MB_SENSOR_NVME1_CPU1:
          sprintf(name, "MB_NVME1_CPU1");
          break;
        case MB_SENSOR_NVME0_CPU2:
          sprintf(name, "MB_NVME0_CPU2");
          break;
        case MB_SENSOR_NVME1_CPU2:
          sprintf(name, "MB_NVME1_CPU2");
          break;
        case MB_SENSOR_NVME0_CPU3:
          sprintf(name, "MB_NVME0_CPU3");
          break;
        case MB_SENSOR_NVME1_CPU3:
          sprintf(name, "MB_NVME1_CPU3");
          break;
      }
      break;
    case FRU_NIC:
      switch(sensor_num) {
        case MEZZ_SENSOR_TEMP:
          sprintf(name, "MEZZ_SENSOR_TEMP");
          break;
      }
      break;
  }
  return 0;
}

int
yellowsub_sensor_read(uint8_t fru, uint8_t sensor_num, void *value) {
  float volt;
  float curr;
  int ret;
  bool discrete;
  int i;

  switch (fru) {
    case FRU_SLOT1:
    case FRU_SLOT2:
    case FRU_SLOT3:
    case FRU_SLOT4:
      if (!(is_server_prsnt(fru))) {
        return -1;
      }

      ret = yellowsub_sdr_init(fru);
      if (ret < 0) {
        return ret;
      }

      discrete = false;

      i = 0;
      while (i < bic_discrete_cnt) {
        if (sensor_num == bic_discrete_list[i++]) {
          discrete = true;
          break;
        }
      }

      return bic_read_sensor_wrapper(fru, sensor_num, discrete, value);

    case FRU_MB:
      switch(sensor_num) {

        // Inlet, Outlet Temp
        case MB_SENSOR_INLET_TEMP:
          return read_temp(MB_INLET_TEMP_DEVICE, (float*) value);
        case MB_SENSOR_OUTLET_TEMP:
          return read_temp(MB_OUTLET_TEMP_DEVICE, (float*) value);
        case MB_SENSOR_ZONE1_TEMP:
          return read_temp(MB_ZONE1_TEMP_DEVICE, (float*) value);
        case MB_SENSOR_ZONE2_TEMP:
          return read_temp(MB_ZONE2_TEMP_DEVICE, (float*) value);
        case MB_SENSOR_ZONE3_TEMP:
          return read_temp(MB_ZONE3_TEMP_DEVICE, (float*) value);

        // Fan Tach Values (Front)
        case MB_SENSOR_FAN0_TACH:
          return read_fan_value(FAN0, FAN_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN1_TACH:
          return read_fan_value(FAN1, FAN_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN2_TACH:
          return read_fan_value(FAN2, FAN_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN3_TACH:
          return read_fan_value(FAN3, FAN_TACH_RPM, (float*) value);

        // Fan 2nd Tach Values (Rear)
        case MB_SENSOR_FAN0_1_TACH:
          return read_fan_value(FAN0, FAN_REAR_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN1_1_TACH:
          return read_fan_value(FAN1, FAN_REAR_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN2_1_TACH:
          return read_fan_value(FAN2, FAN_REAR_TACH_RPM, (float*) value);
        case MB_SENSOR_FAN3_1_TACH:
          return read_fan_value(FAN3, FAN_REAR_TACH_RPM, (float*) value);

        // Various Voltages
        case MB_SENSOR_VDD5V_AUX:
          return read_vmon_value(VMON_PIN0, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD1_2V_STBY:
	  return read_vmon_value(VMON_PIN1, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD1_5V_STBY:
          return read_vmon_value(VMON_PIN2, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD2_5V_STBY:
          return read_vmon_value(VMON_PIN3, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD5V_STBY:
          return read_vmon_value(VMON_PIN4, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD3_3V_STBY:
          return read_vmon_value(VMON_PIN5, VMON_VALUE, (float*) value);
        case MB_SENSOR_VDD12V_SNS_1:
          return read_vmon_value(VMON_PIN9, VMON_VALUE, (float*) value);

        // Hot Swap Controller
        case MB_SENSOR_HSC_IN_VOLT:
          return read_hsc_value(HSC_IN_VOLT, HSC_DEVICE, (float*) value);
        case MB_SENSOR_HSC_OUT_CURR:
          return read_hsc_value(HSC_OUT_CURR, HSC_DEVICE, (float*) value);
        case MB_SENSOR_HSC_IN_POWER:
          return read_hsc_value(HSC_IN_POWER, HSC_DEVICE, (float*) value);

        // NVMe temperature
        case MB_SENSOR_NVME0_CPU0:
        case MB_SENSOR_NVME1_CPU0:
        case MB_SENSOR_NVME0_CPU1:
        case MB_SENSOR_NVME1_CPU1:
        case MB_SENSOR_NVME0_CPU2:
        case MB_SENSOR_NVME1_CPU2:
        case MB_SENSOR_NVME0_CPU3:
        case MB_SENSOR_NVME1_CPU3:
            yellowsub_set_nvme_channel(sensor_num);
            return read_nvme_value(sensor_num,  (float*) value);
      }
      break;

    case FRU_NIC:
      switch(sensor_num) {
      // Mezz Temp
        case MEZZ_SENSOR_TEMP:
          return read_nic_temp(MEZZ_TEMP_DEVICE, (float*) value);
      }
      break;
    default:
      syslog(LOG_WARNING, "invalid FRU: %d\n", fru);
      break;
  }
}
