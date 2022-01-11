#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <openbmc/gpio.h>
#include <openbmc/obmc-i2c.h>
#include "powershelf.h"
#include <syslog.h>
#include <sys/mman.h>

int
i2c_open(uint8_t bus, uint8_t addr) {
  int fd = -1;
  int rc = -1;
  char fn[32];

  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus);
  fd = open(fn, O_RDWR);

  if (fd == -1) {
    return -1;
  }

  rc = ioctl(fd, I2C_SLAVE_FORCE, addr);
  if (rc < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

/*
 * read device sysfs
 */
int
i2c_read_device(const char *device, int *value) {
    FILE    *fp = NULL;
    int      rc = -1;
    int      val;

    for (int i = 0; i < MAX_RETRIES; i++) {
        fp = fopen(device, "r");
        if (!fp) {
            usleep(10000);
            continue;
        }
        else
            break;
    }

    if (fp) {
        rc = fscanf(fp, "%d", &val);
        *value = val;
    }
    else
        return -1;

    fclose(fp);
    if (rc < 0) {
        return -1;
    } else {
        return 0;
    }
}

int
i2c_read_device_float(char* dir, char *device, float *value) {
    int      val;
    float    res;
    char     sysfs_file[DEVICE_FILENAME_SIZE];

    snprintf(sysfs_file, DEVICE_FILENAME_SIZE, "%s/%s", dir, device);

    if (i2c_read_device(sysfs_file, &val) == 0) {
        *value = ((float) val)/1000.0;
        return 0;
    }

    return -1;
}

int
i2c_write_device(const char *device, int value) {
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
 * convert 2's complement to decimal
 */
int convert2compl_to_decimal (int data, int bits)
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
int read_value_linear (char* dir, char *device, float *value)
{
    int     y;
    int     n;
    int     bits = 11;
    int     mask = 0x7FF;
    int     data;
    char filename[DEVICE_FILENAME_SIZE];

    snprintf(filename, DEVICE_FILENAME_SIZE, "%s/%s", dir, device);
    if (i2c_read_device(filename, &data) != 0) {
        syslog(LOG_WARNING, "failed to read %s\n", filename);
        return -1;
    }

    /*
     * linear data format
     * real value X = Y*(2^N)
     *    Y: 11 bit 2's complement integer
     *    N: 5 bit 2's complement integer
     */
    y = convert2compl_to_decimal(data&mask, 11);
    n = convert2compl_to_decimal(data >> 11, 5);

    *value = y*pow(2.0, n);
    return 0;
}

/*
 * PCA9541A I2C-bus master selector
 */

/*
 * I2C-bus master selector: check if bus on and has control
 */
int i2c_master_selector_ctrl(uint8_t ctrl_reg) {
  /*
   * check bus on and has the control
   * PCA9541A I2C-bus master selector:  table 12
   */
  if ((ctrl_reg == 0x4) || (ctrl_reg == 0x7) || (ctrl_reg == 0x8) || (ctrl_reg == 0xb)) {
    return 0;
  }
  else
    return -1;
}

int i2c_master_selector_access(uint8_t bus, uint8_t addr) {
  int      fd = -1;
  int      ret = -1, retry = 0;
  int      val, write_val;
  int      change = 1;
  uint8_t  control_reg = 0x1;

  if(bus > MAX_I2C_BUS_NUM) {
    return -1;
  }

  fd = i2c_open(bus, addr);

  if (fd < 0) {
    return -1;
  }

  val = i2c_smbus_read_byte_data(fd, control_reg);

  while ((retry < MAX_RETRIES) && (val < 0)) {
    usleep(10000);
    val = i2c_smbus_read_byte_data(fd, control_reg);
    if (val < 0)
      retry++;
    else
      break;
  }

  if (val < 0) {
    close(fd);
    return -1;
  }
  val = val & 0xf;

  /*
   * already bus on and has the control
   */
  if (i2c_master_selector_ctrl(val) == 0) {
    close(fd);
    return 0;
  }

  /*
   * PCA9541A I2C-bus master selector spec: table 12
   */
  if ((val == 0) || (val == 1) || (val == 5)) {
     write_val = 4;
  }
  else if ((val == 6) || (val == 3) || (val == 2)) {
     write_val = 5;
  }
  else if ((val == 12) || (val == 9) || (val == 13)) {
     write_val = 0;
  }
  else if ((val == 10) || (val == 14) || (val == 15)) {
     write_val = 1;
  }
  else
    change = 0;

  if (change) {
    i2c_smbus_write_byte_data(fd, control_reg, write_val);
  }

  /*
   * check bus on and has the control after write
   */
  val = i2c_smbus_read_byte_data(fd, control_reg);
  if (i2c_master_selector_ctrl(val) != 0) {
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

int get_file_offset(device_type type)
{
    switch(type) {
    case FILE_FAN:
       return 0;
    case FILE_PSU:
       return (sizeof(fan_info_t));
    case FILE_EFUSE:
       return (sizeof(fan_info_t) + MAX_PSU_NUM*sizeof(psu_info_t));
    default:
       return -1;
    }
}

int
get_mmap_info (int* cache, device_type type, size_t len)
{
    int fd;
    int *map;
    int status, size, offset;
    struct stat s;
    int pagesize = getpagesize();
    int done = 0, retries = 5;

    fd = open(MMAP_FILEPATH, O_RDONLY);
    if (fd == -1) {
        syslog(LOG_WARNING, "Failed to open mmap file\n");
        return -1;
    }

    for(int attempt; attempt < retries; ++attempt) {
        done = flock(fd, LOCK_EX);
        if (done != 0) {
            /*
             * flock errors
             */
            syslog(LOG_WARNING, "Failed to lock mmap file\n");
            sleep(1);
            continue;
        }
        else
            break;
    }

    if (done != 0) {
        syslog(LOG_WARNING, "Failed to lock mmap file\n");
        close(fd);
        return -1;
    }

    /* Get the size of the file. */
    status = fstat (fd, &s);
    size = s.st_size;
    size += pagesize-(size%pagesize);

    map = mmap(0, size,  PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        syslog(LOG_ERR, "mmap failed, errno: %d", errno);
        return -1;
    }

    offset = get_file_offset(type);
    memcpy(cache, map+offset, len);

    flock(fd, LOCK_UN);
    if (munmap(map, size) == -1) {
        syslog(LOG_WARNING, "Error un-mmapping the file");
    }

    close(fd);
    return 0;
}

int
reset_psu_mmap_stats (int psu_num, status_info_t *status_info, int num_status)
{
    int fd;
    int *map;
    int status, size, offset;
    struct stat s;
    int pagesize = getpagesize();
    int done = 0, retries = 5;

    fd = open(MMAP_FILEPATH, O_RDWR);
    if (fd == -1) {
        syslog(LOG_WARNING, "Failed to open mmap file\n");
        return -1;
    }

    for(int attempt; attempt < retries; ++attempt) {
        done = flock(fd, LOCK_EX);
        if (done != 0) {
            /*
             * flock errors
             */
            syslog(LOG_WARNING, "Failed to lock mmap file\n");
            sleep(1);
            continue;
        }
        else
            break;
    }

    if (done != 0) {
        syslog(LOG_WARNING, "Failed to lock mmap file\n");
        close(fd);
        return -1;
    }

    /* Get the size of the file. */
    status = fstat (fd, &s);
    size = s.st_size;
    size += pagesize-(size%pagesize);

    map = mmap(0, size,  PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        syslog(LOG_ERR, "mmap failed, errno: %d", errno);
        return -1;
    }

    offset = get_file_offset(FILE_PSU);

    psu_info_t *psu_info = (psu_info_t*)(map+offset);

    for (int i = 0; i < num_status; i++) {
        printf("%s: %d\n", status_info[i].status_desc, psu_info[psu_num].status_cntr[status_info[i].status]);
    }
    printf("\n");

    memset(psu_info[psu_num].status_cntr, 0, PSU_STATUS_MAX);

    flock(fd, LOCK_UN);
    if (munmap(map, size) == -1) {
        syslog(LOG_WARNING, "Error un-mmapping the file");
    }

    close(fd);
    return 0;
}


uint8_t crc8(const void* vptr, int len) {
  const uint8_t *data = vptr;
  unsigned crc = 0;
  int i, j;
  for (j = len; j; j--, data++) {
    crc ^= (*data << 8);
    for(i = 8; i; i--) {
      if (crc & 0x8000)
        crc ^= (0x1070 << 3);
      crc <<= 1;
    }
  }
  return (uint8_t)(crc >> 8);
}
