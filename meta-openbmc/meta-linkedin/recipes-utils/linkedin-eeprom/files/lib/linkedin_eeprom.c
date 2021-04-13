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

#include "linkedin_eeprom.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>

#include <openbmc/log.h>

/*
 * calculate CRC8 checksum:
 *    x^8 + x^2 + x + 1
 */
#define DI  0x07
static unsigned char crc8_table[256];     /* 8-bit table */
static int made_table=0;

static void init_crc8()
{
    /*
     * Should be called before any other crc function.
     */
    int i,j;
    unsigned char crc;

    if (!made_table) {
        for (i=0; i<256; i++) {
            crc = i;
            for (j=0; j<8; j++)
                crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
            crc8_table[i] = crc & 0xFF;
        }
        made_table=1;
    }
}

unsigned char li_crc8(unsigned char* buf, int length)
{
    unsigned char i, j, crc=0;

    if (!made_table)
        init_crc8();

    for (i = 0; i < length; i++) {
        crc = crc8_table[(crc) ^ buf[i]];
        crc &= 0xFF;
    }
    return crc;
}

static inline void li_copy_uint8(uint8_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = **src;
  (*src) += src_len;
}

static inline void li_copy_uint16(uint16_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src) | ((*(*src + 1)) << 8);
  (*src) += src_len;
}

static inline void li_copy_uint32(uint32_t *val, const uint8_t** src,
                                  int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src)
    | ((*(*src + 1)) << 8)
    | ((*(*src + 2)) << 16)
    | ((*(*src + 3)) << 24);
  (*src) += src_len;
}

static inline void li_strcpy(char *dst, int dst_len,
                              const uint8_t **src, int src_len)
{
  assert(dst_len >= src_len + 1);    /* larger because of '\0' */
  strncpy(dst, (char *)*src, src_len);
  dst[src_len + 1] = '\0';
  (*src) += src_len;
}

static inline void li_copy_product_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 16 letter in the format of XXXXXXXXX-XXXXXXX, 2 additional letters */
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

static inline void li_copy_assembly_number(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 11 letter in the format of XXX-XXXXXX-XX, 3 additional letters */
  assert(dst_len >= src_len + 3);
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 6; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_pcb_part(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;
  /* 12 letter in the format of XXX-XXXXXX-XX, 3 additional letters */
  assert(dst_len >= src_len + 3);
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 6; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 3; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
  (*src) += src_len;
}

static inline void li_copy_date(
    char *dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;

  /* input is 8 bytes MMDDYYYY */
  assert(dst_len >= src_len + 3);
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 2; i++) {
    *dst++ = *cur++;
  }
  *dst++ = '-';
  for (i = 0; i < 4; i++) {
    *dst++ = *cur++;
  }
  *dst = '\0';
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

static inline void li_copy_mac(
    uint8_t* dst, int dst_len, const uint8_t **src, int src_len)
{
  int i;
  const uint8_t *cur = *src;

  assert(dst_len >= 6);
  assert(src_len >= 12);

  for (i = 0; i < 6; i++) {
    *dst = (_a2v(cur) << 4) | _a2v(cur + 1);
    dst++;
    cur +=2;
  }
  (*src) += src_len;
}

static int li_parse_buffer(
    const uint8_t *buf, int len, struct li_eeprom_st *eeprom) {
  int rc = 0;
  const uint8_t* cur = buf;
  uint32_t magic;
  int crc_len;
  uint8_t mac_offset;
  uint8_t crc8;
  uint8_t version;
  char version_str[LI_EEPROM_F_VERSION + 1];

  memset(eeprom, 0, sizeof(*eeprom));

  /* make sure the magic number */
  li_copy_uint32(&magic, &cur, LI_EEPROM_F_MAGIC);
  if (magic != LI_EEPROM_MAGIC_WORD) {
    rc = EFAULT;
    LOG_ERR(rc, "Unexpected magic word 0x%x P1 HW", magic);
    /*
     * linkedin V0 for P1 HW
     */
    version = LI_EEPROM_VERSION0;
    syslog(LOG_WARNING,
           "WARNING: EEPROM needs to be updated to production version 2!\n");
    printf("WARNING: BMC EEPROM needs to be updated to production version 2!\n");

    /*
     * Need to upgrade, only to get the local and extended MAC address
     */
    mac_offset = LI_LOCAL_MAC_OFFSET_V0;
    cur = buf+LI_LOCAL_MAC_OFFSET_V0;
    li_copy_mac(eeprom->li_local_mac,
                sizeof(eeprom->li_local_mac),
                &cur, LI_EEPROM_F_LOCAL_MAC);

    cur = buf+LI_EXT_MAC_OFFSET_V0;
    li_copy_mac(eeprom->li_mac_base,
                sizeof(eeprom->li_mac_base),
                &cur, LI_EEPROM_F_EXT_MAC_BASE);
    goto out;
  }

  /* Got LinkedIn Magic word, V1 or V2 eeprom */
  li_strcpy(eeprom->li_version, sizeof(eeprom->li_version), &cur, LI_EEPROM_F_VERSION);
  syslog(LOG_NOTICE, "EEPROM version      : %s", eeprom->li_version);
  if (strcmp(eeprom->li_version, LI_EEPROM_VERSION2_STR)) {
      syslog(LOG_WARNING,
             "WARNING: EEPROM version: %s,  needs to be updated to production version 2!\n", version_str);
      rc = EFAULT;
      goto out;
   }
   else
      crc_len = LI_EEPROM_V2_SIZE;

  assert(crc_len <= len);

  /* check CRC */
  crc8 = li_crc8(buf+LI_EEPROM_F_MAGIC, crc_len);
  if (crc8 != LI_EEPROM_CRC8_CSUM) {
    syslog(LOG_WARNING, "CRC check failed.  Expected 0x%x, got 0x%x", LI_EEPROM_CRC8_CSUM, crc8);
  }

  /* Product name: ASCII for 16 characters */
  li_strcpy(eeprom->li_product_name,
             sizeof(eeprom->li_product_name),
             &cur, LI_EEPROM_F_PRODUCT_NAME);

  /* Product Part #: 16 charactores */
  li_strcpy(eeprom->li_product_number,
             sizeof(eeprom->li_product_number),
             &cur, LI_EEPROM_F_PRODUCT_NUMBER);

  /* System Assembly Part Number: 12 charactores */
  li_strcpy(eeprom->li_assembly_number,
             sizeof(eeprom->li_assembly_number),
             &cur, LI_EEPROM_F_ASSEMBLY_NUMBER);

  /* Linkedin PCBA Part Number: 14 charactores */
  li_strcpy(eeprom->li_pcba_number,
             sizeof(eeprom->li_pcba_number),
             &cur, LI_EEPROM_F_PCBA_NUMBER);

  /* LinkedIn PCB Part Number: 14 charactores */
  li_strcpy(eeprom->li_pcb_number,
             sizeof(eeprom->li_pcb_number),
             &cur, LI_EEPROM_F_PCB_NUMBER);

  /* ODM PCBA Part Number: 14 charactores */
  li_strcpy(eeprom->li_odm_pcba_number,
             sizeof(eeprom->li_odm_pcba_number),
             &cur, LI_EEPROM_F_ODM_PCBA_NUMBER);

  /* ODM PCBA Serial Number: 12 charactores */
  li_strcpy(eeprom->li_odm_pcba_serial,
             sizeof(eeprom->li_odm_pcba_serial),
             &cur,
             LI_EEPROM_F_ODM_PCBA_SERIAL);

  /* Product Production State */
  li_strcpy(eeprom->li_production_state,
             sizeof(eeprom->li_production_state),
             &cur, LI_EEPROM_F_PRODUCT_STATE);

  li_strcpy(eeprom->li_product_version,
            sizeof(eeprom->li_product_version),
            &cur, LI_EEPROM_F_PRODUCT_VERSION);

  /* Product Sub Version */
  li_strcpy(eeprom->li_product_subversion,
            sizeof(eeprom->li_product_subversion),
            &cur, LI_EEPROM_F_PRODUCT_SUBVERSION);

  /* Product Serial Number: 16 characters */
  li_strcpy(eeprom->li_product_serial,
             sizeof(eeprom->li_product_serial),
             &cur,
             LI_EEPROM_F_PRODUCT_SERIAL);

  /* Product Assert Tag: XXXXXXXX */
  li_strcpy(eeprom->li_product_asset,
             sizeof(eeprom->li_product_asset),
             &cur, LI_EEPROM_F_PRODUCT_ASSET);

  /* System Manufacturer: XXXXXXXX */
  li_strcpy(eeprom->li_system_manufacturer,
             sizeof(eeprom->li_system_manufacturer),
             &cur, LI_EEPROM_F_SYSTEM_MANUFACTURER);

  /* System Manufacturing Date: mm-dd-yyyy */
  li_copy_date(eeprom->li_system_manufacturing_date,
                sizeof(eeprom->li_system_manufacturing_date),
                &cur, LI_EEPROM_F_SYSTEM_MANU_DATE);

  /* PCB Manufacturer: XXXXXXXXX */
  li_strcpy(eeprom->li_pcb_manufacturer,
             sizeof(eeprom->li_pcb_manufacturer),
             &cur, LI_EEPROM_F_PCB_MANUFACTURER);

  /* Assembled At: XXXXXXXX */
  li_strcpy(eeprom->li_assembled,
             sizeof(eeprom->li_assembled),
             &cur, LI_EEPROM_F_ASSEMBLED);

  /* Local MAC Address */
  li_copy_mac(eeprom->li_local_mac,
               sizeof(eeprom->li_local_mac),
               &cur, LI_EEPROM_F_LOCAL_MAC);

  /* Extended MAC Address */
  li_copy_mac(eeprom->li_mac_base,
               sizeof(eeprom->li_mac_base),
               &cur, LI_EEPROM_F_EXT_MAC_BASE);

  /* Extended MAC Address Size */
  li_copy_uint16(&eeprom->li_mac_size,
                 &cur,LI_EEPROM_F_EXT_MAC_SIZE);
  eeprom->li_mac_size = ntohs(eeprom->li_mac_size);

  /* Reserved field */
  li_strcpy(eeprom->li_reserved, sizeof(eeprom->li_reserved),
             &cur,LI_EEPROM_F_RESERVED);

  /* CRC8 */
  li_copy_uint8(&eeprom->li_crc8,
                 &cur, LI_EEPROM_F_CRC8);

  syslog(LOG_NOTICE, "EEPROM CRC          : 0x%x", eeprom->li_crc8);
  assert((cur - buf) <= len);

 out:
  return rc;
}

int li_eeprom_parse(const char *fn, struct li_eeprom_st *eeprom)
{
  int rc = 0;
  uint32_t len;
  FILE *fin, *fout;
  char buf[LI_EEPROM_SIZE];
  int write_eeprom = 1;

  if (!eeprom) {
    return -EINVAL;
  }

  if (!fn) {
    fn = LI_EEPROM_FILE;

    /*
     * Read the eeprom only
     */
    write_eeprom = 0;
  }

  fin = fopen(fn, "r");
  if (fin == NULL) {
    rc = errno;
    LOG_ERR(rc, "Failed to open %s", LI_EEPROM_FILE);
    goto out;
  }

  /* check the file size */
  rc = fseek(fin, 0, SEEK_END);
  if (rc) {
    rc = errno;
    LOG_ERR(rc, "Failed to seek to the end of %s", LI_EEPROM_FILE);
    goto out;
  }

  len = ftell(fin);
  if (len < LI_EEPROM_SIZE) {
    rc = ENOSPC;
    LOG_ERR(rc, "File '%s' is too small (%u < %u)", LI_EEPROM_FILE,
            len, LI_EEPROM_SIZE);
    goto out;
  }

  /* go back to the beginning of the file */
  rewind(fin);

  rc = fread(buf, 1, sizeof(buf), fin);
  if (rc < sizeof(buf)) {
    LOG_ERR(ENOSPC, "Failed to complete the read. Only got %d", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = li_parse_buffer((const uint8_t *)buf, sizeof(buf), eeprom);
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

int li_fill_struct_from_file (FILE *ftxt, struct li_eeprom_gn *eeprom)
{
  char buffer[8];
  char buf2[16];

  if (!eeprom)
      return -1;

  read_data (ftxt, eeprom->li_version, sizeof eeprom->li_version);
  syslog(LOG_NOTICE, "EEP ver from file  : %s", eeprom->li_version);

  read_data (ftxt, eeprom->li_product_name, sizeof eeprom->li_product_name);
  read_data (ftxt, eeprom->li_product_number, sizeof eeprom->li_product_number);

  read_data (ftxt, eeprom->li_assembly_number, sizeof eeprom->li_assembly_number);
  read_data (ftxt, eeprom->li_pcba_number, sizeof eeprom->li_pcba_number);
  read_data (ftxt, eeprom->li_pcb_number, sizeof eeprom->li_pcb_number);
  read_data (ftxt, eeprom->li_odm_pcba_number, sizeof eeprom->li_odm_pcba_number);
  read_data (ftxt, eeprom->li_odm_pcba_serial, sizeof eeprom->li_odm_pcba_serial);
  syslog(LOG_NOTICE, "odm_pcba_serial from file  : %s", eeprom->li_odm_pcba_serial);
  read_data (ftxt, eeprom->li_production_state, sizeof eeprom->li_production_state);
  syslog(LOG_NOTICE, "Prod State from file  : %s",
         eeprom->li_production_state);

  read_data (ftxt, eeprom->li_product_version, sizeof eeprom->li_product_version);
  read_data (ftxt, eeprom->li_product_subversion, sizeof eeprom->li_product_subversion);

  read_data (ftxt, eeprom->li_product_serial, sizeof eeprom->li_product_serial);
  syslog(LOG_NOTICE, "product_serial from file  : %s", eeprom->li_product_serial);

  read_data (ftxt, eeprom->li_product_asset, sizeof eeprom->li_product_asset);
  read_data (ftxt, eeprom->li_system_manufacturer, sizeof eeprom->li_system_manufacturer);
  read_data (ftxt, eeprom->li_system_manufacturing_date, sizeof eeprom->li_system_manufacturing_date);
  syslog(LOG_NOTICE, "Mfg Date from file    : %s", eeprom->li_system_manufacturing_date);

  read_data (ftxt, eeprom->li_pcb_manufacturer, sizeof eeprom->li_pcb_manufacturer);
  read_data (ftxt, eeprom->li_assembled, sizeof eeprom->li_assembled);
  read_data (ftxt, eeprom->li_local_mac, sizeof eeprom->li_local_mac);

  read_data (ftxt, eeprom->li_mac_base, sizeof eeprom->li_mac_base);

  read_data (ftxt, buf2, sizeof buf2);
  eeprom->li_mac_size = atoi(buf2);
  syslog(LOG_NOTICE, "MAC sz from file      : %d", eeprom->li_mac_size);
  eeprom->li_mac_size = htons(eeprom->li_mac_size);

  read_data (ftxt, eeprom->li_reserved, sizeof eeprom->li_reserved);
  read_data (ftxt, buffer, sizeof buffer);
  eeprom->li_crc8 = (uint8_t)atoi(buffer);

  return 0;
}
