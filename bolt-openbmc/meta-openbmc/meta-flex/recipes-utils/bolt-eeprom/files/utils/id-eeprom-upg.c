/*
 * Copyright 2017-present Flextronics. All Rights Reserved.
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
 *
 * Program the Bolt manufacturing EEPROM
 *
 * Author: Adrian Brathwaite (adrian.brathwaite@flextronics.com)
 */

#include <facebook/flex_eeprom.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define FLX_EEV2_F_MAGIC 2
#define FLX_EEV2_F_VERSION 1
#define FLX_EEV2_F_PRODUCT_NAME 12
#define FLX_EEV2_F_PRODUCT_NUMBER 12
#define FLX_EEV2_F_ASSEMBLY_NUMBER 12
#define FLX_EEV2_F_FACEBOOK_PCBA_NUMBER 12
#define FLX_EEV2_F_FACEBOOK_PCB_NUMBER 11
#define FLX_EEV2_F_ODM_PCBA_NUMBER 12
#define FLX_EEV2_F_ODM_PCBA_SERIAL 12
#define FLX_EEV2_F_PRODUCT_STATE 1
#define FLX_EEV2_F_PRODUCT_VERSION 1
#define FLX_EEV2_F_PRODUCT_SUBVERSION 1
#define FLX_EEV2_F_PRODUCT_SERIAL 16
#define FLX_EEV2_F_PRODUCT_SERIAL_EXTRA 12
#define FLX_EEV2_F_PRODUCT_ASSET 12

#define FLX_EEV2_F_SYSTEM_MANUFACTURER 8
#define FLX_EEV2_F_SYSTEM_MANU_DATE 4
#define FLX_EEV2_F_PCB_MANUFACTURER 8
#define FLX_EEV2_F_ASSEMBLED 8
#define FLX_EEV2_F_LOCAL_MAC 12
#define FLX_EEV2_F_EXT_MAC_BASE 12
#define FLX_EEV2_F_EXT_MAC_SIZE 2

#define FLX_EEV2_F_LOCATION 8
#define FLX_EEV2_F_CRC8 1

#define FLX_EEV5_F_MAGIC 2
#define FLX_EEV5_F_VERSION 1
#define FLX_EEV5_F_PRODUCT_NAME 12
#define FLX_EEV5_F_PRODUCT_NUMBER 12
#define FLX_EEV5_F_ASSEMBLY_NUMBER 12
#define FLX_EEV5_F_FACEBOOK_PCBA_NUMBER 12
#define FLX_EEV5_F_FACEBOOK_PCB_NUMBER 11
#define FLX_EEV5_F_ODM_PCBA_NUMBER 12
#define FLX_EEV5_F_ODM_PCBA_SERIAL 12
#define FLX_EEV5_F_PRODUCT_STATE 1
#define FLX_EEV5_F_PRODUCT_VERSION 1
#define FLX_EEV5_F_PRODUCT_SUBVERSION 1
#define FLX_EEV5_F_PRODUCT_SERIAL 12
#define FLX_EEV5_F_PRODUCT_ASSET 12

#define FLX_EEV5_F_SYSTEM_MANUFACTURER 10
#define FLX_EEV5_F_SYSTEM_MANU_DATE 4
#define FLX_EEV5_F_PCB_MANUFACTURER 8
#define FLX_EEV5_F_ASSEMBLED 13
#define FLX_EEV5_F_LOCAL_MAC 12
#define FLX_EEV5_F_EXT_MAC_BASE 12
#define FLX_EEV5_F_EXT_MAC_SIZE 1

#define FLX_EEV5_F_LOCATION 8
#define FLX_EEV5_F_CRC8 1

#define FLX_EE_LI_V1_F_MAGIC 4 
#define FLX_EE_LI_V1_F_VERSION 2
#define FLX_EE_LI_V1_F_PRODUCT_NAME 16
#define FLX_EE_LI_V1_F_PRODUCT_NUMBER 16
#define FLX_EE_LI_V1_F_ASSEMBLY_NUMBER 12
#define FLX_EE_LI_V1_F_FACEBOOK_PCBA_NUMBER 14
#define FLX_EE_LI_V1_F_FACEBOOK_PCB_NUMBER 14
#define FLX_EE_LI_V1_F_ODM_PCBA_NUMBER 14
#define FLX_EE_LI_V1_F_ODM_PCBA_SERIAL 12
#define FLX_EE_LI_V1_F_PRODUCT_STATE 2
#define FLX_EE_LI_V1_F_PRODUCT_VERSION 1
#define FLX_EE_LI_V1_F_PRODUCT_SUBVERSION 1
#define FLX_EE_LI_V1_F_PRODUCT_SERIAL 16
#define FLX_EE_LI_V1_F_PRODUCT_ASSET 12
#define FLX_EE_LI_V1_F_SYSTEM_MANUFACTURER 8 
#define FLX_EE_LI_V1_F_SYSTEM_MANU_DATE 8
#define FLX_EE_LI_V1_F_PCB_MANUFACTURER 8
#define FLX_EE_LI_V1_F_ASSEMBLED 8
#define FLX_EE_LI_V1_F_LOCAL_MAC 12
#define FLX_EE_LI_V1_F_EXT_MAC_BASE 12
#define FLX_EE_LI_V1_F_EXT_MAC_SIZE  2
#define FLX_EE_LI_V1_F_LOCATION 8
#define FLX_EE_LI_V1_F_CRC8 1

#define BOARD_ID_EEPROM_TXT_FILE "/usr/local/bin/bmc-id-eeprom.example"
#define LI_EEPROM_V2_STR   0x3032

FILE *ftxt;

/* Version 2 of the eeprom */
struct bolt_eeprom_v2_st {
  /* version number of the eeprom. Must be the first element */
  uint8_t fbw_version;

  /* Product Name */
  char fbw_product_name[FLX_EEV2_F_PRODUCT_NAME + 1];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char fbw_product_number[FLX_EEV2_F_PRODUCT_NUMBER + 2];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char fbw_assembly_number[FLX_EEV2_F_ASSEMBLY_NUMBER + 2];

  /* Facebook PCBA Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcba_number[FLX_EEV2_F_FACEBOOK_PCBA_NUMBER + 2];

  /* Facebook PCB Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcb_number[FLX_EEV2_F_FACEBOOK_PCB_NUMBER + 3];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_number[FLX_EEV2_F_ODM_PCBA_NUMBER + 2];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_serial[FLX_EEV2_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  uint8_t fbw_production_state;

  /* Product Version */
  uint8_t fbw_product_version;

  /* Product Sub Version */
  uint8_t fbw_product_subversion;

  /* Product Serial Number: XXXXXXXX */
  char fbw_product_serial[FLX_EEV2_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char fbw_product_asset[FLX_EEV2_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char fbw_system_manufacturer[FLX_EEV2_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date: mm-dd-yy */
  uint8_t fbw_system_manufacturing_date[10];

  /* PCB Manufacturer: XXXXXXXXX */
  char fbw_pcb_manufacturer[FLX_EEV2_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char fbw_assembled[FLX_EEV2_F_ASSEMBLED + 1];

  /* Local MAC Address */
  uint8_t fbw_local_mac[6];

  /* Extended MAC Address */
  uint8_t fbw_mac_base[6];

  /* Extended MAC Address Size */
  uint16_t fbw_mac_size;

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char fbw_location[FLX_EEV2_F_LOCATION + 1];

  /* CRC8 */
  uint8_t fbw_crc8;
};

struct bolt_eeprom_v5_st {
  /* version number of the eeprom. Must be the first element */
  uint8_t fbw_version;

  /* Product Name */
  char fbw_product_name[FLX_EEV5_F_PRODUCT_NAME + 1];

  /* Hardware Revision */
  char fbw_hw_rev[FLEX_EEPROM_F_HW_REV + 1];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char fbw_product_number[FLX_EEV5_F_PRODUCT_NUMBER + 2];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char fbw_assembly_number[FLX_EEV5_F_ASSEMBLY_NUMBER + 2];

  /* Facebook PCBA Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcba_number[FLX_EEV5_F_FACEBOOK_PCBA_NUMBER + 2];

  /* Facebook PCB Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcb_number[FLX_EEV5_F_FACEBOOK_PCB_NUMBER + 3];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_number[FLX_EEV5_F_ODM_PCBA_NUMBER + 2];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_serial[FLX_EEV5_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  uint8_t fbw_production_state;

  /* Product Version */
  char fbw_product_version[FLEX_EEPROM_F_PROD_VERS + 1];

  /* Product Serial Number: XXXXXXXX */
  char fbw_product_serial[FLX_EEV5_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char fbw_product_asset[FLX_EEV5_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char fbw_system_manufacturer[FLX_EEV5_F_SYSTEM_MANUFACTURER + 1];

  /* New System Manufacturing Date: Day, Month DayofMonth Year */
  char fbw_system_manufacturing_date[FBW_EEPROM_F_SYSTEM_MANU_DATE_v3 + 5];

  /* PCB Manufacturer: XXXXXXXXX */
  char fbw_pcb_manufacturer[FLX_EEV5_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char fbw_assembled[FLX_EEV5_F_ASSEMBLED + 1];

  /* Local MAC Address */
  uint8_t fbw_local_mac[6];

  /* Extended MAC Address */
  uint8_t fbw_mac_base[6];

  /* Extended MAC Address Size */
  uint8_t fbw_mac_size;

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char fbw_location[FLX_EEV5_F_LOCATION + 1];

  /* CRC8 */
  uint8_t fbw_crc8;
};

struct bolt_eeprom_li_v1 {
  /* version number of the eeprom */
  uint16_t fbw_version;

  /* Product Name */
  char fbw_product_name[FLX_EE_LI_V1_F_PRODUCT_NAME + 1];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char fbw_product_number[FLX_EE_LI_V1_F_PRODUCT_NUMBER + 1];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char fbw_assembly_number[FLX_EE_LI_V1_F_ASSEMBLY_NUMBER + 1];

  /* Facebook PCBA Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcba_number[FLX_EE_LI_V1_F_FACEBOOK_PCBA_NUMBER + 1];

  /* Facebook PCB Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcb_number[FLX_EE_LI_V1_F_FACEBOOK_PCB_NUMBER + 1];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_number[FLX_EE_LI_V1_F_ODM_PCBA_NUMBER + 1];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_serial[FLX_EE_LI_V1_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  char fbw_production_state[FLX_EE_LI_V1_F_PRODUCT_STATE + 1];

  /* Product Version */
  char fbw_product_version[FLX_EE_LI_V1_F_PRODUCT_VERSION + 1];

  /* Product Sub Version */
  char fbw_product_subversion[FLX_EE_LI_V1_F_PRODUCT_SUBVERSION + 1];

  /* Product Serial Number: XXXXXXXX */
  char fbw_product_serial[FLX_EE_LI_V1_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char fbw_product_asset[FLX_EE_LI_V1_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char fbw_system_manufacturer[FLX_EE_LI_V1_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date: mm-dd-yyyy */
  char fbw_system_manufacturing_date[FLX_EE_LI_V1_F_SYSTEM_MANU_DATE + 1];

  /* PCB Manufacturer: XXXXXXXXX */
  char fbw_pcb_manufacturer[FLX_EE_LI_V1_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char fbw_assembled[FLX_EE_LI_V1_F_ASSEMBLED + 1];

  /* Local MAC Address */
  char fbw_local_mac[FLX_EE_LI_V1_F_LOCAL_MAC + 1];

  /* Extended MAC Address */
  char fbw_mac_base[FLX_EE_LI_V1_F_EXT_MAC_BASE + 1];

  /* Extended MAC Address Size */
  char fbw_mac_size[FLX_EE_LI_V1_F_EXT_MAC_SIZE + 1];

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char fbw_location[FLX_EE_LI_V1_F_LOCATION + 1];

  /* CRC8 */
  char fbw_crc8[FLX_EE_LI_V1_F_CRC8 + 1];
};

static inline void flx_copy_uint8(uint8_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = **src;
  (*src) += src_len;
}

static inline void flx_copy_uint16(uint16_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src) | ((*(*src + 1)) << 8);
  (*src) += src_len;
}

static inline void flx_strcpy(char *dst, int dst_len,
                              const uint8_t **src, int src_len)
{
//  assert(dst_len >= src_len + 1);    /* larger because of '\0' */
  strncpy(dst, (char *)*src, src_len);
  dst[src_len + 1] = '\0';
  (*src) += src_len;
}

static inline void flx_copy_product_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PAXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void flx_copy_assembly_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PAXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void flx_copy_facebook_pcba_part(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format of PBXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-'; 
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void flx_copy_facebook_pcb_part(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 11 chars in the format of PAXX-XXXXX-XX, 3 additional chars */
  assert(dst_len >= src_len + 3);
  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 5; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void flx_copy_date_v5(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 chars in the format Day Mon DayofMonth, Year, 5 additional chars */
  assert(dst_len >= src_len + 5);

  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ' ';

  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ' ';

  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = ',';
  *dst++ = ' ';

  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }

  *dst = '\0';

  (*src) += src_len;
}

static inline void flx_copy_date_v2(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  const uint8_t *cur = *src;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  /* mm-dd-yy in output */
  assert(dst_len >= 9);
  /* input is 4 bytes YY YY MM DD */
  assert(src_len >= 4);
  flx_copy_uint16(&year, &cur, 2);
  flx_copy_uint8(&month, &cur, 1);
  flx_copy_uint8(&day, &cur, 1);
  snprintf(dst, dst_len, "%02d-%02d-%02d", month % 13, day % 32, year % 100);
  (*src) += src_len;
}

static inline uint8_t _a2v(const uint8_t *a)
{
  uint8_t v = *a;
  if ('0' <= v && v <= '9') {
    return v - '0';
  }
  if ('a' <= v && v <= 'z') {
    return v - 'a' + 10;
  }
  if ('A' <= v && v <= 'Z') {
    return v - 'A' + 10;
  }
  return 0;
}

static inline void flx_copy_mac(
    uint8_t* dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;

  assert(dst_len >= 6);
  assert(src_len >= 12);

  for (i = 0; i < 6; i++) {
    *dst = (_a2v(cur) << 4) | _a2v(cur + 1);
    dst++;
    cur +=2 ;
  }
  (*src) += src_len;
}

static int
v2_parse_buf(
             const uint8_t *buf,
             int len,
             struct bolt_eeprom_v2_st *eeprom)
{
  int rc = 0;
  const uint8_t* cur = buf;
  uint16_t magic;
  int crc_len;
  uint8_t crc8;

  memset(eeprom, 0, sizeof(*eeprom));

  /* Verify the magic number */
  flx_copy_uint16(&magic, &cur, FLX_EEV2_F_MAGIC);
  if (magic != FBW_EEPROM_MAGIC_NUM) {
    rc = EFAULT;
    syslog(LOG_WARNING, "Unexpected magic word 0x%x", magic);
    goto out;
  }

  /* Confirm the version number, should be 2 */
  flx_copy_uint8(&eeprom->fbw_version, &cur, FLX_EEV2_F_VERSION);
  syslog(LOG_NOTICE, "Detected EEPROM version %d", eeprom->fbw_version);
  if (eeprom->fbw_version != FBW_EEPROM_VERSION2) {
    syslog(LOG_WARNING, "Unsupported version %d for this upgrade",
           eeprom->fbw_version);
    goto out;
  } else {
    crc_len = FBW_EEPROM_V2_SIZE;
  }
  syslog(LOG_NOTICE, "Read EEPROM version     : %d",
         eeprom->fbw_version);

  /* Now read the eeprom and store the contents */
  /* Product name: ASCII for 12 characters */
  flx_strcpy(eeprom->fbw_product_name,
             sizeof(eeprom->fbw_product_name),
             &cur, FLX_EEV2_F_PRODUCT_NAME);
  syslog(LOG_NOTICE, "V2 EEPROM product name  : %s",
         eeprom->fbw_product_name);

  /* Product Part #: 12 byte data shown as PAXXXXXXX-000 */
  flx_copy_product_number(eeprom->fbw_product_number,
                          sizeof(eeprom->fbw_product_number),
                          &cur, FLX_EEV2_F_PRODUCT_NUMBER);
  syslog(LOG_NOTICE, "V2 EEPROM product num   : %s",
         eeprom->fbw_product_number);

  /* System Assembly Part Number: XXX-XXXXXX-XX */
  flx_copy_assembly_number(eeprom->fbw_assembly_number,
                           sizeof(eeprom->fbw_assembly_number),
                           &cur, FLX_EEV2_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  flx_copy_facebook_pcba_part(eeprom->fbw_facebook_pcba_number,
                              sizeof(eeprom->fbw_facebook_pcba_number),
                              &cur, FLX_EEV2_F_FACEBOOK_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  flx_copy_facebook_pcb_part(eeprom->fbw_facebook_pcb_number,
                             sizeof(eeprom->fbw_facebook_pcb_number),
                             &cur, FLX_EEV2_F_FACEBOOK_PCB_NUMBER);

  /* Flex PCBA Part Number: PAXX-XXXXX-XX */
  flx_copy_facebook_pcba_part(eeprom->fbw_odm_pcba_number,
                              sizeof(eeprom->fbw_odm_pcba_number),
                              &cur, FLX_EEV2_F_ODM_PCBA_NUMBER);

  /* Flex PCBA Serial Number: XXXXXXXXXXXX */
  flx_strcpy(eeprom->fbw_odm_pcba_serial,
             sizeof(eeprom->fbw_odm_pcba_serial),
             &cur, FLX_EEV2_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  flx_copy_uint8(&eeprom->fbw_production_state,
                 &cur, FLX_EEV2_F_PRODUCT_STATE);

  /* Product Version */
  flx_copy_uint8(&eeprom->fbw_product_version,
                 &cur, FLX_EEV2_F_PRODUCT_VERSION);

  /* Product Sub Version */
  flx_copy_uint8(&eeprom->fbw_product_subversion,
                 &cur, FLX_EEV2_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_serial,
             sizeof(eeprom->fbw_product_serial),
             &cur, FLX_EEV2_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "V2 EEPROM product ser   : %s",
         eeprom->fbw_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_asset,
             sizeof(eeprom->fbw_product_asset),
             &cur, FLX_EEV2_F_PRODUCT_ASSET);
  /* System Manufacturer: XXXXXXXX */
  flx_strcpy(eeprom->fbw_system_manufacturer,
             sizeof(eeprom->fbw_system_manufacturer),
             &cur, FLX_EEV2_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yy */
  flx_copy_date_v2(eeprom->fbw_system_manufacturing_date,
                   sizeof(eeprom->fbw_system_manufacturing_date),
                   &cur, FLX_EEV2_F_SYSTEM_MANU_DATE);
  syslog(LOG_NOTICE, "V2 EEPROM Mfg Date      : %s",
         eeprom->fbw_system_manufacturing_date);

  /* PCB Manufacturer: XXXXXXXXX */
  flx_strcpy(eeprom->fbw_pcb_manufacturer,
             sizeof(eeprom->fbw_pcb_manufacturer),
             &cur, FLX_EEV2_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  flx_strcpy(eeprom->fbw_assembled,
             sizeof(eeprom->fbw_assembled),
             &cur, FLX_EEV2_F_ASSEMBLED);

  /* Local MAC Address */
  flx_copy_mac(eeprom->fbw_local_mac,
               sizeof(eeprom->fbw_local_mac),
               &cur, FLX_EEV2_F_LOCAL_MAC);

  /* Extended MAC Address */
  flx_copy_mac(eeprom->fbw_mac_base,
               sizeof(eeprom->fbw_mac_base),
               &cur, FLX_EEV2_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  flx_copy_uint16(&eeprom->fbw_mac_size,
                  &cur,FLX_EEV2_F_EXT_MAC_SIZE);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  flx_strcpy(eeprom->fbw_location,
             sizeof(eeprom->fbw_location),
             &cur, FLX_EEV2_F_LOCATION);

  /* CRC8 */
  flx_copy_uint8(&eeprom->fbw_crc8,
                 &cur, FLX_EEV2_F_CRC8);

  syslog(LOG_NOTICE, "V2 EEPROM CRC           : 0x%x", eeprom->fbw_crc8);

 out:
  return rc;
}

int
v2_parse(struct bolt_eeprom_v2_st *v2)
{
  int rc = 0;
  const char *fn = FBW_EEPROM_FILE;
  uint32_t len;
  FILE *fin;
  char buf[FBW_EEPROM_SIZE];

  if (!v2) {
    return -EINVAL;
  }

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", FBW_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", FBW_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < FBW_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", FBW_EEPROM_FILE,
            len, FBW_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = v2_parse_buf((const uint8_t *)buf, sizeof(buf), v2);
  if (rc) {
    goto out;
  }

 out:

  if (fin) {
    fclose(fin);
  }

  return rc;
}

static int
v5_parse_buf(
             const uint8_t *buf,
             int len,
             struct bolt_eeprom_v5_st *eeprom)
{
  int rc = 0;
  const uint8_t* cur = buf;
  uint16_t magic;
  int crc_len;
  uint8_t crc8;

  memset(eeprom, 0, sizeof(struct bolt_eeprom_v5_st));

  /* Verify the magic number */
  flx_copy_uint16(&magic, &cur, FLX_EEV5_F_MAGIC);
  if (magic != FLEX_EEPROM_MAGIC_NUM) {
    rc = EFAULT;
    syslog(LOG_WARNING, "Unexpected magic word 0x%x", magic);
    goto out;
  }

  /* Confirm the version number, should be 2 */
  flx_copy_uint8(&eeprom->fbw_version, &cur, FLX_EEV5_F_VERSION);
  syslog(LOG_NOTICE, "Read EEPROM version     : %d",
         eeprom->fbw_version);
  if (eeprom->fbw_version != FBW_EEPROM_VERSION5) {
    syslog(LOG_WARNING, "Expected EEPROM version 5, exiting");
    rc = EEXIST;
    goto out;
  }

  /* Product name: ASCII for 12 characters */
  flx_strcpy(eeprom->fbw_product_name,
             sizeof(eeprom->fbw_product_name),
             &cur, FLX_EEV5_F_PRODUCT_NAME);
  syslog(LOG_NOTICE, "V5 EEPROM product name  : %s",
         eeprom->fbw_product_name);

  /* Hardware Revsion */
  flx_strcpy(eeprom->fbw_hw_rev,
             sizeof(eeprom->fbw_hw_rev),
             &cur, FLEX_EEPROM_F_HW_REV);
  syslog(LOG_NOTICE, "V5 EEPROM hardware rev  : %s",
         eeprom->fbw_hw_rev);

  /* Product Part #: 12 byte data shown as PAXXXXXXX-000 */
  flx_copy_product_number(eeprom->fbw_product_number,
                          sizeof(eeprom->fbw_product_number),
                          &cur, FLX_EEV5_F_PRODUCT_NUMBER);
  syslog(LOG_NOTICE, "V5 EEPROM product num   : %s",
         eeprom->fbw_product_number);

  /* System Assembly Part Number: XXX-XXXXXX-XX */
  flx_copy_assembly_number(eeprom->fbw_assembly_number,
                           sizeof(eeprom->fbw_assembly_number),
                           &cur, FLX_EEV5_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  flx_copy_facebook_pcba_part(eeprom->fbw_facebook_pcba_number,
                              sizeof(eeprom->fbw_facebook_pcba_number),
                              &cur, FLX_EEV5_F_FACEBOOK_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  flx_copy_facebook_pcb_part(eeprom->fbw_facebook_pcb_number,
                             sizeof(eeprom->fbw_facebook_pcb_number),
                             &cur, FLX_EEV5_F_FACEBOOK_PCB_NUMBER);

  /* Flex PCBA Part Number: PAXX-XXXXX-XX */
  flx_copy_facebook_pcba_part(eeprom->fbw_odm_pcba_number,
                              sizeof(eeprom->fbw_odm_pcba_number),
                              &cur, FLX_EEV5_F_ODM_PCBA_NUMBER);

  /* Flex PCBA Serial Number: XXXXXXXXXXXX */
  flx_strcpy(eeprom->fbw_odm_pcba_serial,
             sizeof(eeprom->fbw_odm_pcba_serial),
             &cur, FLX_EEV5_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  flx_copy_uint8(&eeprom->fbw_production_state,
                 &cur, FLX_EEV5_F_PRODUCT_STATE);

  /* Product Version */
  flx_strcpy(eeprom->fbw_product_version,
             sizeof(eeprom->fbw_product_version),
             &cur, FLEX_EEPROM_F_PROD_VERS);

  /* Product Serial Number: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_serial,
             sizeof(eeprom->fbw_product_serial),
             &cur, FLX_EEV5_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "V5 EEPROM product ser   : %s",
         eeprom->fbw_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_asset,
             sizeof(eeprom->fbw_product_asset),
             &cur, FLX_EEV5_F_PRODUCT_ASSET);
  syslog(LOG_NOTICE, "V5 EEPROM product asset : %s",
         eeprom->fbw_product_asset);

  /* System Manufacturer: XXXXXXXX */
  flx_strcpy(eeprom->fbw_system_manufacturer,
             sizeof(eeprom->fbw_system_manufacturer),
             &cur, FLX_EEV5_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yy */
  flx_copy_date_v5(eeprom->fbw_system_manufacturing_date,
                   sizeof(eeprom->fbw_system_manufacturing_date),
                   &cur, FBW_EEPROM_F_SYSTEM_MANU_DATE_v3);
  syslog(LOG_NOTICE, "V5 EEPROM Mfg Date      : %s",
         eeprom->fbw_system_manufacturing_date);

  /* PCB Manufacturer: XXXXXXXXX */
  flx_strcpy(eeprom->fbw_pcb_manufacturer,
             sizeof(eeprom->fbw_pcb_manufacturer),
             &cur, FLX_EEV5_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  flx_strcpy(eeprom->fbw_assembled,
             sizeof(eeprom->fbw_assembled),
             &cur, FLX_EEV5_F_ASSEMBLED);

  /* Local MAC Address */
  flx_copy_mac(eeprom->fbw_local_mac,
               sizeof(eeprom->fbw_local_mac),
               &cur, FLX_EEV5_F_LOCAL_MAC);

  /* Extended MAC Address */
  flx_copy_mac(eeprom->fbw_mac_base,
               sizeof(eeprom->fbw_mac_base),
               &cur, FLX_EEV5_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  flx_copy_uint8(&eeprom->fbw_mac_size,
                 &cur,FLX_EEV5_F_EXT_MAC_SIZE);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  flx_strcpy(eeprom->fbw_location,
             sizeof(eeprom->fbw_location),
             &cur, FLX_EEV5_F_LOCATION);

  /* CRC8 */
  flx_copy_uint8(&eeprom->fbw_crc8,
                 &cur, FLX_EEV5_F_CRC8);

  syslog(LOG_NOTICE, "V5 EEPROM CRC           : 0x%x", eeprom->fbw_crc8);

 out:
  return rc;
}

int
v5_parse(struct bolt_eeprom_v5_st *v5)
{
  int rc = 0;
  const char *fn = FBW_EEPROM_FILE;
  uint32_t len;
  FILE *fin;
  char buf[FBW_EEPROM_SIZE];

  if (!v5) {
    return -EINVAL;
  }

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", FBW_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", FBW_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < FBW_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", FBW_EEPROM_FILE,
            len, FBW_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = v5_parse_buf((const uint8_t *)buf, sizeof(buf), v5);
  if (rc) {
    goto out;
  }

 out:
//  syslog(LOG_NOTICE, "wedge_eeprom_parse: return code 0x%x", rc);

  if (fin) {
    fclose(fin);
  }

  return rc;
}

static int
li_v1_parse_buf(
             const uint8_t *buf,
             int len,
             struct bolt_eeprom_li_v1 *eeprom)
{
  int rc = 0;
  const uint8_t* cur = buf;
  char magics[FLX_EE_LI_V1_F_MAGIC + 1];
  int crc_len;
  uint8_t crc8;

  memset(eeprom, 0, sizeof(struct bolt_eeprom_li_v1));
  memset(magics, 0, sizeof(magics));

  /* Verify the magic number */
  cur = buf;
  flx_strcpy(magics, sizeof(magics), &cur, FLX_EE_LI_V1_F_MAGIC);
  syslog(LOG_NOTICE, "li_v1_parse_buf(): Detected magic number   : %s", magics);
  if (!strcmp(LI_EEPROM_MAGIC_NUM, magics) == 0) {
    rc = EFAULT;
    syslog(LOG_WARNING, "Unexpected magic word %s", magics);
    goto out;
  }

  /* Confirm the version number, should be 2 */
  flx_copy_uint16(&eeprom->fbw_version, &cur, FLX_EE_LI_V1_F_VERSION);
  eeprom->fbw_version = ntohs(eeprom->fbw_version);
  syslog(LOG_NOTICE, "Read EEPROM version     : %d",
         eeprom->fbw_version);
  if (eeprom->fbw_version != LI_EEPROM_VERSION1) {
    syslog(LOG_WARNING, "Expected EEPROM version 1, exiting");
    rc = EEXIST;
    goto out;
  }

  flx_strcpy(eeprom->fbw_product_name,
             sizeof(eeprom->fbw_product_name),
             &cur, FLX_EE_LI_V1_F_PRODUCT_NAME);
  syslog(LOG_NOTICE, "LI V1 EEPROM product name  : %s",
         eeprom->fbw_product_name);

  flx_strcpy(eeprom->fbw_product_number,
             sizeof(eeprom->fbw_product_number),
             &cur, FLX_EE_LI_V1_F_PRODUCT_NUMBER);
  syslog(LOG_NOTICE, "LI V1 EEPROM product num   : %s",
         eeprom->fbw_product_number);

  /* System Assembly Part Number */
  flx_strcpy(eeprom->fbw_assembly_number,
             sizeof(eeprom->fbw_assembly_number),
             &cur, FLX_EE_LI_V1_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  flx_strcpy(eeprom->fbw_facebook_pcba_number,
                              sizeof(eeprom->fbw_facebook_pcba_number),
                              &cur, FLX_EE_LI_V1_F_FACEBOOK_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  flx_strcpy(eeprom->fbw_facebook_pcb_number,
                             sizeof(eeprom->fbw_facebook_pcb_number),
                             &cur, FLX_EE_LI_V1_F_FACEBOOK_PCB_NUMBER);

  /* Flex PCBA Part Number: PAXX-XXXXX-XX */
  flx_strcpy(eeprom->fbw_odm_pcba_number,
                              sizeof(eeprom->fbw_odm_pcba_number),
                              &cur, FLX_EE_LI_V1_F_ODM_PCBA_NUMBER);

  /* Flex PCBA Serial Number: XXXXXXXXXXXX */
  flx_strcpy(eeprom->fbw_odm_pcba_serial,
             sizeof(eeprom->fbw_odm_pcba_serial),
             &cur, FLX_EE_LI_V1_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  flx_strcpy(eeprom->fbw_production_state,
             sizeof(eeprom->fbw_production_state),
             &cur, FLX_EE_LI_V1_F_PRODUCT_STATE);

  /* Product Version */
  flx_strcpy(eeprom->fbw_product_version,
             sizeof(eeprom->fbw_product_version),
             &cur, FLX_EE_LI_V1_F_PRODUCT_VERSION);

  /* Product subversion */
  flx_strcpy(eeprom->fbw_product_subversion,
             sizeof(eeprom->fbw_product_subversion),
             &cur, FLX_EE_LI_V1_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_serial,
             sizeof(eeprom->fbw_product_serial),
             &cur, FLX_EE_LI_V1_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "Li V1 EEPROM product ser   : %s",
         eeprom->fbw_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  flx_strcpy(eeprom->fbw_product_asset,
             sizeof(eeprom->fbw_product_asset),
             &cur, FLX_EE_LI_V1_F_PRODUCT_ASSET);
  syslog(LOG_NOTICE, "Li V1 EEPROM product asset : %s",
         eeprom->fbw_product_asset);

  /* System Manufacturer: XXXXXXXX */
  flx_strcpy(eeprom->fbw_system_manufacturer,
             sizeof(eeprom->fbw_system_manufacturer),
             &cur, FLX_EE_LI_V1_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yy */
  flx_strcpy(eeprom->fbw_system_manufacturing_date,
                   sizeof(eeprom->fbw_system_manufacturing_date),
                   &cur, FLX_EE_LI_V1_F_SYSTEM_MANU_DATE);
  syslog(LOG_NOTICE, "Li V1 EEPROM Mfg Date      : %s",
         eeprom->fbw_system_manufacturing_date);


  /* PCB Manufacturer: XXXXXXXXX */
  flx_strcpy(eeprom->fbw_pcb_manufacturer,
             sizeof(eeprom->fbw_pcb_manufacturer),
             &cur, FLX_EE_LI_V1_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  flx_strcpy(eeprom->fbw_assembled,
             sizeof(eeprom->fbw_assembled),
             &cur, FLX_EE_LI_V1_F_ASSEMBLED);

  /* Local MAC Address */
  flx_strcpy(eeprom->fbw_local_mac,
               sizeof(eeprom->fbw_local_mac),
               &cur, FLX_EE_LI_V1_F_LOCAL_MAC);

  /* Extended MAC Address */
  flx_strcpy(eeprom->fbw_mac_base,
               sizeof(eeprom->fbw_mac_base),
               &cur, FLX_EE_LI_V1_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  flx_strcpy(eeprom->fbw_mac_size,
             sizeof(eeprom->fbw_mac_size),
                 &cur,FLX_EE_LI_V1_F_EXT_MAC_SIZE);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  flx_strcpy(eeprom->fbw_location,
             sizeof(eeprom->fbw_location),
             &cur, FLX_EE_LI_V1_F_LOCATION);

  /* CRC8 */
  flx_strcpy(eeprom->fbw_crc8,
             sizeof(eeprom->fbw_crc8),
                 &cur, FLX_EE_LI_V1_F_CRC8);
 out:
  return rc;
}

int
li_v1_parse(struct bolt_eeprom_li_v1 *v1)
{
  int rc = 0;
  const char *fn = FBW_EEPROM_FILE;
  uint32_t len;
  FILE *fin;
  char buf[FBW_EEPROM_SIZE];

  if (!v1) {
    return -EINVAL;
  }

  /* 
   * read from eeprom
   */
  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", FBW_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", FBW_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < FBW_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", FBW_EEPROM_FILE,
            len, FBW_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = li_v1_parse_buf((const uint8_t *)buf, sizeof(buf), v1);
  if (rc) {
    goto out;
  }

 out:
//  syslog(LOG_NOTICE, "wedge_eeprom_parse: return code 0x%x", rc);

  if (fin) {
    fclose(fin);
  }

  return rc;
}

static int
get_version(uint8_t *eep_ver)
{
  int rc = 0;
  const char *fn = FBW_EEPROM_FILE;
  char buf[FBW_EEPROM_SIZE];
  FILE *fin;
  uint16_t magic;
  uint8_t vers = 0;
  uint32_t len;
  const uint8_t *cur = buf;
  uint16_t  version;

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", FBW_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to seek to the end of %s", FBW_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < FBW_EEPROM_SIZE) {
    rc = ENOSPC;
    syslog(LOG_WARNING, "File '%s' is too small (%u < %u)", FBW_EEPROM_FILE,
            len, FBW_EEPROM_SIZE);
    goto out;
  }

  syslog(LOG_NOTICE, "EEPROM file size        : %dKB", len/1024);
  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = 0;
  /* Get the magic number */
  flx_copy_uint16(&magic, &cur, FBW_EEPROM_F_MAGIC);
  syslog(LOG_NOTICE, "Detected magic number   : 0x%X", magic);

  if (magic == FLEX_EEPROM_MAGIC_NUM) {
    syslog(LOG_NOTICE, "Detected EEPROM version : 5");
    *eep_ver = FBW_EEPROM_VERSION5;
  } else if (magic == FBW_EEPROM_MAGIC_NUM) {
    syslog(LOG_NOTICE, "Detected EEPROM version : 2");
    *eep_ver = FBW_EEPROM_VERSION2;
  } else {
    /*
     * Could be the latest version.  Rewind the pointer and
     * try reading the magic again
     */
    char magics[FLX_EE_LI_V1_F_MAGIC + 1];
    cur = buf;

    flx_strcpy(magics, FLX_EE_LI_V1_F_MAGIC, &cur, FLX_EE_LI_V1_F_MAGIC);
    syslog(LOG_NOTICE, "LI Detected magic number   : %s", magics);
    if (strcmp(LI_EEPROM_MAGIC_NUM, magics) == 0) {
        flx_copy_uint16(&version, &cur, FLX_EE_LI_V1_F_VERSION);
        version = ntohs(version);
        syslog(LOG_NOTICE, "LI MAGIC WORD, Detected EEPROM version : 0x%X", version);
      
        if (version == LI_EEPROM_VERSION1) {
            *eep_ver = LI_EEPROM_VERSION1;
        } else {
            /* version: 02 */
            if (version == LI_EEPROM_V2_STR) {
                syslog(LOG_NOTICE, "LI MAGIC WORD, Detected EEPROM version 2");
                *eep_ver = LI_EEPROM_VERSION2;
            }
            else
                *eep_ver = 0;
        }
     }
 }

 out:
  if (fin) {
    fclose(fin);
  }

  syslog(LOG_NOTICE, "get_version returns version: 0x%X", version);
  return rc;
}

int main (int argc, char **argv)
{
  int rc = 0;
  const char *fn = FBW_EEPROM_FILE;
  char buf[FBW_EEPROM_SIZE];
  struct wedge_eeprom_gn eeprom;
  struct bolt_eeprom_v2_st v2_ee;
  struct bolt_eeprom_v5_st v5_ee;
  struct bolt_eeprom_li_v1 li_v1_ee;
  FILE *fin;
  uint8_t crc_len;
  uint8_t eep_version;
  char tmp_buf[2];
  int i;
  /* Serial number update */
  int serial_upg = 0;

  if (argc == 2) {
     if (!strcmp(argv[1], "-s")) {
         /* 
          * read serial number from example file
          */
         printf("\nupdate serial numbers from example file");
         serial_upg = 1;
     }
  }

  /* First get the current eeprom version */ 
  rc = get_version(&eep_version);
  if (rc) {
    syslog(LOG_WARNING, "Failed to read current EEPROM version");
    printf("Failed to read current EEPROM version\n");
    goto out;
  }

  switch (eep_version) {
  case FBW_EEPROM_VERSION2:
    rc = v2_parse(&v2_ee);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse V2 EEPROM");
      printf("Failed to parse V2 EEPROM\n");
      goto out;
    }

    syslog(LOG_NOTICE, "Storing these EEPROM v2 parameters\n");
    syslog(LOG_NOTICE, "%-24s : %s\n", "Flex PCBA Serial Number", v2_ee.fbw_odm_pcba_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "System Serial Number", v2_ee.fbw_product_serial);
    syslog(LOG_NOTICE, "%-24s : %02X:%02X:%02X:%02X:%02X:%02X\n", "BMC MAC",
           v2_ee.fbw_local_mac[0], v2_ee.fbw_local_mac[1],
           v2_ee.fbw_local_mac[2], v2_ee.fbw_local_mac[3],
           v2_ee.fbw_local_mac[4], v2_ee.fbw_local_mac[5]);

    break;

  case FBW_EEPROM_VERSION5:
    rc = v5_parse(&v5_ee);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse V5 EEPROM");
      printf("Failed to parse V5 EEPROM\n");
      goto out;
    }

    syslog(LOG_NOTICE, "Storing these EEPROM v5 parameters\n");
    syslog(LOG_NOTICE, "%-24s : %s\n", "Flex PCBA Serial Number", v5_ee.fbw_odm_pcba_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "System Serial Number", v5_ee.fbw_product_serial);
    syslog(LOG_NOTICE, "%-24s : %02X:%02X:%02X:%02X:%02X:%02X\n", "BMC MAC",
           v5_ee.fbw_local_mac[0], v5_ee.fbw_local_mac[1],
           v5_ee.fbw_local_mac[2], v5_ee.fbw_local_mac[3],
           v5_ee.fbw_local_mac[4], v5_ee.fbw_local_mac[5]);

    break;

  case LI_EEPROM_VERSION1:
   printf("\n LinkedIn V2 eeprom upgrade ..."); 
   rc = li_v1_parse(&li_v1_ee);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse LI V1 EEPROM");
      printf("Failed to parse LI V1 EEPROM\n");
      goto out;
    }
    syslog(LOG_NOTICE, "Storing these EEPROM linked v1 parameters\n");
    syslog(LOG_NOTICE, "%-24s : %s\n", "Flex PCBA Serial Number", li_v1_ee.fbw_odm_pcba_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "System Serial Number", li_v1_ee.fbw_product_serial);
    syslog(LOG_NOTICE, "%-24s : %s\n", "BMC MAC", li_v1_ee.fbw_local_mac);
/*
    syslog(LOG_NOTICE, "%-24s : %02X:%02X:%02X:%02X:%02X:%02X\n", "BMC MAC",
           li_v1_ee.fbw_local_mac[0], li_v1_ee.fbw_local_mac[1],
           li_v1_ee.fbw_local_mac[2], li_v1_ee.fbw_local_mac[3],
           li_v1_ee.fbw_local_mac[4], li_v1_ee.fbw_local_mac[5]);
*/
    break;

  case LI_EEPROM_VERSION2:
    printf("EEPROM version 2 is current, no need to upgrade\n");
    syslog(LOG_NOTICE, "EEPROM version 2 is current, no need to upgrade");
    goto out;
    break;

  default:
    syslog(LOG_WARNING, "Invalid EEPROM version");
    return -1;
    break;
  }

  /* 
   * read from example file
   */
  ftxt = fopen(BOARD_ID_EEPROM_TXT_FILE, "r+");
  if (ftxt == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", BOARD_ID_EEPROM_TXT_FILE);
    printf("Failed to open %s\n", BOARD_ID_EEPROM_TXT_FILE);

    goto out;
  }

  fin = fopen(fn, "r+");
  if (fin == NULL) {
    rc = errno;
    syslog(LOG_WARNING, "Failed to open %s", FBW_EEPROM_FILE);
    goto out;
  }

  rc = fread(buf, 1, sizeof buf, fin);
  if (rc < sizeof buf) {
    syslog(LOG_WARNING, "Failed to complete read, only got %d bytes", rc);
    printf("Failed to complete read, only got %d bytes\n");
    rc = ENOSPC;
    goto out;
  }
  memcpy ((char*)&eeprom, (char*)buf+FBW_EEPROM_F_MAGIC, FBW_EEPROM_SIZE-FBW_EEPROM_F_MAGIC);

  /* go back to the beginning of the file */
  rewind(fin);

  copy_data (LI_EEPROM_MAGIC_NUM, buf, FBW_EEPROM_F_MAGIC);
  rc = fwrite(buf, 1, FBW_EEPROM_F_MAGIC, fin);
  if (rc < 2) {
    syslog(LOG_WARNING, "Failed to write magic number");
    printf("Failed to write magic number\n");
    goto out;
  }

  if (fbw_fill_struct_from_file (ftxt, &eeprom)) {
    goto out;
  } else {
    crc_len = LI_EEPROM_V2_SIZE;
  }

  /* Fill in the archived fields */
  syslog(LOG_NOTICE, "BEFORE RESTORING ARCHIVED ITEMS:");
  syslog(LOG_NOTICE, "Product Asset Tag     : %s",
         eeprom.fbw_product_asset);
  syslog(LOG_NOTICE, "Product Serial Num    : %s",
         eeprom.fbw_product_serial);
  syslog(LOG_NOTICE, "Manufacturing date    : %s",
         eeprom.fbw_system_manufacturing_date);
  syslog(LOG_NOTICE, "Fabric Location       : %s",
         eeprom.fbw_location);

  char product_asset[FBW_EEPROM_F_PRODUCT_ASSET];
  copy_data((char*)eeprom.fbw_product_asset,
            (char*)product_asset,
            sizeof eeprom.fbw_product_asset);

  switch (eep_version) {
    case FBW_EEPROM_VERSION2:
    copy_data((char*)v2_ee.fbw_product_serial,
              (char*)eeprom.fbw_product_serial,
              sizeof v2_ee.fbw_product_serial);
  
    copy_data((char*)v2_ee.fbw_odm_pcba_serial,
              (char*)eeprom.fbw_odm_pcba_serial,
              sizeof v2_ee.fbw_odm_pcba_serial);
  
    /* Convert int MAC to string MAC */
    memset(eeprom.fbw_local_mac, 0, sizeof eeprom.fbw_local_mac);
    for (i=0; i<6; i++) {
      sprintf(tmp_buf, "%02X", v2_ee.fbw_local_mac[i]);
      strcat(eeprom.fbw_local_mac, tmp_buf);
    }
    break;

  case FBW_EEPROM_VERSION5:
    copy_data((char*)v5_ee.fbw_product_serial,
              (char*)eeprom.fbw_product_serial,
              sizeof v5_ee.fbw_product_serial);
  
    copy_data((char*)v5_ee.fbw_odm_pcba_serial,
              (char*)eeprom.fbw_odm_pcba_serial,
              sizeof v5_ee.fbw_odm_pcba_serial);
  
    /* Convert int MAC to string MAC */
    memset(eeprom.fbw_local_mac, 0, sizeof eeprom.fbw_local_mac);
    for (i=0; i<6; i++) {
      sprintf(tmp_buf, "%02X", v5_ee.fbw_local_mac[i]);
      strcat(eeprom.fbw_local_mac, tmp_buf);
    }
    break;
  case LI_EEPROM_VERSION1:
    /* serial numbers update from eeprom or example file */
    if (serial_upg == 0) {
        /* serial numbers update from eeeprom */
        copy_data((char*)li_v1_ee.fbw_product_serial,
                  (char*)eeprom.fbw_product_serial,
                  sizeof li_v1_ee.fbw_product_serial);

        copy_data((char*)li_v1_ee.fbw_odm_pcba_serial,
                  (char*)eeprom.fbw_odm_pcba_serial,
                   sizeof li_v1_ee.fbw_odm_pcba_serial);
    }

    printf("\n fbw_product_serial: %s", eeprom.fbw_product_serial);
    printf("\n fbw_odm_pcba_serial: %s", eeprom.fbw_odm_pcba_serial);
    printf("\n fbw_local_mac: %s \n", li_v1_ee.fbw_local_mac);
    /* Copy MAC from eeprom */
    copy_data((char*)li_v1_ee.fbw_local_mac, 
              (char*)eeprom.fbw_local_mac,
               sizeof(eeprom.fbw_local_mac));
 
   /* Extended MAC Address from eeprom */
     copy_data((char*)li_v1_ee.fbw_local_mac, 
               (char*)eeprom.fbw_local_mac,
                sizeof(eeprom.fbw_local_mac));
    break;
  default:
    syslog(LOG_NOTICE, "Invalid EEPROM version %d", eep_version);
    printf("Invalid EEPROM version %d\n", eep_version);
    goto out;
  break;
  }

  syslog(LOG_NOTICE, "AFTER RESTORING ARCHIVED ITEMS:");
  syslog(LOG_NOTICE, "Product Asset Tag     : %s",
         eeprom.fbw_product_asset);
  syslog(LOG_NOTICE, "Product Serial Num    : %s",
         eeprom.fbw_product_serial);
  syslog(LOG_NOTICE, "Manufacturing date    : %s",
         eeprom.fbw_system_manufacturing_date);
  syslog(LOG_NOTICE, "Fabric Location       : %s",
         eeprom.fbw_location);

/*
  copy_data((char*)product_asset,
            (char*)eeprom.fbw_product_asset,
            sizeof product_asset);
  syslog(LOG_NOTICE, "Restored Asset Tag    : %s",
         eeprom.fbw_product_asset);
*/
  /* calculate based on all bytes but crc */
  /* validate on all bytes, resulting in crc value 0 */
  eeprom.fbw_crc8 = flx_crc8((void *) &eeprom, crc_len - 1) ^ FLEX_EEPROM_CRC8_CSUM;
//  eeprom.fbw_crc8 = flx_crc8((void *) &eeprom, crc_len - 1);
  ssize_t sz = sizeof eeprom;
  rc = fwrite(&eeprom, 1, sz, fin);
  if (rc < sz) {
      syslog(LOG_WARNING,
             "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes",
             sz, rc);
      printf("Failed to write entire EEPROM (%d bytes).  Wrote %d bytes\n",
             sz, rc);

      goto out;
  }
  syslog(LOG_NOTICE, "Wrote all %d bytes to EEPROM", rc); 
  rc = 0;

out:
  if (fin) {
    fclose(fin);
  }

  if (ftxt) {
    fclose(ftxt);
  }

  return rc;
}
