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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "bic.h"
#include <openbmc/edb.h>
#include <openbmc/obmc-i2c.h>

#define FRUID_READ_COUNT_MAX 0x30
#define FRUID_WRITE_COUNT_MAX 0x30
#define IPMB_READ_COUNT_MAX 224
#define IPMB_WRITE_COUNT_MAX 224
#define BIOS_ERASE_PKT_SIZE (64*1024)
#define BIOS_VERIFY_PKT_SIZE (32*1024)
#define SDR_READ_COUNT_MAX 0x1A
#define SIZE_SYS_GUID 16
#define SIZE_IANA_ID 3

#define BIOS_VER_REGION_SIZE (4*1024*1024)
#define BIOS_VER_STR "F09_"
#define Y2_BIOS_VER_STR "YMV2"

#define BIC_UPDATE_RETRIES 12
#define BIC_UPDATE_TIMEOUT 500

#define BIC_FLASH_START 0x8000
#define BIC_PKT_MAX 252

#define BIC_CMD_DOWNLOAD 0x21
#define BIC_CMD_RUN 0x22
#define BIC_CMD_STATUS 0x23
#define BIC_CMD_DATA 0x24

#define CMD_DOWNLOAD_SIZE 11
#define CMD_RUN_SIZE 7
#define CMD_STATUS_SIZE 3
#define CMD_DATA_SIZE 0xFF

#define GPIO_VAL "/sys/class/gpio/gpio%d/value"

#define IMC_VER_SIZE 8

#define SERVER_TYPE_FILE "/tmp/server_type.bin"

#pragma pack(push, 1)
typedef struct _sdr_rec_hdr_t {
  uint16_t rec_id;
  uint8_t ver;
  uint8_t type;
  uint8_t len;
} sdr_rec_hdr_t;
#pragma pack(pop)

const static uint8_t gpio_bic_update_ready[] = { 0, GPIO_I2C_SLOT1_ALERT_N, GPIO_I2C_SLOT2_ALERT_N, GPIO_I2C_SLOT3_ALERT_N, GPIO_I2C_SLOT4_ALERT_N };

// Helper Functions
static void
msleep(int msec) {
  struct timespec req;

  req.tv_sec = 0;
  req.tv_nsec = msec * 1000 * 1000;

  while(nanosleep(&req, &req) == -1 && errno == EINTR) {
    continue;
  }
}

static int
i2c_open(uint8_t bus_id) {
  int fd;
  char fn[32];
  int rc;

  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_id);
  fd = open(fn, O_RDWR);
  if (fd == -1) {
    syslog(LOG_ERR, "Failed to open i2c device %s", fn);
    return -1;
  }

  rc = ioctl(fd, I2C_SLAVE, BRIDGE_SLAVE_ADDR);
  if (rc < 0) {
    syslog(LOG_ERR, "Failed to open slave @ address 0x%x", BRIDGE_SLAVE_ADDR);
    close(fd);
  }

  return fd;
}

static int
i2c_io(int fd, uint8_t *tbuf, uint8_t tcount, uint8_t *rbuf, uint8_t rcount) {
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg[2];
  int n_msg = 0;
  int rc;

  memset(&msg, 0, sizeof(msg));

  if (tcount) {
    msg[n_msg].addr = BRIDGE_SLAVE_ADDR;
    msg[n_msg].flags = 0;
    msg[n_msg].len = tcount;
    msg[n_msg].buf = tbuf;
    n_msg++;
  }

  if (rcount) {
    msg[n_msg].addr = BRIDGE_SLAVE_ADDR;
    msg[n_msg].flags = I2C_M_RD;
    msg[n_msg].len = rcount;
    msg[n_msg].buf = rbuf;
    n_msg++;
  }

  data.msgs = msg;
  data.nmsgs = n_msg;

  rc = ioctl(fd, I2C_RDWR, &data);
  if (rc < 0) {
    syslog(LOG_ERR, "Failed to do raw io");
    return -1;
  }

  return 0;
}

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

static uint8_t
_is_bic_update_ready(uint8_t slot_id) {
  int val;
  char path[64] = {0};

  if (slot_id < 1 || slot_id > 4) {
    return 0;
  }

  sprintf(path, GPIO_VAL, gpio_bic_update_ready[slot_id]);

  if (read_device(path, &val)) {
    return 0;
  }

  if (val == 0x0) {
    return 0;
  } else {
    return 1;
  }
}

// Common IPMB Wrapper function

static int
get_ipmb_bus_id(uint8_t slot_id) {
  int bus_id;

  switch(slot_id) {
  case FRU_SLOT1:
    bus_id = IPMB_BUS_SLOT1;
    break;
  case FRU_SLOT2:
    bus_id = IPMB_BUS_SLOT2;
    break;
  case FRU_SLOT3:
    bus_id = IPMB_BUS_SLOT3;
    break;
  case FRU_SLOT4:
    bus_id = IPMB_BUS_SLOT4;
    break;
  default:
    bus_id = -1;
    break;
  }

  return bus_id;
}

int
bic_ipmb_wrapper(uint8_t slot_id, uint8_t netfn, uint8_t cmd,
                  uint8_t *txbuf, uint16_t txlen,
                  uint8_t *rxbuf, uint8_t *rxlen) {
  ipmb_req_t *req;
  ipmb_res_t *res;
  uint8_t rbuf[MAX_IPMB_RES_LEN] = {0};
  uint8_t tbuf[MAX_IPMB_RES_LEN] = {0};
  uint16_t tlen = 0;
  uint8_t rlen = 0;
  int count = 0;
  int i = 0;
  int ret;
  uint8_t bus_id;

  ret = get_ipmb_bus_id(slot_id);
  if (ret < 0) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_ipmb_wrapper: Wrong Slot ID %d\n", slot_id);
#endif
    return ret;
  }

  bus_id = (uint8_t) ret;

  req = (ipmb_req_t*)tbuf;

  req->res_slave_addr = BRIDGE_SLAVE_ADDR << 1;
  req->netfn_lun = netfn << LUN_OFFSET;
  req->hdr_cksum = req->res_slave_addr +
                  req->netfn_lun;
  req->hdr_cksum = ZERO_CKSUM_CONST - req->hdr_cksum;

  req->req_slave_addr = BMC_SLAVE_ADDR << 1;
  req->seq_lun = 0x00;
  req->cmd = cmd;

  //copy the data to be send
  if (txlen) {
    memcpy(req->data, txbuf, txlen);
  }

  tlen = IPMB_HDR_SIZE + IPMI_REQ_HDR_SIZE + txlen;

  // Invoke IPMB library handler
  lib_ipmb_handle(bus_id, tbuf, tlen, rbuf, &rlen);

  if (rlen == 0) {
#ifdef DEBUG
    syslog(LOG_DEBUG, "bic_ipmb_wrapper: Zero bytes received\n");
#endif
    return -1;
  }

  // Handle IPMB response
  res  = (ipmb_res_t*) rbuf;

  if (res->cc) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_ipmb_wrapper: Completion Code: 0x%X\n", res->cc);
#endif
    return -1;
  }

  // copy the received data back to caller
  *rxlen = rlen - IPMB_HDR_SIZE - IPMI_RESP_HDR_SIZE;
  memcpy(rxbuf, res->data, *rxlen);

  return 0;
}

// Get Self-Test result
int
bic_get_self_test_result(uint8_t slot_id, uint8_t *self_test_result) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_APP_REQ, CMD_APP_GET_SELFTEST_RESULTS, NULL, 0, (uint8_t *) self_test_result, &rlen);

  return ret;
}

// Get Device ID
int
bic_get_dev_id(uint8_t slot_id, ipmi_dev_id_t *dev_id) {
  uint8_t rlen = 0;
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_APP_REQ, CMD_APP_GET_DEVICE_ID, NULL, 0, (uint8_t *) dev_id, &rlen);

  return ret;
}

// Get GPIO value and configuration
int
bic_get_gpio(uint8_t slot_id, bic_gpio_t *gpio) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[12] = {0x00};
  uint8_t rlen = 0;
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_GPIO, tbuf, 3, rbuf, &rlen);

  // Ignore first 3 bytes of IANA ID
  memcpy((uint8_t*) gpio, &rbuf[3], 6);

  return ret;
}

int
bic_get_gpio_raw(uint8_t slot_id, uint8_t *gpio) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[12] = {0x00};
  uint8_t rlen = 0;
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_GPIO, tbuf, 3, rbuf, &rlen);

  // Ignore first 3 bytes of IANA ID
  memcpy((uint8_t*) gpio, &rbuf[3], 6);

  return ret;
}


int
bic_set_gpio(uint8_t slot_id, uint8_t gpio, uint8_t value) {
  uint8_t tbuf[16] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[4] = {0x00};
  uint8_t rlen = 0;
  uint64_t pin;
  int ret;

  pin = 1LL << gpio;

  tbuf[3] = pin & 0xFF;
  tbuf[4] = (pin >> 8) & 0xFF;
  tbuf[5] = (pin >> 16) & 0xFF;
  tbuf[6] = (pin >> 24) & 0xFF;
  tbuf[7] = (pin >> 32) & 0xFF;
  tbuf[8] = (pin >> 40) & 0xFF;

  // Fill the value
  if (value) {
    memset(&tbuf[9], 0xFF, 6);
  } else {
    memset(&tbuf[9] , 0x00, 6);
  }

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_SET_GPIO, tbuf, 15, rbuf, &rlen);

  return ret;
}

int
bic_get_gpio_config(uint8_t slot_id, uint8_t gpio, bic_gpio_config_t *gpio_config) {
  uint8_t tbuf[12] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[8] = {0x00};
  uint8_t rlen = 0;
  uint64_t pin;
  int ret;

  pin = 1LL << gpio;

  tbuf[3] = pin & 0xFF;
  tbuf[4] = (pin >> 8) & 0xFF;
  tbuf[5] = (pin >> 16) & 0xFF;
  tbuf[6] = (pin >> 24) & 0xFF;
  tbuf[7] = (pin >> 32) & 0xFF;
  tbuf[8] = (pin >> 40) & 0xFF;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_GPIO_CONFIG, tbuf, 9, rbuf, &rlen);

  // Ignore IANA ID
  *(uint8_t *) gpio_config = rbuf[3];

  return ret;
}

int
bic_set_gpio_config(uint8_t slot_id, uint8_t gpio, bic_gpio_config_t *gpio_config) {
  uint8_t tbuf[12] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[4] = {0x00};
  uint8_t rlen = 0;
  uint64_t pin;
  int ret;

  pin = 1LL << gpio;

  tbuf[3] = pin & 0xFF;
  tbuf[4] = (pin >> 8) & 0xFF;
  tbuf[5] = (pin >> 16) & 0xFF;
  tbuf[6] = (pin >> 24) & 0xFF;
  tbuf[7] = (pin >> 32) & 0xFF;
  tbuf[8] = (pin >> 40) & 0xFF;
  tbuf[9] = (*(uint8_t *) gpio_config) & 0x1F;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_SET_GPIO_CONFIG, tbuf, 10, rbuf, &rlen);

  return ret;
}

// Get BIC Configuration
int
bic_get_config(uint8_t slot_id, bic_config_t *cfg) {
  uint8_t tbuf[3] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[4] = {0x00};
  uint8_t rlen = 0;
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_CONFIG,
              tbuf, 0x03, rbuf, &rlen);
  // Ignore IANA ID
  *(uint8_t *) cfg = rbuf[3];

  return ret;
}

// Set BIC Configuration
int
bic_set_config(uint8_t slot_id, bic_config_t *cfg) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rlen = 0;
  uint8_t rbuf[4] = {0};
  int ret;

  tbuf[3] = *(uint8_t *) cfg;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_SET_CONFIG,
              tbuf, 0x04, rbuf, &rlen);
  return ret;
}

// Read POST Buffer
int
bic_get_post_buf(uint8_t slot_id, uint8_t *buf, uint8_t *len) {
  uint8_t tbuf[3] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[255] = {0x00};
  uint8_t rlen = 0;
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_POST_BUF, tbuf, 0x03, rbuf, &rlen);

  //Ignore IANA ID
  memcpy(buf, &rbuf[3], rlen-3);

  *len = rlen - 3;

  return ret;
}

// Read Firwmare Versions of various components
int
bic_get_fw_ver(uint8_t slot_id, uint8_t comp, uint8_t *ver) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[16] = {0x00};
  uint8_t rlen = 0;
  int ret;

#ifdef CONFIG_FBY2_RC
  if (comp == FW_ME)
    return get_imc_version(slot_id, ver);
#endif

  // Fill the component for which firmware is requested
  tbuf[3] = comp;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_FW_VER, tbuf, 0x04, rbuf, &rlen);
  // fw version has to be between 2 and 5 bytes based on component
  if (ret || (rlen < 2+SIZE_IANA_ID) || (rlen > 5+SIZE_IANA_ID)) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_get_fw_ver: ret: %d, rlen: %d\n", ret, rlen);
#endif
    return -1;
  }

  //Ignore IANA ID
  memcpy(ver, &rbuf[3], rlen-3);

  return ret;
}

// Read checksum of various components
int
bic_get_fw_cksum(uint8_t slot_id, uint8_t target, uint32_t offset, uint32_t len, uint8_t *ver) {
  uint8_t tbuf[12] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[16] = {0x00};
  uint8_t rlen = 0;
  int ret;
  int retries = 3;

  // Fill the component for which firmware is requested
  tbuf[3] = target;

  // Fill the offset
  tbuf[4] = (offset) & 0xFF;
  tbuf[5] = (offset >> 8) & 0xFF;
  tbuf[6] = (offset >> 16) & 0xFF;
  tbuf[7] = (offset >> 24) & 0xFF;

  // Fill the length
  tbuf[8] = (len) & 0xFF;
  tbuf[9] = (len >> 8) & 0xFF;
  tbuf[10] = (len >> 16) & 0xFF;
  tbuf[11] = (len >> 24) & 0xFF;

bic_send:
  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_FW_CKSUM, tbuf, 12, rbuf, &rlen);
  if ((ret || (rlen != 4+SIZE_IANA_ID)) && (retries--)) {  // checksum has to be 4 bytes
    sleep(1);
    syslog(LOG_ERR, "bic_get_fw_cksum: slot: %d, target %d, offset: %d, ret: %d, rlen: %d\n", slot_id, target, offset, ret, rlen);
    goto bic_send;
  }
  if (ret || (rlen != 4+SIZE_IANA_ID)) {
    return -1;
  }

#ifdef DEBUG
  printf("cksum returns: %x:%x:%x::%x:%x:%x:%x\n", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6]);
#endif

  //Ignore IANA ID
  memcpy(ver, &rbuf[SIZE_IANA_ID], rlen-SIZE_IANA_ID);

  return ret;
}

// Read Firwmare Versions of various components
static int
_enable_bic_update(uint8_t slot_id) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00};
  uint8_t rbuf[16] = {0x00};
  uint8_t rlen = 0;
  int ret;

  // 0x1: Update on I2C Channel, the only available option from BMC
  tbuf[3] = 0x1;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_ENABLE_BIC_UPDATE, tbuf, 4, rbuf, &rlen);

  return ret;
}

// Update firmware for various components
static int
_update_fw(uint8_t slot_id, uint8_t target, uint32_t offset, uint16_t len, uint8_t *buf) {
  uint8_t tbuf[256] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[16] = {0x00};
  uint8_t tlen = 0;
  uint8_t rlen = 0;
  int ret;
  int retries = 3;

  // Fill the component for which firmware is requested
  tbuf[3] = target;

  tbuf[4] = (offset) & 0xFF;
  tbuf[5] = (offset >> 8) & 0xFF;
  tbuf[6] = (offset >> 16) & 0xFF;
  tbuf[7] = (offset >> 24) & 0xFF;

  tbuf[8] = len & 0xFF;
  tbuf[9] = (len >> 8) & 0xFF;

  memcpy(&tbuf[10], buf, len);

  tlen = len + 10;

bic_send:
  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_UPDATE_FW, tbuf, tlen, rbuf, &rlen);
  if ((ret) && (retries--)) {
    sleep(1);
    printf("_update_fw: slot: %d, target %d, offset: %d, len: %d retrying..\
           \n",    slot_id, target, offset, len);
    goto bic_send;
  }

  return ret;
}

// Read firmware for various components
static int
_dump_fw(uint8_t slot_id, uint8_t target, uint32_t offset, uint8_t len, uint8_t *rbuf, uint8_t *rlen) {
  uint8_t tbuf[16] = {0x15, 0xA0, 0x00};  // IANA ID
  int ret;
  int retries = 3;

  // Fill the component for which firmware is requested
  tbuf[3] = target;
  memcpy(&tbuf[4], &offset, sizeof(uint32_t));
  tbuf[8] = len;

  do {
    ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_READ_FW_IMAGE, tbuf, 9, rbuf, rlen);
    if (!ret && (len == (*rlen -= 3)))
      return 0;

    sleep(1);
    printf("_dump_fw: slot: %d, target %d, offset: %d, len: %d retrying..\n", slot_id, target, offset, len);
  } while ((--retries));

  return -1;
}

// Get CPLD update progress
static int
_get_cpld_update_progress(uint8_t slot_id, uint8_t *progress) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00};  // IANA ID
  uint8_t rbuf[16] = {0x00};
  uint8_t rlen = 0;
  int ret;
  int retries = 3;

bic_send:
  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_CPLD_UPDATE_PROGRESS, tbuf, 3, rbuf, &rlen);
  if ((ret) && (retries--)) {
    sleep(1);
    printf("_get_cpld_update_progress: slot: %d, retrying..\n", slot_id);
    goto bic_send;
  }

  // Ignore IANA ID
  memcpy(progress, &rbuf[3], 1);

  return ret;
}

static int
_update_bic_main(uint8_t slot_id, char *path) {
  int fd;
  int ifd = -1;
  char cmd[100] = {0};
  struct stat buf;
  int size;
  uint8_t tbuf[256] = {0};
  uint8_t rbuf[16] = {0};
  uint8_t tcount;
  uint8_t rcount;
  volatile int xcount;
  int i = 0;
  int ret = -1, rc;
  uint8_t xbuf[256] = {0};
  uint32_t offset = 0, last_offset = 0, dsize;
  struct rlimit mqlim;

  syslog(LOG_CRIT, "bic_update_fw: update bic firmware on slot %d\n", slot_id);

  // Open the file exclusively for read
  fd = open(path, O_RDONLY, 0666);
  if (fd < 0) {
    syslog(LOG_ERR, "bic_update_fw: open fails for path: %s\n", path);
    goto error_exit;
  }

  fstat(fd, &buf);
  size = buf.st_size;
  printf("size of file is %d bytes\n", size);
  dsize = size/20;

  // Open the i2c driver
  ifd = i2c_open(get_ipmb_bus_id(slot_id));
  if (ifd < 0) {
    printf("ifd error\n");
    goto error_exit;
  }

  // Kill ipmb daemon for this slot
  sprintf(cmd, "sv stop ipmbd_%d", get_ipmb_bus_id(slot_id));
  system(cmd);
  printf("stop ipmbd for slot %x..\n", slot_id);

  // The I2C high speed clock (1M) could cause to read BIC data abnormally.
  // So reduce I2C bus clock speed which is a workaround for BIC update.
  switch(slot_id)
  {
     case FRU_SLOT1:
       system("devmem 0x1e78a084 w 0xFFF77304");
       break;
     case FRU_SLOT2:
       system("devmem 0x1e78a104 w 0xFFF77304");
       break;
     case FRU_SLOT3:
       system("devmem 0x1e78a184 w 0xFFF77304");
       break;
     case FRU_SLOT4:
       system("devmem 0x1e78a304 w 0xFFF77304");
       break;
     default:
       syslog(LOG_CRIT, "bic_update_fw: incorrect slot_id %d\n", slot_id);
       goto error_exit;
       break;
  }
  sleep(1);
  printf("Stopped ipmbd for this slot %x..\n",slot_id);

  if (!_is_bic_update_ready(slot_id)) {
    mqlim.rlim_cur = RLIM_INFINITY;
    mqlim.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_MSGQUEUE, &mqlim) < 0) {
      goto error_exit;
    }

    // Restart ipmb daemon with "bicup" for bic update
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "/usr/local/bin/ipmbd %d %d bicup > /dev/null 2>&1 &", get_ipmb_bus_id(slot_id), slot_id);
    system(cmd);
    printf("start ipmbd bicup for this slot %x..\n",slot_id);

    sleep(2);

    // Enable Bridge-IC update
    _enable_bic_update(slot_id);

    // Kill ipmb daemon "bicup" for this slot
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "ps | grep -v 'grep' | grep 'ipmbd %d' |awk '{print $1}'| xargs kill", get_ipmb_bus_id(slot_id));
    system(cmd);
    printf("stop ipmbd for slot %x..\n", slot_id);
  }

  // Wait for SMB_BMC_3v3SB_ALRT_N
  for (i = 0; i < BIC_UPDATE_RETRIES; i++) {
    if (_is_bic_update_ready(slot_id)) {
      printf("bic ready for update after %d tries\n", i);
      break;
    }
    msleep(BIC_UPDATE_TIMEOUT);
  }

  if (i == BIC_UPDATE_RETRIES) {
    printf("bic is NOT ready for update\n");
    syslog(LOG_CRIT, "bic_update_fw: bic is NOT ready for update\n");
    goto update_done;
  }

  sleep(1);

  // Start Bridge IC update(0x21)

  tbuf[0] = CMD_DOWNLOAD_SIZE;
  tbuf[1] = 0x00; //Checksum, will fill later

  tbuf[2] = BIC_CMD_DOWNLOAD;

  // update flash address: 0x8000
  tbuf[3] = (BIC_FLASH_START >> 24) & 0xff;
  tbuf[4] = (BIC_FLASH_START >> 16) & 0xff;
  tbuf[5] = (BIC_FLASH_START >> 8) & 0xff;
  tbuf[6] = (BIC_FLASH_START) & 0xff;

  // image size
  tbuf[7] = (size >> 24) & 0xff;
  tbuf[8] = (size >> 16) & 0xff;
  tbuf[9] = (size >> 8) & 0xff;
  tbuf[10] = (size) & 0xff;

  // calcualte checksum for data portion
  for (i = 2; i < CMD_DOWNLOAD_SIZE; i++) {
    tbuf[1] += tbuf[i];
  }

  tcount = CMD_DOWNLOAD_SIZE;

  rcount = 0;

  rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
  if (rc) {
    printf("i2c_io failed download\n");
    goto error_exit;
  }

  //delay for download command process ---
  msleep(500);
  tcount = 0;
  rcount = 2;
  rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
  if (rc) {
    printf("i2c_io failed download ack\n");
    goto error_exit;
  }

  if (rbuf[0] != 0x00 || rbuf[1] != 0xcc) {
    printf("download response: %x:%x\n", rbuf[0], rbuf[1]);
    goto error_exit;
  }

  // Loop to send all the image data
  while (1) {
    // Get Status
    tbuf[0] = CMD_STATUS_SIZE;
    tbuf[1] = BIC_CMD_STATUS;// checksum same as data
    tbuf[2] = BIC_CMD_STATUS;

    tcount = CMD_STATUS_SIZE;
    rcount = 0;

    rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
    if (rc) {
      printf("i2c_io failed get status\n");
      goto error_exit;
    }

    tcount = 0;
    rcount = 5;

    rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
    if (rc) {
      printf("i2c_io failed get status ack\n");
      goto error_exit;
    }

    if (rbuf[0] != 0x00 ||
        rbuf[1] != 0xcc ||
        rbuf[2] != 0x03 ||
        rbuf[3] != 0x40 ||
        rbuf[4] != 0x40) {
      printf("status resp: %x:%x:%x:%x:%x", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4]);
      goto error_exit;
    }

    // Send ACK ---
    tbuf[0] = 0xcc;
    tcount = 1;
    rcount = 0;
    rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
    if (rc) {
      printf("i2c_io failed send ack\n");
      goto error_exit;
    }

    // send next packet

    xcount = read(fd, xbuf, BIC_PKT_MAX);
    if (xcount <= 0) {
#ifdef DEBUG
      printf("read returns %d\n", xcount);
#endif
      break;
    }
#ifdef DEBUG
    printf("read returns %d bytes\n", xcount);
#endif

    offset += xcount;
    if((last_offset + dsize) <= offset) {
       printf("updated bic: %d %%\n", offset/dsize*5);
       last_offset += dsize;
    }

    tbuf[0] = xcount + 3;
    tbuf[1] = BIC_CMD_DATA;
    tbuf[2] = BIC_CMD_DATA;
    memcpy(&tbuf[3], xbuf, xcount);

    for (i = 0; i < xcount; i++) {
      tbuf[1] += xbuf[i];
    }

    tcount = tbuf[0];
    rcount = 0;

    rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
    if (rc) {
      printf("i2c_io error send data\n");
      goto error_exit;
    }

    msleep(10);
    tcount = 0;
    rcount = 2;

    rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
    if (rc) {
      printf("i2c_io error send data ack\n");
      goto error_exit;
    }

    if (rbuf[0] != 0x00 || rbuf[1] != 0xcc) {
      printf("data error: %x:%x\n", rbuf[0], rbuf[1]);
      goto error_exit;
    }
  }
  msleep(500);

  // Run the new image
  tbuf[0] = CMD_RUN_SIZE;
  tbuf[1] = 0x00; //checksum
  tbuf[2] = BIC_CMD_RUN;
  tbuf[3] = (BIC_FLASH_START >> 24) & 0xff;
  tbuf[4] = (BIC_FLASH_START >> 16) & 0xff;
  tbuf[5] = (BIC_FLASH_START >> 8) & 0xff;
  tbuf[6] = (BIC_FLASH_START) & 0xff;

  for (i = 2; i < CMD_RUN_SIZE; i++) {
    tbuf[1] += tbuf[i];
  }

  tcount = CMD_RUN_SIZE;
  rcount = 0;

  rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
  if (rc) {
    printf("i2c_io failed run\n");
    goto error_exit;
  }

  tcount = 0;
  rcount = 2;

  rc = i2c_io(ifd, tbuf, tcount, rbuf, rcount);
  if (rc) {
    printf("i2c_io failed run ack\n");
    goto error_exit;
  }

  if (rbuf[0] != 0x00 || rbuf[1] != 0xcc) {
    printf("run response: %x:%x\n", rbuf[0], rbuf[1]);
    goto error_exit;
  }
  msleep(500);

  // Wait for SMB_BMC_3v3SB_ALRT_N
  for (i = 0; i < BIC_UPDATE_RETRIES; i++) {
    if (!_is_bic_update_ready(slot_id))
      break;

    msleep(BIC_UPDATE_TIMEOUT);
  }
  if (i == BIC_UPDATE_RETRIES) {
    printf("bic is NOT ready\n");
    goto error_exit;
  }

update_done:
  ret = 0;
  // Restore the I2C bus clock to 1M.
  switch(slot_id)
  {
     case FRU_SLOT1:
       system("devmem 0x1e78a084 w 0xFFF5E700");
       break;
     case FRU_SLOT2:
       system("devmem 0x1e78a104 w 0xFFF5E700");
       break;
     case FRU_SLOT3:
       system("devmem 0x1e78a184 w 0xFFF5E700");
       break;
     case FRU_SLOT4:
       system("devmem 0x1e78a304 w 0xFFF5E700");
       break;
     default:
       syslog(LOG_ERR, "bic_update_fw: incorrect slot_id %d\n", slot_id);
       break;
  }

  // Restart ipmbd daemon
  sleep(1);
  memset(cmd, 0, sizeof(cmd));
  sprintf(cmd, "sv start ipmbd_%d", get_ipmb_bus_id(slot_id));
  system(cmd);

error_exit:
  syslog(LOG_CRIT, "bic_update_fw: updating bic firmware is exiting\n");
  if (fd > 0) {
    close(fd);
  }

  if (ifd > 0) {
     close(ifd);
  }

  return ret;
}

static int
check_vr_image(uint8_t slot_id, int fd, long size) {
  uint8_t buf[32];
  uint8_t hdr_tl[] = {0x00,0x01,0x4c,0x1c,0x00,0x46,0x30,0x39};
  uint8_t *hdr = hdr_tl, hdr_size = sizeof(hdr_tl);
#if defined(CONFIG_FBY2_EP)
  int ret;
  uint8_t server_type = 0xFF;
  uint8_t hdr_ep[] = {0x00,0x01,0x4c,0x1c,0x00,0x46,0x30,0x39,0x41};

  ret = bic_get_server_type(slot_id, &server_type);
  if (ret) {
    syslog(LOG_ERR, "%s, Get server type failed\n", __func__);
    return -1;
  }

  switch (server_type) {
    case SERVER_TYPE_EP:
      hdr = hdr_ep;
      hdr_size = sizeof(hdr_ep);
      break;
  }
#endif

  if (size < 16)
    return -1;

  lseek(fd, 1, SEEK_SET);

  if (read(fd, buf, hdr_size) != hdr_size)
    return -1;

  if (memcmp(buf, hdr, hdr_size))
    return -1;

  lseek(fd, 0, SEEK_SET);
  return 0;
}

static int
check_cpld_image(uint8_t slot_id, int fd, long size) {
  uint8_t buf[32];
  uint8_t hdr_tl[] = {0x01,0x00,0x4c,0x1c,0x00,0x01,0x2b,0xb0,0x43,0x46,0x30,0x39};
  uint8_t *hdr = hdr_tl, hdr_size = sizeof(hdr_tl);
#if defined(CONFIG_FBY2_RC) || defined(CONFIG_FBY2_EP)
  int ret;
  uint8_t server_type = 0xFF;
  uint8_t hdr_ep[] = {0x01,0x00,0x4c,0x1c,0x00,0xe1,0x2b,0xc0,0x43,0x46,0x30,0x39,0x41};

  ret = bic_get_server_type(slot_id, &server_type);
  if (ret) {
    syslog(LOG_ERR, "%s, Get server type failed\n", __func__);
    return -1;
  }

  switch (server_type) {
    case SERVER_TYPE_RC:
      return 0;
    case SERVER_TYPE_EP:
      hdr = hdr_ep;
      hdr_size = sizeof(hdr_ep);
      break;
  }
#endif

  if (size < 32)
    return -1;

  if (read(fd, buf, hdr_size) != hdr_size)
    return -1;

  if (memcmp(buf, hdr, hdr_size))
    return -1;

  lseek(fd, 0, SEEK_SET);
  return 0;
}

static int
check_bios_image(uint8_t slot_id, int fd, long size) {
  int i, rcnt, end;
  uint8_t *buf;
  uint8_t ver_sig[] = { 0x46, 0x49, 0x44, 0x04, 0x78, 0x00 };
#if defined(CONFIG_FBY2_EP)
  int ret;
  uint8_t server_type = 0xFF;

  ret = bic_get_server_type(slot_id, &server_type);
  if (ret) {
    syslog(LOG_ERR, "%s, Get server type failed\n", __func__);
    return -1;
  }

  switch (server_type) {
    case SERVER_TYPE_EP:
      return 0;
  }
#endif

  if (size < BIOS_VER_REGION_SIZE)
    return -1;

  buf = (uint8_t *)malloc(BIOS_VER_REGION_SIZE);
  if (!buf) {
    return -1;
  }

  lseek(fd, (size - BIOS_VER_REGION_SIZE), SEEK_SET);
  i = 0;
  while (i < BIOS_VER_REGION_SIZE) {
    rcnt = read(fd, (buf + i), BIOS_ERASE_PKT_SIZE);
    if ((rcnt < 0) && (errno != EINTR)) {
      free(buf);
      return -1;
    }
    i += rcnt;
  }

  end = BIOS_VER_REGION_SIZE - (sizeof(ver_sig) + strlen(BIOS_VER_STR));
  for (i = 0; i < end; i++) {
    if (!memcmp(buf+i, ver_sig, sizeof(ver_sig))) {
      if (memcmp(buf+i+sizeof(ver_sig), BIOS_VER_STR, strlen(BIOS_VER_STR)) && memcmp(buf+i+sizeof(ver_sig), Y2_BIOS_VER_STR, strlen(Y2_BIOS_VER_STR))) {
        i = end;
      }
      break;
    }
  }
  free(buf);

  if (i >= end)
    return -1;

  lseek(fd, 0, SEEK_SET);
  return 0;
}

static int
_set_fw_update_ongoing(uint8_t slot_id, uint16_t tmout) {
  char key[64];
  char value[64] = {0};
  struct timespec ts;

  sprintf(key, "fru%u_fwupd", slot_id);

  clock_gettime(CLOCK_MONOTONIC, &ts);
  ts.tv_sec += tmout;
  sprintf(value, "%d", ts.tv_sec);

  if (edb_cache_set(key, value) < 0) {
     return -1;
  }

  return 0;
}

static int
verify_bios_image(uint8_t slot_id, int fd, long size) {
  int ret = -1;
  int rc, i;
  uint32_t offset;
  uint32_t tcksum, gcksum;
  volatile uint16_t count;
  uint8_t target, last_pkt = 0x00;
  uint8_t *tbuf = NULL;
#if defined(CONFIG_FBY2_EP)
  uint8_t server_type = 0xFF;

  rc = bic_get_server_type(slot_id, &server_type);
  if (rc) {
    syslog(LOG_ERR, "%s, Get server type failed\n", __func__);
    return -1;
  }

  switch (server_type) {
    case SERVER_TYPE_EP:
      last_pkt = 0x80;
      break;
  }
#endif

  // Checksum calculation for BIOS image
  tbuf = malloc(BIOS_VERIFY_PKT_SIZE * sizeof(uint8_t));
  if (!tbuf) {
    return -1;
  }

  lseek(fd, 0, SEEK_SET);
  offset = 0;
  while (1) {
    count = read(fd, tbuf, BIOS_VERIFY_PKT_SIZE);
    if (count <= 0) {
      if (offset >= size) {
        ret = 0;
      }
      break;
    }

    tcksum = 0;
    for (i = 0; i < count; i++) {
      tcksum += tbuf[i];
    }

    target = ((offset + count) >= size) ? (UPDATE_BIOS | last_pkt) : UPDATE_BIOS;

    // Get the checksum of binary image
    rc = bic_get_fw_cksum(slot_id, target, offset, count, (uint8_t*)&gcksum);
    if (rc) {
      printf("get checksum failed, offset:0x%x\n", offset);
      break;
    }

    // Compare both and see if they match or not
    if (gcksum != tcksum) {
      printf("checksum does not match, offset:0x%x, 0x%x:0x%x\n", offset, tcksum, gcksum);
      break;
    }

    offset += count;
  }

  free(tbuf);
  return ret;
}

int
bic_dump_fw(uint8_t slot_id, uint8_t comp, char *path) {
  int ret = -1, rc, fd;
  uint32_t offset, next_doffset;
  uint32_t img_size = 0x2000000, dsize;
  uint8_t count, read_count;
  uint8_t buf[256], rlen;

  if (comp != DUMP_BIOS) {
    printf("ERROR: only support dump BIOS image!\n");
    return ret;
  }
  printf("dumping fw on slot %d:\n", slot_id);

  fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    printf("ERROR: invalid file path!\n");
#ifdef DEBUG
    syslog(LOG_ERR, "bic_dump_fw: open fails for path: %s\n", path);
#endif
    goto error_exit;
  }

  // Write chunks of binary data in a loop
  dsize = img_size / 100;
  offset = 0;
  next_doffset = offset + dsize;
  while (1) {
    read_count = ((offset + IPMB_READ_COUNT_MAX) <= img_size) ? IPMB_READ_COUNT_MAX : (img_size - offset);

    // read image from Bridge-IC
    rc = _dump_fw(slot_id, comp, offset, read_count, buf, &rlen);
    if (rc) {
      goto error_exit;
    }

    // Write to file
    count = write(fd, &buf[3], rlen);
    if (count <= 0) {
      goto error_exit;
    }

    // Update counter
    offset += count;
    if (offset >= next_doffset) {
      switch (comp) {
        case DUMP_BIOS:
          printf("\rdumped bios: %d %%", offset/dsize);
          break;
      }
      fflush(stdout);
      next_doffset += dsize;
    }

    if (offset >= img_size)
      break;
  }

  ret = 0;

error_exit:
  printf("\n");
  if (fd > 0 ) {
    close(fd);
  }

  return ret;
}

int
bic_update_fw(uint8_t slot_id, uint8_t comp, char *path) {
  int ret = -1, rc;
  uint32_t offset;
  volatile uint16_t count, read_count;
  uint8_t buf[256] = {0};
  uint8_t target;
  int fd;
  int i;

  printf("updating fw on slot %d:\n", slot_id);
  // Handle Bridge IC firmware separately as the process differs significantly from others
  if (comp == UPDATE_BIC) {
    return  _update_bic_main(slot_id, path);
  }

  uint32_t dsize, last_offset;
  struct stat st;
  // Open the file exclusively for read
  fd = open(path, O_RDONLY, 0666);
  if (fd < 0) {
    printf("ERROR: invalid file path!\n");
#ifdef DEBUG
    syslog(LOG_ERR, "bic_update_fw: open fails for path: %s\n", path);
#endif
    goto error_exit;
  }

  stat(path, &st);
  if (comp == UPDATE_BIOS) {
    if (check_bios_image(slot_id, fd, st.st_size) < 0) {
      printf("invalid BIOS file!\n");
      lseek(fd, 0, SEEK_SET);
      //goto error_exit;
    }
    syslog(LOG_CRIT, "Update BIOS: update bios firmware on slot %d\n", slot_id);
    dsize = st.st_size/100;
  } else if (comp == UPDATE_VR) {
    if (check_vr_image(slot_id, fd, st.st_size) < 0) {
      printf("invalid VR file!\n");
      goto error_exit;
    }
    syslog(LOG_CRIT, "Update VR: update vr firmware on slot %d\n", slot_id);
    dsize = st.st_size/5;
  } else {
    if ((comp == UPDATE_CPLD) && (check_cpld_image(slot_id, fd, st.st_size) < 0)) {
      printf("invalid CPLD file!\n");
      goto error_exit;
    }
    switch(comp){
      case UPDATE_CPLD:
        syslog(LOG_CRIT, "Update CPLD: update cpld firmware on slot %d\n", slot_id);
        break;
      case UPDATE_BIC_BOOTLOADER:
        syslog(LOG_CRIT, "Update BIC BL: update bic bootloader firmware on slot %d\n", slot_id);
        break;
    }
    dsize = st.st_size/20;
  }
  // Write chunks of binary data in a loop
  offset = 0;
  last_offset = 0;
  i = 1;
  while (1) {
    // For BIOS, send packets in blocks of 64K
    if (comp == UPDATE_BIOS && ((offset+IPMB_WRITE_COUNT_MAX) > (i * BIOS_ERASE_PKT_SIZE))) {
      read_count = (i * BIOS_ERASE_PKT_SIZE) - offset;
      i++;
    } else {
      read_count = IPMB_WRITE_COUNT_MAX;
    }

    // Read from file
    count = read(fd, buf, read_count);
    if (count <= 0) {
      break;
    }

    // For non-BIOS update, the last packet is indicated by extra flag
    if ((comp != UPDATE_BIOS) && ((offset + count) >= st.st_size)) {
      target = comp | 0x80;
    } else {
      target = comp;
    }

    // Send data to Bridge-IC
    rc = _update_fw(slot_id, target, offset, count, buf);
    if (rc) {
      goto error_exit;
    }

    // Update counter
    offset += count;
    if ((last_offset + dsize) <= offset) {
       switch(comp) {
         case UPDATE_BIOS:
           _set_fw_update_ongoing(slot_id, 60);
           printf("\rupdated bios: %d %%", offset/dsize);
           break;
         case UPDATE_CPLD:
           printf("\ruploaded cpld: %d %%", offset/dsize*5);
           break;
         case UPDATE_VR:
           printf("\rupdated vr: %d %%", offset/dsize*20);
           break;
         default:
           printf("\rupdated bic boot loader: %d %%", offset/dsize*5);
           break;
       }
       fflush(stdout);
       last_offset += dsize;
    }
  }

  if (comp == UPDATE_CPLD) {
    for (i = 0; i < 60; i++) {  // wait 60s at most
      rc = _get_cpld_update_progress(slot_id, buf);
      if (rc) {
        goto error_exit;
      }

      if (buf[0] == 0xFD) {  // error code
        goto error_exit;
      }

      printf("\rupdated cpld: %d %%", buf[0]);
      fflush(stdout);
      buf[0] %= 101;
      if (buf[0] == 100)
        break;

      sleep(2);
    }
  }

  if (comp == UPDATE_BIOS) {
    _set_fw_update_ongoing(slot_id, 60 * 2);
    if (verify_bios_image(slot_id, fd, st.st_size))
      goto error_exit;
  }

update_done:
  ret = 0;
error_exit:
  printf("\n");
  switch(comp) {
    case UPDATE_BIOS:
      syslog(LOG_CRIT, "Update BIOS: updating bios firmware is exiting\n");
      break;
    case UPDATE_CPLD:
      syslog(LOG_CRIT, "Update CPLD: updating cpld firmware is exiting\n");
      break;
    case UPDATE_VR:
      syslog(LOG_CRIT, "Update VR: updating vr firmware is exiting\n");
      break;
    case UPDATE_BIC_BOOTLOADER:
      syslog(LOG_CRIT, "Update BIC BL: updating bic bootloader firmware is exiting\n");
      break;
  }
  if (fd > 0 ) {
    close(fd);
  }

  return ret;
}

int
bic_me_xmit(uint8_t slot_id, uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t *rxlen) {
  uint8_t tbuf[256] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[256] = {0x00};
  uint8_t rlen = 0;
  uint8_t tlen = 0;
  int ret;

  // Fill the interface number as ME
  tbuf[3] = BIC_INTF_ME;

  // Fill the data to be sent
  memcpy(&tbuf[4], txbuf, txlen);

  // Send data length includes IANA ID and interface number
  tlen = txlen + 4;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_MSG_OUT, tbuf, tlen, rbuf, &rlen);
  if (ret ) {
    return -1;
  }

  // Make sure the received interface number is same
  if (rbuf[3] != tbuf[3]) {
    return -1;
  }

  // Copy the received data to caller skipping header
  memcpy(rxbuf, &rbuf[6], rlen-6);

  *rxlen = rlen-6;

  return 0;
}

// Read 1S server's FRUID
int
bic_get_fruid_info(uint8_t slot_id, uint8_t fru_id, ipmi_fruid_info_t *info) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_GET_FRUID_INFO, &fru_id, 1, (uint8_t *) info, &rlen);

  return ret;
}

static int
_read_fruid(uint8_t slot_id, uint8_t fru_id, uint32_t offset, uint8_t count, uint8_t *rbuf, uint8_t *rlen) {
  int ret;
  uint8_t tbuf[4] = {0};
  uint8_t tlen = 0;

  tbuf[0] = fru_id;
  tbuf[1] = offset & 0xFF;
  tbuf[2] = (offset >> 8) & 0xFF;
  tbuf[3] = count;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_READ_FRUID_DATA, tbuf, 4, rbuf, rlen);

  return ret;
}

int
bic_read_fruid(uint8_t slot_id, uint8_t fru_id, const char *path, int *fru_size) {
  int ret = 0;
  uint32_t nread;
  uint32_t offset;
  uint8_t count;
  uint8_t rbuf[256] = {0};
  uint8_t rlen = 0;
  int fd;
  ipmi_fruid_info_t info;

  // Remove the file if exists already
  unlink(path);

  // Open the file exclusively for write
  fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd < 0) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_read_fruid: open fails for path: %s\n", path);
#endif
    goto error_exit;
  }

  // Read the FRUID information
  ret = bic_get_fruid_info(slot_id, fru_id, &info);
  if (ret) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_read_fruid: bic_read_fruid_info returns %d\n", ret);
#endif
    goto error_exit;
  }

  // Indicates the size of the FRUID
  nread = (info.size_msb << 6) + (info.size_lsb);
  *fru_size = nread;
  if (*fru_size == 0)
     goto error_exit;

  // Read chunks of FRUID binary data in a loop
  offset = 0;
  while (nread > 0) {
    if (nread > FRUID_READ_COUNT_MAX) {
      count = FRUID_READ_COUNT_MAX;
    } else {
      count = nread;
    }

    ret = _read_fruid(slot_id, fru_id, offset, count, rbuf, &rlen);
    if (ret) {
#ifdef DEBUG
      syslog(LOG_ERR, "bic_read_fruid: ipmb_wrapper fails\n");
#endif
      goto error_exit;
    }

    // Ignore the first byte as it indicates length of response
    write(fd, &rbuf[1], rlen-1);

    // Update offset
    offset += (rlen-1);
    nread -= (rlen-1);
  }

 close(fd);
 return ret;

error_exit:
  if (fd > 0 ) {
    close(fd);
  }

  return -1;
}

static int
_write_fruid(uint8_t slot_id, uint8_t fru_id, uint32_t offset, uint8_t count, uint8_t *buf) {
  int ret;
  uint8_t tbuf[64] = {0};
  uint8_t rbuf[4] = {0};
  uint8_t tlen = 0;
  uint8_t rlen = 0;

  tbuf[0] = fru_id;
  tbuf[1] = offset & 0xFF;
  tbuf[2] = (offset >> 8) & 0xFF;

  memcpy(&tbuf[3], buf, count);
  tlen = count + 3;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_WRITE_FRUID_DATA, tbuf, tlen, rbuf, &rlen);

  if (ret) {
    return ret;
  }

  if (rbuf[0] != count) {
    return -1;
  }

  return ret;
}

int
bic_write_fruid(uint8_t slot_id, uint8_t fru_id, const char *path) {
  int ret;
  uint32_t offset;
  uint8_t count;
  uint8_t buf[64] = {0};
  uint8_t len = 0;
  int fd;

  // Open the file exclusively for read
  fd = open(path, O_RDONLY, 0666);
  if (fd < 0) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_write_fruid: open fails for path: %s\n", path);
#endif
    goto error_exit;
  }

  // Write chunks of FRUID binary data in a loop
  offset = 0;
  while (1) {
    // Read from file
    count = read(fd, buf, FRUID_WRITE_COUNT_MAX);
    if (count <= 0) {
      break;
    }

    // Write to the FRUID
    ret = _write_fruid(slot_id, fru_id, offset, count, buf);
    if (ret) {
      break;
    }

    // Update counter
    offset += count;
  }

error_exit:
  if (fd > 0 ) {
    close(fd);
  }

  return ret;
}

// Read System Event Log (SEL)
int
bic_get_sel_info(uint8_t slot_id, ipmi_sel_sdr_info_t *info) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_GET_SEL_INFO, NULL, 0, (uint8_t *)info, &rlen);

  return ret;
}

static int
_get_sel_rsv(uint8_t slot_id, uint16_t *rsv) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_RSV_SEL, NULL, 0, (uint8_t *) rsv, &rlen);
  return ret;
}

int
bic_get_sel(uint8_t slot_id, ipmi_sel_sdr_req_t *req, ipmi_sel_sdr_res_t *res, uint8_t *rlen) {

  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_GET_SEL, (uint8_t *)req, sizeof(ipmi_sel_sdr_req_t), (uint8_t*)res, rlen);

  return ret;
}

// Read Sensor Data Records (SDR)
int
bic_get_sdr_info(uint8_t slot_id, ipmi_sel_sdr_info_t *info) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_GET_SDR_INFO, NULL, 0, (uint8_t *) info, &rlen);

  return ret;
}

static int
_get_sdr_rsv(uint8_t slot_id, uint16_t *rsv) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_RSV_SDR, NULL, 0, (uint8_t *) rsv, &rlen);

  return ret;
}

static int
_get_sdr(uint8_t slot_id, ipmi_sel_sdr_req_t *req, ipmi_sel_sdr_res_t *res, uint8_t *rlen) {
  int ret;

  ret = bic_ipmb_wrapper(slot_id, NETFN_STORAGE_REQ, CMD_STORAGE_GET_SDR, (uint8_t *)req, sizeof(ipmi_sel_sdr_req_t), (uint8_t*)res, rlen);

  return ret;
}

int
bic_get_sdr(uint8_t slot_id, ipmi_sel_sdr_req_t *req, ipmi_sel_sdr_res_t *res, uint8_t *rlen) {
  int ret;
  uint8_t tbuf[MAX_IPMB_RES_LEN] = {0};
  uint8_t tlen;
  uint8_t len;
  ipmi_sel_sdr_res_t *tres;
  sdr_rec_hdr_t *hdr;

  tres = (ipmi_sel_sdr_res_t *) tbuf;

  // Get SDR reservation ID for the given record
  ret = _get_sdr_rsv(slot_id, &req->rsv_id);
  if (ret) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_read_sdr: _get_sdr_rsv returns %d\n", ret);
#endif
    return ret;
  }

  // Initialize the response length to zero
  *rlen = 0;

  // Read SDR Record Header
  req->offset = 0;
  req->nbytes = sizeof(sdr_rec_hdr_t);

  ret = _get_sdr(slot_id, req, (ipmi_sel_sdr_res_t *)tbuf, &tlen);
  if (ret) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_read_sdr: _get_sdr returns %d\n", ret);
#endif
    return ret;
  }

  // Copy the next record id to response
  res->next_rec_id = tres->next_rec_id;

  // Copy the header excluding first two bytes(next_rec_id)
  memcpy(res->data, tres->data, tlen-2);

  // Update response length and offset for next request
  *rlen += tlen-2;
  req->offset = tlen-2;

  // Find length of data from header info
  hdr = (sdr_rec_hdr_t *) tres->data;
  len = hdr->len;

  // Keep reading chunks of SDR record in a loop
  while (len > 0) {
    if (len > SDR_READ_COUNT_MAX) {
      req->nbytes = SDR_READ_COUNT_MAX;
    } else {
      req->nbytes = len;
    }

    ret = _get_sdr(slot_id, req, (ipmi_sel_sdr_res_t *)tbuf, &tlen);
    if (ret) {
#ifdef DEBUG
      syslog(LOG_ERR, "bic_read_sdr: _get_sdr returns %d\n", ret);
#endif
      return ret;
    }

    // Copy the data excluding the first two bytes(next_rec_id)
    memcpy(&res->data[req->offset], tres->data, tlen-2);

    // Update response length, offset for next request, and remaining length
    *rlen += tlen-2;
    req->offset += tlen-2;
    len -= tlen-2;
  }

  return 0;
}

int
bic_read_sensor(uint8_t slot_id, uint8_t sensor_num, ipmi_sensor_reading_t *sensor) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_SENSOR_REQ, CMD_SENSOR_GET_SENSOR_READING, (uint8_t *)&sensor_num, 1, (uint8_t *)sensor, &rlen);

  return ret;
}

int
bic_read_accuracy_sensor(uint8_t slot_id, uint8_t sensor_num, ipmi_accuracy_sensor_reading_t *sensor) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00, 0x00}; // IANA ID + Sensor Num
  uint8_t rbuf[255] = {0x00};
  uint8_t rlen = 0;
  int ret;

  tbuf[3] = sensor_num;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_ACCURACY_SENSOR_READING, tbuf, 0x04, rbuf, &rlen);

  //Ignore IANA ID
  memcpy(sensor, &rbuf[3], rlen-3);

  return ret;
}

int
bic_get_sys_guid(uint8_t slot_id, uint8_t *guid) {
  int ret;
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_APP_REQ, CMD_APP_GET_SYSTEM_GUID, NULL, 0, guid, &rlen);
  if (rlen != SIZE_SYS_GUID) {
#ifdef DEBUG
    syslog(LOG_ERR, "bic_get_sys_guid: returned rlen of %d\n");
#endif
    return -1;
  }

  return ret;
}

int
bic_set_sys_guid(uint8_t slot_id, uint8_t *guid) {
  int ret;
  uint8_t rlen = 0;
  uint8_t rbuf[MAX_IPMB_RES_LEN]={0x00};

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_REQ, CMD_OEM_SET_SYSTEM_GUID, guid, SIZE_SYS_GUID, rbuf, &rlen);

  return ret;
}

int
bic_request_post_buffer_data(uint8_t slot_id, uint8_t *port_buff, uint8_t *len) {
  int ret;
  uint8_t tbuf[3] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rbuf[MAX_IPMB_RES_LEN]={0x00};
  uint8_t rlen = 0;

  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_GET_POST_BUF, tbuf, 0x03, rbuf, &rlen);

  if(0 != ret)
    goto exit_done;

  // Ignore first 3 bytes of IANA ID
  memcpy(port_buff, &rbuf[3], rlen - 3);
  *len = rlen - 3;

exit_done:
  return ret;
}

/*
    0x2E 0xDF: Force Intel ME Recovery
Request
  Byte 1:3 = Intel Manufacturer ID - 000157h, LS byte first.

  Byte 4 - Command
    = 01h Restart using Recovery Firmware
      (Intel ME FW configuration is not restored to factory defaults)
    = 02h Restore Factory Default Variable values and restart the Intel ME FW
    = 03h PTT Initial State Restore
Response
  Byte 1 - Completion Code
  = 00h - Success
  (Remaining standard Completion Codes are shown in Section 2.12)
  = 81h - Unsupported Command parameter value in the Byte 4 of the request.

  Byte 2:4 = Intel Manufacturer ID - 000157h, LS byte first.
*/
int
me_recovery(uint8_t slot_id, uint8_t command) {
  uint8_t tbuf[256] = {0x00};
  uint8_t rbuf[256] = {0x00};
  uint8_t tlen = 0;
  uint8_t rlen = 0;
  int ret = 0;
  int retry = 0;

  while (retry <= 3) {
    tbuf[0] = 0xB8;
    tbuf[1] = 0xDF;
    tbuf[2] = 0x57;
    tbuf[3] = 0x01;
    tbuf[4] = 0x00;
    tbuf[5] = command;
    tlen = 6;

    ret = bic_me_xmit(slot_id, tbuf, tlen, rbuf, &rlen);
    if (ret) {
      retry++;
      sleep(1);
      continue;
    }
    else
      break;
  }
  if (retry == 4) { //if the third retry still failed, return -1
    syslog(LOG_CRIT, "%s: Restart using Recovery Firmware failed..., retried: %d", __func__,  retry);
    return -1;
  }

  sleep(10);
  retry = 0;
  memset(&tbuf, 0, 256);
  memset(&rbuf, 0, 256);
  /*
      0x6 0x4: Get Self-Test Results
    Byte 1 - Completion Code
    Byte 2
      = 55h - No error. All Self-Tests Passed.
      = 81h - Firmware entered Recovery bootloader mode
    Byte 3 For byte 2 = 55h, 56h, FFh:
      =00h
      =02h - recovery mode entered by IPMI command "Force ME Recovery"
  */
  //Using ME self-test result to check if the ME Recovery Command Success or not
  while (retry <= 3) {
    tbuf[0] = 0x18;
    tbuf[1] = 0x04;
    tlen = 2;
    ret = bic_me_xmit(slot_id, tbuf, tlen, rbuf, &rlen);
    if (ret) {
      retry++;
      sleep(1);
      continue;
    }

    //if Get Self-Test Results is 0x55 0x00, means No error. All Self-Tests Passed.
    //if Get Self-Test Results is 0x81 0x02, means Firmware entered Recovery bootloader mode
    if ( (command == RECOVERY_MODE) && (rbuf[1] == 0x81) && (rbuf[2] == 0x02) ) {
      return 0;
    }
    else if ( (command == RESTORE_FACTORY_DEFAULT) && (rbuf[1] == 0x55) && (rbuf[2] == 0x00) ) {
      return 0;
    }
    else {
      return -1;
    }
  }
  if (retry == 4) { //if the third retry still failed, return -1
    syslog(LOG_CRIT, "%s: Restore Factory Default failed..., retried: %d", __func__,  retry);
    return -1;
  }
}

int
bic_get_server_type(uint8_t fru, uint8_t *type) {
  int ret;
  int retries = 3;
  int server_type;
  ipmi_dev_id_t id = {0};

  // SERVER_TYPE[7:6] = 0(TwinLake), 1(RC), 2(EP), 3(unknown)
  // SERVER_TYPE[5:4] = 0(TwinLake), 1(RC), 2(EP), 3(unknown)
  // SERVER_TYPE[3:2] = 0(TwinLake), 1(RC), 2(EP), 3(unknown)
  // SERVER_TYPE[1:0] = 0(TwinLake), 1(RC), 2(EP), 3(unknown)
  if (read_device(SERVER_TYPE_FILE, &server_type)) {
    do{
      ret = bic_get_dev_id(fru, &id);
      if (!ret) {
        // Use product ID to identify the server type
        if (id.prod_id[0] == 0x43 && id.prod_id[1] == 0x52) {
          *type = SERVER_TYPE_RC;
        } else if (id.prod_id[0] == 0x50 && id.prod_id[1] == 0x45) {
          *type = SERVER_TYPE_EP;
        } else if (id.prod_id[0] == 0x39 && id.prod_id[1] == 0x30) {
          *type = SERVER_TYPE_TL;
        } else {
          *type = SERVER_TYPE_NONE;
        }
        break;
      }
    }while ((--retries));

    if(retries == 0) {
      *type = SERVER_TYPE_NONE;
      syslog(LOG_ERR, "%s : Get server type failed for slot%u", __func__, fru);
      return -1;
    }
  }
  else {
    *type = server_type;
    switch(fru)
    {
      case FRU_SLOT1:
        *type = (*type & (0x3 << 0)) >> 0;
        break;
      case FRU_SLOT2:
        *type = (*type & (0x3 << 2)) >> 2;
        break;
      case FRU_SLOT3:
        *type = (*type & (0x3 << 4)) >> 4;
        break;
      case FRU_SLOT4:
        *type = (*type & (0x3 << 6)) >> 6;
        break;
      default:
        *type = SERVER_TYPE_NONE;   //set default to unknown server type
        break;
    }
  }

  return 0;
}

int
bic_set_pcie_config(uint8_t slot_id, uint8_t config) {
  uint8_t tbuf[4] = {0x15, 0xA0, 0x00}; // IANA ID
  uint8_t rlen = 0;
  uint8_t rbuf[16] = {0};
  int ret;

  tbuf[3] = config;
  ret = bic_ipmb_wrapper(slot_id, NETFN_OEM_1S_REQ, CMD_OEM_1S_SET_PCIE_CONFIG, tbuf, 0x04, rbuf, &rlen);

  return ret;
}

int get_imc_version(uint8_t slot, uint8_t *ver) {
  int i;
  int ret;
  char key[MAX_KEY_LEN] = {0};
  char str[MAX_VALUE_LEN] = {0};

  sprintf(key, "slot%d_imc_ver", (int)slot);
  ret = kv_get(key, str);
  if (ret) {
    return ret;
  }

  for (i = 0; i < IMC_VER_SIZE; i++) {
    ver[i] = str[i] - '0';
  }
  return 0;
}
