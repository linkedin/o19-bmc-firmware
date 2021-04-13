/*
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

#include "flex_eeprom.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>

#include <openbmc/log.h>

#define FLX_CRC8_WORKS

uint8_t flx_crc8(const uint8_t *buf, int len)
{
  uint8_t crc = 0;
  uint8_t tmp;

  while (len--) {
    uint8_t extract = *buf++;
    for (tmp = 8; tmp; tmp--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;

      if (sum) {
        crc ^= 0x8c;
      }

      extract >>= 1;
    }
  }
  return crc;
}

static inline void fbw_copy_uint8(uint8_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = **src;
  (*src) += src_len;
}

static inline void fbw_copy_uint16(uint16_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));

  *val = (**src) | ((*(*src + 1)) << 8);
  (*src) += src_len;
}

static inline void fbw_copy_uint32(uint32_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src)
    | ((*(*src + 1)) << 8)
    | ((*(*src + 2)) << 16)
    | ((*(*src + 3)) << 24);
  (*src) += src_len;
}

static inline void fbw_strcpy(char *dst, int dst_len,
                              const uint8_t **src, int src_len)
{
  assert(dst_len >= src_len + 1);    /* larger because of '\0' */
  strncpy(dst, (char *)*src, src_len);
  dst[src_len + 1] = '\0';
  (*src) += src_len;
}

static inline void fbw_copy_product_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 16 chars in the format of PAXXXXXXX-XXX, 2 additional chars */
  assert(dst_len >= src_len + 2);
  for (i = 0; i < 9; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 7; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void fbw_copy_assembly_number(
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

static inline void fbw_copy_facebook_pcba_part(
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

static inline void fbw_copy_facebook_pcb_part(
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

static inline void fbw_copy_date_v1(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 8 chars in the format DDMMYYYY, 3 additional chars */
  assert(dst_len >= src_len + 3);
  
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '/';
  
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '/';
  
  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }
  
  *dst = '\0';
  
  (*src) += src_len;
}

static inline void fbw_copy_date(
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
  fbw_copy_uint16(&year, &cur, 2);
  fbw_copy_uint8(&month, &cur, 1);
  fbw_copy_uint8(&day, &cur, 1);
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

static inline void fbw_copy_mac(
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

static int fbw_parse_buffer(
    const uint8_t *buf, int len, struct wedge_eeprom_st *eeprom) {
  int rc = 0;
  const uint8_t* cur = buf;
  char magic[FBW_EEPROM_F_MAGIC+1];
  int crc_len;
  uint8_t version;
  uint8_t crc8;
  uint8_t mac_offset;

  memset(eeprom, 0, sizeof(*eeprom));

  /* Verify the magic number is "LI19" */
  fbw_strcpy(magic, sizeof(magic), &cur, FBW_EEPROM_F_MAGIC);
  syslog(LOG_NOTICE, "EEPROM MAGIC NUM    : %s", magic);

  if (strncmp(LI_EEPROM_MAGIC_NUM, magic, FBW_EEPROM_F_MAGIC) != 0) {
    syslog(LOG_WARNING, "Unexpected magic word %s", magic);
    rc = EFAULT;

    #ifdef FBW_EEPROM_BOOT_WITH_BAD_MAGIC_NUM
      syslog(LOG_WARNING, "Booting with bad magic word");
    #else
      syslog(LOG_WARNING, "Parse ID EEPROM FAILED!");
      goto out;
    #endif
  }
  else {
      /* 
       * Found LI19 magic word
       *  LI V1 or V2 eeprom 
       */
      crc_len = LI_EEPROM_V1_SIZE; 
  }

  /* Confirm the version number.
   * Version 2 with LI19 magic word is the current production version.  There are 
   * versions 2 and 5 without LI19 magic word, and version 1 with LI19 magic word from preproduction 
   * that need to be scrubbed 
   */
  fbw_strcpy(eeprom->fbw_version, sizeof(eeprom->fbw_version), &cur, FBW_EEPROM_F_VERSION);
  syslog(LOG_NOTICE, "EEPROM version      : %s", eeprom->fbw_version);
  if (strcmp(eeprom->fbw_version, LI_EEPROM_VERSION2_STR)) {
      syslog(LOG_WARNING,
              "WARNING: EEPROM needs to be updated to production version 2!");
      printf("WARNING: BMC EEPROM needs to be updated to productin version 2!\n\n");
      
      cur = buf;
      fbw_copy_uint8(&version, &cur, FBW_OLD_VERSION_SIZE);
      version = ntohs(version); 
      syslog(LOG_WARNING,
             "WARNING: EEPROM needs to be updated to production version 2!");
      switch (version) { 
      case FBW_EEPROM_VERSION2:
          crc_len = FBW_EEPROM_V2_SIZE;
          break;

      case FBW_EEPROM_VERSION5:
          crc_len = FBW_EEPROM_V5_SIZE;
          break;

      default:
          rc = EFAULT;
          syslog(LOG_WARNING, "Parse ID EEPROM FAILED!");
          goto out;
      }
   }
   else
      crc_len = LI_EEPROM_V2_SIZE;

  assert(crc_len <= len);

// TODO: Fix CRC!!!
// Forget about it.  Can't get the CRC to work.  Different
//  sum gets generated each time even with the same (seeminly)
//  contents.  That's the first problem.  The second is no
//  matter what I calculate, I always read back a sum of zero
//  anyway.  Will have to use the I2C programmer to take a
//  look at what is actually being written into the EEROM 
//  because I don't yet know if the problem is in the reading
//  or the writing.
  /* check CRC */
#ifdef FLX_CRC8_WORKS
  crc8 = flx_crc8(buf+FBW_EEPROM_F_MAGIC, crc_len);

  if (crc8 != FLEX_EEPROM_CRC8_CSUM) {
    syslog(LOG_WARNING, "CRC check failed.  Expected 0x%x, got 0x%x", FLEX_EEPROM_CRC8_CSUM, crc8);
    // XXX rc = EFAULT;
    // XXX goto out;
  }
#endif // FLX_CRC8_WORKS

  /* Product name: ASCII for 12 characters */
  fbw_strcpy(eeprom->fbw_product_name,
             sizeof(eeprom->fbw_product_name),
             &cur, FBW_EEPROM_F_PRODUCT_NAME);

  syslog(LOG_NOTICE, "EEPROM product name : %s", eeprom->fbw_product_name);

  /* Product Part #: 12 byte data shown as PAXXXXXXX-000 */
  fbw_copy_product_number(eeprom->fbw_product_number,
                          sizeof(eeprom->fbw_product_number),
                          &cur, FBW_EEPROM_F_PRODUCT_NUMBER);

  /* System Assembly Part Number: XXX-XXXXXX-XX */
  fbw_copy_assembly_number(eeprom->fbw_assembly_number,
                           sizeof(eeprom->fbw_assembly_number),
                           &cur, FBW_EEPROM_F_ASSEMBLY_NUMBER);

  /* Partner PCBA Part Number: PAXX-XXXXX-XX */
  fbw_copy_facebook_pcba_part(eeprom->fbw_facebook_pcba_number,
                              sizeof(eeprom->fbw_facebook_pcba_number),
                              &cur, FBW_EEPROM_F_FACEBOOK_PCBA_NUMBER);

  /* Partner PCB Part Number: PBXXXXXXX-XXX */
  fbw_copy_facebook_pcb_part(eeprom->fbw_facebook_pcb_number,
                             sizeof(eeprom->fbw_facebook_pcb_number),
                             &cur, FBW_EEPROM_F_FACEBOOK_PCB_NUMBER);

  /* Flex PCBA Part Number: PAXX-XXXXX-XX */
  fbw_copy_facebook_pcba_part(eeprom->fbw_odm_pcba_number,
                              sizeof(eeprom->fbw_odm_pcba_number),
                              &cur, FBW_EEPROM_F_ODM_PCBA_NUMBER);

  /* Flex PCBA Serial Number: XXXXXXXXXXXX */
  fbw_strcpy(eeprom->fbw_odm_pcba_serial,
             sizeof(eeprom->fbw_odm_pcba_serial),
             &cur, FBW_EEPROM_F_ODM_PCBA_SERIAL);
  syslog(LOG_NOTICE, "PCBA Serial         : %s",
         eeprom->fbw_odm_pcba_serial);

  /* Product Production State */
  fbw_strcpy(eeprom->fbw_production_state, 
             sizeof(eeprom->fbw_production_state),
             &cur, FBW_EEPROM_F_PRODUCT_STATE);
//  eeprom->fbw_production_state = ntohs(eeprom->fbw_production_state);
  syslog(LOG_NOTICE, "Production State    : %s",
         eeprom->fbw_production_state);

  /* Product Version */
  fbw_strcpy(eeprom->fbw_product_version,
             sizeof(eeprom->fbw_product_version), 
             &cur, FBW_EEPROM_F_PRODUCT_VERSION);
  syslog(LOG_NOTICE, "Product Version     : %s",
         eeprom->fbw_product_version);

  /* Product Sub Version */
  fbw_strcpy(eeprom->fbw_product_subversion,
             sizeof(eeprom->fbw_product_subversion),
             &cur, FBW_EEPROM_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: XXXXXXXX */
  fbw_strcpy(eeprom->fbw_product_serial,
             sizeof(eeprom->fbw_product_serial),
             &cur, FBW_EEPROM_F_PRODUCT_SERIAL);
  syslog(LOG_NOTICE, "System Serial       : %s",
         eeprom->fbw_product_serial);

  /* Product Asset Tag: XXXXXXXX */
  fbw_strcpy(eeprom->fbw_product_asset,
             sizeof(eeprom->fbw_product_asset),
             &cur, FBW_EEPROM_F_PRODUCT_ASSET);

  /* System Manufacturer: XXXXXXXX */
  fbw_strcpy(eeprom->fbw_system_manufacturer,
             sizeof(eeprom->fbw_system_manufacturer),
             &cur, FBW_EEPROM_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yyyy */
  fbw_copy_date_v1(eeprom->fbw_system_manufacturing_date,
                   sizeof(eeprom->fbw_system_manufacturing_date),
                   &cur, FBW_EEPROM_F_SYSTEM_MANU_DATE);
  syslog(LOG_NOTICE, "EEPROM Mfg Date     : %s",
         eeprom->fbw_system_manufacturing_date);

  /* PCB Manufacturer: XXXXXXXXX */
  fbw_strcpy(eeprom->fbw_pcb_manufacturer,
             sizeof(eeprom->fbw_pcb_manufacturer),
             &cur, FBW_EEPROM_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  fbw_strcpy(eeprom->fbw_assembled,
             sizeof(eeprom->fbw_assembled),
             &cur, FBW_EEPROM_F_ASSEMBLED);

  /* Local MAC Address */
  fbw_copy_mac(eeprom->fbw_local_mac,
               sizeof(eeprom->fbw_local_mac),
               &cur, FBW_EEPROM_F_LOCAL_MAC);

  /* Extended MAC Address */
  fbw_copy_mac(eeprom->fbw_mac_base,
               sizeof(eeprom->fbw_mac_base),
               &cur, FBW_EEPROM_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  fbw_copy_uint16(&eeprom->fbw_mac_size, &cur,FBW_EEPROM_F_EXT_MAC_SIZE);
  eeprom->fbw_mac_size = ntohs(eeprom->fbw_mac_size);
  syslog(LOG_NOTICE, "Extended MAC sz     : %d", eeprom->fbw_mac_size);

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  fbw_strcpy(eeprom->fbw_location,
             sizeof(eeprom->fbw_location),
             &cur, FBW_EEPROM_F_LOCATION);

  /* CRC8 */
  fbw_copy_uint8(&eeprom->fbw_crc8,
                 &cur, FBW_EEPROM_F_CRC8);
  syslog(LOG_NOTICE, "EEPROM CRC          : 0x%x", eeprom->fbw_crc8);

  assert((cur - buf) <= len);

 out:
  return rc;
}

int wedge_eeprom_parse(const char *fn, struct wedge_eeprom_st *eeprom)
{
  int rc = 0;
  uint32_t len;
  FILE *fin;
  char buf[FBW_EEPROM_SIZE];

  if (!eeprom) {
    return -EINVAL;
  }

  if (!fn) {
    fn = FBW_EEPROM_FILE;
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
    syslog(LOG_WARNING, "Failed to complete the read. Only got %d bytes", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = fbw_parse_buffer((const uint8_t *)buf, sizeof(buf), eeprom);
  if (rc) {
    goto out;
  }

 out:
  if (fin) {
    fclose(fin);
  }

  return -rc;
}

void copy_data (char *src, char *dst, int bytes)
{
  if (*src == '\0')
    return;

  memcpy (dst, src, bytes);
}

void read_data (FILE *ftxt, char *str, int size)
{
  char line[128], *eol;
  memset(line, '\0', size);
  fgets(line, sizeof line, ftxt);
  if ((eol = strchr(line, '\n')))
    *eol = '\0';
  memcpy(str, line, size);
}

int fbw_fill_struct_from_file (FILE *ftxt, struct wedge_eeprom_gn *eeprom)
{
  char buffer[8];
  char buf2[16];

  read_data (ftxt, eeprom->fbw_version, sizeof eeprom->fbw_version);
//  syslog(LOG_NOTICE, "EEP ver from file  : %s", eeprom->fbw_version);

  read_data (ftxt, eeprom->fbw_product_name, sizeof eeprom->fbw_product_name);
  read_data (ftxt, eeprom->fbw_product_number, sizeof eeprom->fbw_product_number);
  read_data (ftxt, eeprom->fbw_assembly_number, sizeof eeprom->fbw_assembly_number);
  read_data (ftxt, eeprom->fbw_facebook_pcba_number, sizeof eeprom->fbw_facebook_pcba_number);
  read_data (ftxt, eeprom->fbw_facebook_pcb_number, sizeof eeprom->fbw_facebook_pcb_number);
  read_data (ftxt, eeprom->fbw_odm_pcba_number, sizeof eeprom->fbw_odm_pcba_number);
  read_data (ftxt, eeprom->fbw_odm_pcba_serial, sizeof eeprom->fbw_odm_pcba_serial);
//  syslog(LOG_NOTICE, "odm_pcba_serial from file  : %s", eeprom->fbw_odm_pcba_serial);
  read_data (ftxt, eeprom->fbw_production_state, sizeof eeprom->fbw_production_state);
//  syslog(LOG_NOTICE, "Prod State from file  : %s",
//         eeprom->fbw_production_state);

  read_data (ftxt, eeprom->fbw_product_version, sizeof eeprom->fbw_product_version);
  read_data (ftxt, eeprom->fbw_product_subversion, sizeof eeprom->fbw_product_subversion);

  read_data (ftxt, eeprom->fbw_product_serial, sizeof eeprom->fbw_product_serial);
//  syslog(LOG_NOTICE, "product_serial from file  : %s", eeprom->fbw_product_serial);

  read_data (ftxt, eeprom->fbw_product_asset, sizeof eeprom->fbw_product_asset);
  read_data (ftxt, eeprom->fbw_system_manufacturer, sizeof eeprom->fbw_system_manufacturer);
  read_data (ftxt, eeprom->fbw_system_manufacturing_date, sizeof eeprom->fbw_system_manufacturing_date);
//  syslog(LOG_NOTICE, "Mfg Date from file    : %s", eeprom->fbw_system_manufacturing_date);

  read_data (ftxt, eeprom->fbw_pcb_manufacturer, sizeof eeprom->fbw_pcb_manufacturer);
  read_data (ftxt, eeprom->fbw_assembled, sizeof eeprom->fbw_assembled);
  read_data (ftxt, eeprom->fbw_local_mac, sizeof eeprom->fbw_local_mac);
  read_data (ftxt, eeprom->fbw_mac_base, sizeof eeprom->fbw_mac_base);

  read_data (ftxt, buf2, sizeof buf2);
  eeprom->fbw_mac_size = atoi(buf2);
//  syslog(LOG_NOTICE, "MAC sz from file      : %d", eeprom->fbw_mac_size);
  eeprom->fbw_mac_size = htons(eeprom->fbw_mac_size);

  read_data (ftxt, eeprom->fbw_location, sizeof eeprom->fbw_location);
//  syslog(LOG_NOTICE, "Location from file    : %s", eeprom->fbw_location);
  read_data (ftxt, buffer, sizeof buffer);
  eeprom->fbw_crc8 = (uint8_t)atoi(buffer);

  return 0;
}
