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
#include <endian.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define ONIE_EEPROM_FILE "/sys/class/i2c-adapter/i2c-4/4-0050/eeprom"
#define ONIE_EEPROM_TXT  "/usr/bin/onie-eeprom.example"
#define ONIE_EEPROM_TLV_HDR_IDSTR	"TlvInfo"
#define ONIE_EEPROM_TLV_HDR_IDSTR_SZ 	8
#define ONIE_TLV_DATE_TIMESTAMP_DFLT	"14:30:01"
#define ONIE_TLV_TOTAL_LEN_WO_HDR	137
#define ONIE_EEPROM_TLV_HDR_SZ		11

#define ONIE_TLV_0x21_PRODUCT_NAME	0x21
#define ONIE_TLV_0x22_PART_NUMBER	0x22
#define ONIE_TLV_0x23_SERIAL_NUMBER	0x23
#define ONIE_TLV_0x25_MFG_DATE  	0x25
#define ONIE_TLV_0x26_DEVICE_VERSION	0x26
#define ONIE_TLV_0x28_PLATFORM_NAME	0x28
#define ONIE_TLV_0x29_LOADER_VERS	0x29
#define ONIE_TLV_0x2A_MAC_ADDRS 	0x2A
#define ONIE_TLV_0x2B_MANUFACTURER	0x2B
#define ONIE_TLV_0x2C_COUNTRY_CODE	0x2C
#define ONIE_TLV_0x2D_VENDOR_NAME	0x2D
#define ONIE_TLV_0x2E_DIAG_VERSION	0x2E
#define ONIE_TLV_0x2F_SERVICE_TAG	0x2F
#define ONIE_TLV_0xFD_VENDOR_EXT	0xFD
#define ONIE_TLV_0xFE_CRC32		0xFE

#define ONIE_TLV_0x21_PROD_NAME_LEN	4
#define ONIE_TLV_0x22_PART_NUM_LEN	13
#define ONIE_TLV_0x23_SERIAL_NUM_LEN    28 
#define ONIE_TLV_0x25_MFG_DATE_LEN	19
#define ONIE_TLV_0x26_DEVICE_VER_LEN	1
#define ONIE_TLV_0x28_PLAT_NAME_LEN	21
#define ONIE_TLV_0x29_LOADER_VER_LEN	7
#define ONIE_TLV_0x2A_MAC_ADDRS_LEN	2
#define ONIE_TLV_0x2B_MANUFACTURER_LEN	10
#define ONIE_TLV_0x2C_COUNTRY_CODE_LEN	2
#define ONIE_TLV_0x2D_VENDOR_NAME_LEN	8
#define ONIE_TLV_0x2E_DIAG_VERSION_LEN	1
#define ONIE_TLV_0x2F_SERVICE_TAG_LEN	1
#define ONIE_TLV_0xFD_VENDOR_EXT_LEN	5
#define ONIE_TLV_0xFE_CRC32_LEN         4


FILE *f_onie_eep, *ftxt;

typedef enum _eeprom_op_ {
   EEP_OPER_PROGRAM,
   EEP_OPER_DISPLAY,
   EEP_OPER_ERASE,
   EEP_OPER_INVALID,
} eeprom_op_t;

typedef struct __attribute__((__packed__)) _onie_tlv_hdr {
  char     idstr[ONIE_EEPROM_TLV_HDR_IDSTR_SZ];
  uint8_t  version;
  uint16_t total_len;
} onie_tlv_hdr;

typedef struct __attribute__((__packed__)) _onie_tlv_b {
  uint8_t type;
  uint8_t length;
  uint8_t val;
} onie_tlv_b;

typedef struct __attribute__((__packed__)) _onie_tlv_s {
  uint8_t type;
  uint8_t length;
  uint16_t val;
} onie_tlv_s;

typedef struct __attribute__((__packed__)) _onie_tlv_w {
  uint8_t type;
  uint8_t length;
  uint32_t val;
} onie_tlv_w;

typedef struct __attribute__((__packed__)) _onie_type_len {
  uint8_t type;
  uint8_t length;
} onie_tl;

struct __attribute__((__packed__)) bolt_onie_eeprom_gn {
  onie_tlv_hdr hdr;
  onie_tl      product_name;          /* 0x21 */
  char         str21[ONIE_TLV_0x21_PROD_NAME_LEN];

  onie_tl      part_number;           /* 0x22 */
  char         str22[ONIE_TLV_0x22_PART_NUM_LEN];

  onie_tl      serial_number;         /* 0x23 */
  char         str23[ONIE_TLV_0x23_SERIAL_NUM_LEN];

  onie_tl      mfg_date;              /* 0x25 */
  char         str25[ONIE_TLV_0x25_MFG_DATE_LEN];

  onie_tlv_b   device_version;        /* 0x26 */
  onie_tl      platform_name;         /* 0x28 */
  char         str28[ONIE_TLV_0x28_PLAT_NAME_LEN];

  onie_tl      loader_version;        /* 0x29 */
  char         str29[ONIE_TLV_0x29_LOADER_VER_LEN];

  onie_tlv_s   mac_addrs;             /* 0x2A */
  onie_tl      manufacturer;          /* 0x2B */
  char         str2B[ONIE_TLV_0x2B_MANUFACTURER_LEN];

  onie_tl      country_code;          /* 0x2C */
  char         str2C[ONIE_TLV_0x2C_COUNTRY_CODE_LEN];

  onie_tl      vendor_name;           /* 0x2D */
  char         str2D[ONIE_TLV_0x2D_VENDOR_NAME_LEN];

  onie_tl      diag_version;          /* 0x2E */
  char         str2E[ONIE_TLV_0x2E_DIAG_VERSION_LEN];

  onie_tl      service_tag;           /* 0x2F */
  char         str2F[ONIE_TLV_0x2F_SERVICE_TAG_LEN];

  onie_tl      vendor_ext;            /* 0xFD */
  char         strFD[ONIE_TLV_0xFD_VENDOR_EXT_LEN];

  onie_tlv_w   crc32;                 /* 0xFE */
};

static uint32_t
flx_crc32(char *buf, size_t len)
{
  uint32_t crc = 0;
  static uint32_t table[256];
  static int have_table = 0;
  uint32_t rem;
  uint8_t octet;
  int i, j;
  const char *p, *q;

  /* This check is not thread safe; there is no mutex. */
  if (have_table == 0) {
    /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else
                    rem >>= 1;
                }
                table[i] = rem;
      }
      have_table = 1;
  }

  crc = ~crc;
  q = buf + len;
  for (p = buf; p < q; p++) {
    octet = *p;  /* Cast to unsigned octet. */
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }
  return ~crc;
}

static uint16_t
flx_swap16(uint16_t val)
{
  return ((val & 0xFF) << 8) |\
          ((val >> 8) & 0xFF);
}

static inline void
flx_get_raw(char *dst, const uint8_t **src, int len)
{
  memcpy(dst, *src, len);
  (*src) += len;
}

static inline void
flx_get_str(char *dst, int dst_len,
            const uint8_t **src, int src_len)
{
  assert(dst_len >= src_len + 1);    /* larger because of '\0' */

  strncpy(dst, (char *)*src, src_len);
  dst[src_len + 1] = '\0';
  (*src) += src_len;
}

static inline void
flx_get_uint8(uint8_t *val, const uint8_t** src,
              int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = **src;
  (*src) += src_len;
}

static inline void
flx_get_uint16(uint16_t *val, const uint8_t** src,
               int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src) | ((*(*src + 1)) << 8);
  (*src) += src_len;
}

static inline void
flx_get_uint32(uint32_t *val, const uint8_t** src,
               int src_len)
{
  assert(src_len >= sizeof(*val));
  *val = (**src)
    | ((*(*src + 1)) << 8)
    | ((*(*src + 2)) << 16)
    | ((*(*src + 3)) << 24);
  (*src) += src_len;
}

static void
read_tlv_hdr(onie_tlv_hdr *hdr)
{
  char line[64], *eol;
  char *val;
  uint16_t short_val;

  memset(hdr, 0, sizeof (onie_tlv_hdr));
  memset(line, '\0', ONIE_EEPROM_TLV_HDR_IDSTR_SZ);

  fgets(line, sizeof line, ftxt);
  if ((eol = strchr(line, '\n')))
    *eol = '\0';

  val = strtok(line, ",");
  memcpy(&hdr->idstr, line, ONIE_EEPROM_TLV_HDR_IDSTR_SZ);

  hdr->version = atoi(strtok(NULL, ","));
  hdr->total_len = atoi(strtok(NULL, ","));
}

static void
read_tlv_short(onie_tlv_s *tlv)
{
  char line[128], *eol;
  char *val;

  memset(tlv, 0, sizeof (onie_tlv_s));
  memset(line, '\0', sizeof(line));

  fgets(line, sizeof line, ftxt);
  if ((eol = strchr(line, '\n')))
    *eol = '\0';

//  syslog(LOG_NOTICE, "%s: Got line            : %s", __FUNCTION__, line);

  tlv->type = strtol(strtok(line, ","), NULL, 16);
//  syslog(LOG_NOTICE, "%s: Copied type         : %d", __FUNCTION__, tlv->type);

  tlv->length = atoi(strtok(NULL, ","));
//  syslog(LOG_NOTICE, "%s: Copied length: %d", __FUNCTION__, tlv->length);

  tlv->val = atoi(strtok(NULL, ","));
//    syslog(LOG_NOTICE, "%s: Copied val        : %d", __FUNCTION__, tlv->val);
}

static void
read_tlv_byte(onie_tlv_b *tlv)
{
  char line[128], *eol;
  char *val;

  memset(tlv, 0, sizeof (onie_tlv_b));
  memset(line, '\0', sizeof(line));

  fgets(line, sizeof line, ftxt);
  if ((eol = strchr(line, '\n')))
    *eol = '\0';

//  syslog(LOG_NOTICE, "%s: Got line            : %s", __FUNCTION__, line);

  tlv->type = strtol(strtok(line, ","), NULL, 16);
//  syslog(LOG_NOTICE, "%s: Copied type         : %d", __FUNCTION__, tlv->type);

  tlv->length = atoi(strtok(NULL, ","));
//  syslog(LOG_NOTICE, "%s: Copied length: %d", __FUNCTION__, tlv->length);

  tlv->val = atoi(strtok(NULL, ","));
//    syslog(LOG_NOTICE, "%s: Copied val        : %d", __FUNCTION__, tlv->val);
}

static void
read_tlv_str(onie_tl *tl, char *p, int len)
{
  char line[128], *eol;
  char *val;

  memset(tl, 0, sizeof (onie_tl));
  memset(line, '\0', len);

  fgets(line, sizeof line, ftxt);
  if ((eol = strchr(line, '\n')))
    *eol = '\0';

//  syslog(LOG_NOTICE, "%s: Got line            : %s", __FUNCTION__, line);

  tl->type = strtol(strtok(line, ","), NULL, 16);
//  syslog(LOG_NOTICE, "%s: Copied type         : %d", __FUNCTION__, tl->type);

  tl->length = atoi(strtok(NULL, ","));
//  syslog(LOG_NOTICE, "%s: Copied length: %d", __FUNCTION__, tl->length);

  val = strtok(NULL, ",");
//    syslog(LOG_NOTICE, "%s: Got val           : %s", __FUNCTION__, val);
  memcpy(p, val, len);
//    syslog(LOG_NOTICE, "%s: Copied val        : %s", __FUNCTION__, p);
}

static int
init_eeprom_data (struct bolt_onie_eeprom_gn *eeprom)
{
  read_tlv_hdr(&eeprom->hdr);

  syslog(LOG_NOTICE, "ONIE hdr ID addr        : %p", &eeprom->hdr);
  syslog(LOG_NOTICE, "ONIE hdr ID str         : %s", eeprom->hdr.idstr);
  syslog(LOG_NOTICE, "ONIE hdr version        : %d", eeprom->hdr.version);
  syslog(LOG_NOTICE, "ONIE hdr total len      : %d", eeprom->hdr.total_len);
  /* Store in Big Endian */
  eeprom->hdr.total_len = htons(eeprom->hdr.total_len);

  read_tlv_str(&eeprom->product_name,
               eeprom->str21,
               ONIE_TLV_0x21_PROD_NAME_LEN);
  syslog(LOG_NOTICE, "Product name addr       : %p", &eeprom->product_name);
  syslog(LOG_NOTICE, "Product name type       : 0x%x",
         eeprom->product_name.type);
  syslog(LOG_NOTICE, "Product name len        : %d",
         eeprom->product_name.length);
  syslog(LOG_NOTICE, "Product name value      : %s",
         eeprom->str21);

  read_tlv_str(&eeprom->part_number,
               eeprom->str22,
               ONIE_TLV_0x22_PART_NUM_LEN);
  syslog(LOG_NOTICE, "Part Number addr        : %p", &eeprom->part_number);
  syslog(LOG_NOTICE, "Part Number type        : 0x%x",
         eeprom->part_number.type);
  syslog(LOG_NOTICE, "Part Number length      : %d",
         eeprom->part_number.length);
  syslog(LOG_NOTICE, "Part Number value       : %s",
         eeprom->str22);

  read_tlv_str(&eeprom->serial_number,
               eeprom->str23,
               ONIE_TLV_0x23_SERIAL_NUM_LEN);
  syslog(LOG_NOTICE, "Serial Number addr      : %p", &eeprom->serial_number);
  syslog(LOG_NOTICE, "Serial Number type      : 0x%x",
         eeprom->serial_number.type);
  syslog(LOG_NOTICE, "Serial Number length    : %d",
         eeprom->serial_number.length);
  syslog(LOG_NOTICE, "Serial Number value     : %s",
         eeprom->str23);

  read_tlv_str(&eeprom->mfg_date,
               eeprom->str25,
               ONIE_TLV_0x25_MFG_DATE_LEN);
  syslog(LOG_NOTICE, "Mfg Date addr           : %p", &eeprom->mfg_date);
  syslog(LOG_NOTICE, "Mfg Date type           : 0x%x",
         eeprom->mfg_date.type);
  syslog(LOG_NOTICE, "Mfg Date length         : %d",
         eeprom->mfg_date.length);
  syslog(LOG_NOTICE, "Mfg Date value          : %s",
         eeprom->str25);

  read_tlv_byte(&eeprom->device_version);
  syslog(LOG_NOTICE, "Device Version addr     : %p", &eeprom->device_version);
  syslog(LOG_NOTICE, "Device Version type     : 0x%x",
         eeprom->device_version.type);
  syslog(LOG_NOTICE, "Device Version length   : %d",
         eeprom->device_version.length);
  syslog(LOG_NOTICE, "Device Version value    : %d",
         eeprom->device_version.val);

  read_tlv_str(&eeprom->platform_name,
               eeprom->str28,
               ONIE_TLV_0x28_PLAT_NAME_LEN);
  syslog(LOG_NOTICE, "Platform Name addr      : %p", &eeprom->platform_name);
  syslog(LOG_NOTICE, "Platform Name type      : 0x%x",
         eeprom->platform_name.type);
  syslog(LOG_NOTICE, "Platform Name length    : %d",
         eeprom->platform_name.length);
  syslog(LOG_NOTICE, "Platform Name val       : %s",
         eeprom->str28);

  read_tlv_str(&eeprom->loader_version,
               eeprom->str29,
               ONIE_TLV_0x29_LOADER_VER_LEN);
  syslog(LOG_NOTICE, "Loader Version addr     : %p", &eeprom->loader_version);
  syslog(LOG_NOTICE, "Loader Version type     : 0x%x",
         eeprom->loader_version.type);
  syslog(LOG_NOTICE, "Loader Version length   : %d",
         eeprom->loader_version.length);
  syslog(LOG_NOTICE, "Loader Version val      : %s",
         eeprom->str29);

  read_tlv_short(&eeprom->mac_addrs);
  syslog(LOG_NOTICE, "MAC Addresses addr      : %p", &eeprom->mac_addrs);
  syslog(LOG_NOTICE, "MAC Addresses type      : 0x%x",
         eeprom->mac_addrs.type);
  syslog(LOG_NOTICE, "MAC Addresses length    : %d",
         eeprom->mac_addrs.length);
  syslog(LOG_NOTICE, "MAC Addresses val       : %d",
         eeprom->mac_addrs.val);
  /* Store in Big Endian */
  eeprom->mac_addrs.val = htons(eeprom->mac_addrs.val);

  read_tlv_str(&eeprom->manufacturer,
               eeprom->str2B,
               ONIE_TLV_0x2B_MANUFACTURER_LEN);
  syslog(LOG_NOTICE, "Manufacturer addr       : %p", &eeprom->manufacturer);
  syslog(LOG_NOTICE, "Manufacturer type       : 0x%x",
         eeprom->manufacturer.type);
  syslog(LOG_NOTICE, "Manufacturer length     : %d",
         eeprom->manufacturer.length);
  syslog(LOG_NOTICE, "Manufacturer val        : %s",
         eeprom->str2B);

  read_tlv_str(&eeprom->country_code,
               eeprom->str2C,
               ONIE_TLV_0x2C_COUNTRY_CODE_LEN);
  syslog(LOG_NOTICE, "Country code addr       : %p", &eeprom->country_code);
  syslog(LOG_NOTICE, "Country code type       : 0x%x",
         eeprom->country_code.type);
  syslog(LOG_NOTICE, "Country code length     : %d",
         eeprom->country_code.length);
  syslog(LOG_NOTICE, "Country code val        : %s",
         eeprom->str2C);

  read_tlv_str(&eeprom->vendor_name,
               eeprom->str2D,
               ONIE_TLV_0x2D_VENDOR_NAME_LEN);
  syslog(LOG_NOTICE, "Vendor Name addr        : %p", &eeprom->vendor_name);
  syslog(LOG_NOTICE, "Vendor Name type        : 0x%x",
         eeprom->vendor_name.type);
  syslog(LOG_NOTICE, "Vendor Name length      : %d",
         eeprom->vendor_name.length);
  syslog(LOG_NOTICE, "Vendor Name val         : %s",
         eeprom->str2D);

  read_tlv_str(&eeprom->diag_version,
               eeprom->str2E,
               ONIE_TLV_0x2E_DIAG_VERSION_LEN);
  syslog(LOG_NOTICE, "Diag Version addr       : %p", &eeprom->diag_version);
  syslog(LOG_NOTICE, "Diag Version type       : 0x%x",
         eeprom->diag_version.type);
  syslog(LOG_NOTICE, "Diag Version length     : %d",
         eeprom->diag_version.length);
  syslog(LOG_NOTICE, "Diag Version val        : %s",
         eeprom->str2E);

  read_tlv_str(&eeprom->service_tag,
               eeprom->str2F,
               ONIE_TLV_0x2F_SERVICE_TAG_LEN);
  syslog(LOG_NOTICE, "Service Tag addr        : %p", &eeprom->service_tag);
  syslog(LOG_NOTICE, "Service Tag type        : 0x%x",
         eeprom->service_tag.type);
  syslog(LOG_NOTICE, "Service Tag length      : %d",
         eeprom->service_tag.length);
  syslog(LOG_NOTICE, "Service Tag val         : %s",
         eeprom->str2F);

  read_tlv_str(&eeprom->vendor_ext,
               eeprom->strFD,
               ONIE_TLV_0xFD_VENDOR_EXT_LEN);
  syslog(LOG_NOTICE, "Vendor Ext addr         : %p", &eeprom->vendor_ext);
  syslog(LOG_NOTICE, "Vendor Ext type         : 0x%x",
         eeprom->vendor_ext.type);
  syslog(LOG_NOTICE, "Vendor Ext length       : %d",
         eeprom->vendor_ext.length);
  syslog(LOG_NOTICE, "Vendor Ext val          : %s",
         eeprom->strFD);

  return 0;
}

static int
program_onie_eeprom(const char* onie_fn, bool overwrite)
{
  int rc = 0;
  struct bolt_onie_eeprom_gn eeprom;

  memset(&eeprom, 0, sizeof (struct bolt_onie_eeprom_gn));

  if (!onie_fn) {
    onie_fn = ONIE_EEPROM_TXT;
  }

  syslog(LOG_NOTICE, "Opening %s", onie_fn);
  ftxt = fopen(onie_fn, "r");
  if (ftxt == NULL) {
      rc = errno;
      fprintf(stderr, "Failed to open %s\n", onie_fn);
      goto out;
  }

  syslog(LOG_NOTICE, "Initializing eeprom data");
  rc = init_eeprom_data(&eeprom);
  if (rc) {
    goto out;
  } 
  syslog(LOG_NOTICE, "Done initializing eeprom data");

  if (overwrite) {
      syslog(LOG_NOTICE, "Now copy the data from ID eeprom");
    struct wedge_eeprom_st id_eeprom;

    /*
     * Read the Board ID EEPROM
     */ 
    rc = wedge_eeprom_parse(NULL, &id_eeprom);
    if (rc) {
      syslog(LOG_WARNING, "Failed to parse Board ID EEPROM");
      goto out;
    }
  
    /*
     * Copy Board specifics from ID EEPROM
     */
    eeprom.serial_number.type   = ONIE_TLV_0x23_SERIAL_NUMBER;
    eeprom.serial_number.length = ONIE_TLV_0x23_SERIAL_NUM_LEN;
    memcpy(eeprom.str23,
           id_eeprom.fbw_product_serial,
           ONIE_TLV_0x23_SERIAL_NUM_LEN);
    syslog(LOG_NOTICE, "Serial Number addr      : %p",
           &eeprom.serial_number);
      syslog(LOG_NOTICE, "Serial Number type      : 0x%x",
           eeprom.serial_number.type);
    syslog(LOG_NOTICE, "Serial Number length    : %d",
           eeprom.serial_number.length);
    syslog(LOG_NOTICE, "Serial Number val       : %s",
           eeprom.str23);

    eeprom.mfg_date.type   = ONIE_TLV_0x25_MFG_DATE;
    eeprom.mfg_date.length = ONIE_TLV_0x25_MFG_DATE_LEN;

    char tmp[ONIE_TLV_0x25_MFG_DATE_LEN+1];
    sprintf(tmp, "%s %s",
            id_eeprom.fbw_system_manufacturing_date,
            ONIE_TLV_DATE_TIMESTAMP_DFLT);
    memcpy(eeprom.str25,
           tmp,
           ONIE_TLV_0x25_MFG_DATE_LEN);
    syslog(LOG_NOTICE, "Mfg Date addr           : %p",
           &eeprom.mfg_date);
    syslog(LOG_NOTICE, "Mfg Date type           : 0x%x",
           eeprom.mfg_date.type);
    syslog(LOG_NOTICE, "Mfg Date length         : %d",
           eeprom.mfg_date.length);
    syslog(LOG_NOTICE, "Mfg Date val            : %s",
           eeprom.str25);
  }

  uint32_t crc32;
  ssize_t sz = sizeof (struct bolt_onie_eeprom_gn);
  size_t crc_len;
  /* calculate CRC32 based on all bytes but crc itself */
  /* validate on all bytes, resulting in crc value 0 */
  crc_len              = sz - ONIE_TLV_0xFE_CRC32_LEN;

  eeprom.crc32.type    = ONIE_TLV_0xFE_CRC32;
  eeprom.crc32.length  = ONIE_TLV_0xFE_CRC32_LEN;
  crc32                = flx_crc32((char*)&eeprom, crc_len);

  syslog(LOG_NOTICE, "CRC32 addr              : %p",
         &eeprom.crc32);
  syslog(LOG_NOTICE, "CRC32 type              : 0x%x",
         eeprom.crc32.type);
  syslog(LOG_NOTICE, "CRC32 length            : %d",
         eeprom.crc32.length);
  syslog(LOG_NOTICE, "CRC32 val               : 0x%x",
         crc32);
  syslog(LOG_NOTICE, "EEPROM sz to calc crc   : %d bytes",
         crc_len);

  eeprom.crc32.val = htonl(crc32);
  syslog(LOG_NOTICE, "CRC32 swapped           : 0x%x",
         eeprom.crc32.val);
  syslog(LOG_NOTICE, "bolt_onie_eeprom_gn sz  : %d bytes",
         sz);

  rc = fwrite(&eeprom, 1, sz, f_onie_eep);
  if (rc < sz) {
      fprintf(stderr,
             "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes\n",
             sz, rc);
      goto out;
  }
  syslog(LOG_NOTICE, "Wrote all %d bytes to EEPROM", rc); 
  rc = 0;

out:
  if (ftxt) {
    fclose(ftxt);
  }

  return rc;
}

static int
erase_onie_eeprom(void)
{
  int rc = 0;
  uint8_t erase_byte[1];
  uint8_t len;
  char tmp[ONIE_TLV_TOTAL_LEN_WO_HDR + ONIE_EEPROM_TLV_HDR_SZ];

  len = sizeof tmp;
  memset(&tmp, 0xFF, len);

  syslog(LOG_NOTICE, "Initializing eeprom data.  %d bytes with 0xFF",
         len);

  rc = fwrite(&tmp, 1, len, f_onie_eep);
  if (rc < len) {
      fprintf(stderr,
             "Failed to write entire EEPROM (%d bytes).  Wrote %d bytes\n",
             len, rc);
      goto out;
  }
  syslog(LOG_NOTICE, "Wrote all %d bytes to EEPROM", rc); 
  rc = 0;

out:
  return rc;
}

typedef struct _onie_tlv_hdr2{
  char     idstr[ONIE_EEPROM_TLV_HDR_IDSTR_SZ+1];
  uint8_t  version;
  uint16_t total_len;
} onie_tlv_hdr2;

typedef struct _onie_tlv_ext{
  uint8_t  type;
  uint8_t  length;
  uint8_t  val[ONIE_TLV_0xFD_VENDOR_EXT_LEN];
} onie_tlv_ext;

struct onie_display_eeprom {
  onie_tlv_hdr2 hdr;
  onie_tl      product_name;          /* 0x21 */
  char         str21[ONIE_TLV_0x21_PROD_NAME_LEN+1];

  onie_tl      part_number;           /* 0x22 */
  char         str22[ONIE_TLV_0x22_PART_NUM_LEN+1];

  onie_tl      serial_number;         /* 0x23 */
  char         str23[ONIE_TLV_0x23_SERIAL_NUM_LEN+1];

  onie_tl      mfg_date;              /* 0x25 */
  char         str25[ONIE_TLV_0x25_MFG_DATE_LEN+1];

  onie_tlv_b   device_version;        /* 0x26 */
  onie_tl      platform_name;         /* 0x28 */
  char         str28[ONIE_TLV_0x28_PLAT_NAME_LEN+1];

  onie_tl      loader_version;        /* 0x29 */
  char         str29[ONIE_TLV_0x29_LOADER_VER_LEN+1];

  onie_tlv_s   mac_addrs;             /* 0x2A */
  onie_tl      manufacturer;          /* 0x2B */
  char         str2B[ONIE_TLV_0x2B_MANUFACTURER_LEN+1];

  onie_tl      country_code;          /* 0x2C */
  char         str2C[ONIE_TLV_0x2C_COUNTRY_CODE_LEN+1];

  onie_tl      vendor_name;           /* 0x2D */
  char         str2D[ONIE_TLV_0x2D_VENDOR_NAME_LEN+1];

  onie_tl      diag_version;          /* 0x2E */
  char         str2E[ONIE_TLV_0x2E_DIAG_VERSION_LEN+1];

  onie_tl      service_tag;           /* 0x2F */
  char         str2F[ONIE_TLV_0x2F_SERVICE_TAG_LEN+1];

  onie_tlv_ext vendor_ext;            /* 0xFD */

  onie_tlv_w   crc32;                 /* 0xFE */
};

static int
parse_onie_buffer(
  const uint8_t *buf, int len, struct onie_display_eeprom *eep)
{
  int rc = 0;
  const uint8_t* cur = buf;
  int crc_len;
  uint32_t crc32;
  bool LAST_TLV = false;

  if (!eep) {
    return -EINVAL;
  }

  /* Get the header which has to be first */
  flx_get_str(eep->hdr.idstr,
              sizeof(eep->hdr.idstr),
              &cur, ONIE_EEPROM_TLV_HDR_IDSTR_SZ);

  if (strncmp(eep->hdr.idstr, ONIE_EEPROM_TLV_HDR_IDSTR, ONIE_EEPROM_TLV_HDR_IDSTR_SZ) != 0) {
    fprintf(stderr, "Invalid ONIE Header ID : %s\n",eep->hdr.idstr);
    fprintf(stderr, "Please reprogram the eeprom using the \"prog\" option\n");
    return -1;
  }

  flx_get_uint8(&eep->hdr.version, &cur, 1);

  flx_get_uint16(&eep->hdr.total_len, &cur, 2);
  eep->hdr.total_len = ntohs(eep->hdr.total_len);

  syslog(LOG_NOTICE, "ONIE hdr ID             : %s", eep->hdr.idstr);
  syslog(LOG_NOTICE, "ONIE hdr version        : %d", eep->hdr.version);
  syslog(LOG_NOTICE, "ONIE hdr total len      : %d", eep->hdr.total_len);

  while (!LAST_TLV) {
    onie_tl tl;
    uint32_t tmp32;
    uint16_t tmp16;
    uint8_t tmp8;

    flx_get_uint8(&tl.type, &cur, 1);
    flx_get_uint8(&tl.length, &cur, 1);

    switch (tl.type) {
      case ONIE_TLV_0x21_PRODUCT_NAME:
        eep->product_name.type   = tl.type;
        eep->product_name.length = tl.length;
        flx_get_str(eep->str21, sizeof(eep->str21),
                    &cur, ONIE_TLV_0x21_PROD_NAME_LEN);

        syslog(LOG_NOTICE, "Product Name type       : 0x%x",
               eep->product_name.type);
        syslog(LOG_NOTICE, "Product Name length     : %d",
               eep->product_name.length);
        syslog(LOG_NOTICE, "Product Name            : %s",
               eep->str21);
        break;

      case ONIE_TLV_0x22_PART_NUMBER:
        eep->part_number.type   = tl.type;
        eep->part_number.length = tl.length;
        flx_get_str(eep->str22, sizeof(eep->str22),
                    &cur, ONIE_TLV_0x22_PART_NUM_LEN);

        syslog(LOG_NOTICE, "Part Number type        : 0x%x",
               eep->part_number.type);
        syslog(LOG_NOTICE, "Part Number length      : %d",
               eep->part_number.length);
        syslog(LOG_NOTICE, "Part Number             : %s",
               eep->str22);
        break;

      case ONIE_TLV_0x23_SERIAL_NUMBER:
        eep->serial_number.type   = tl.type;
        eep->serial_number.length = tl.length;
        flx_get_str(eep->str23, sizeof(eep->str23),
                    &cur, ONIE_TLV_0x23_SERIAL_NUM_LEN);

        syslog(LOG_NOTICE, "Serial Number type      : 0x%x",
               eep->serial_number.type);
        syslog(LOG_NOTICE, "Serial Number length    : %d",
               eep->serial_number.length);
        syslog(LOG_NOTICE, "Serial Number           : %s",
               eep->str23);
        break;

      case ONIE_TLV_0x25_MFG_DATE:
        eep->mfg_date.type   = tl.type;
        eep->mfg_date.length = tl.length;
        flx_get_str(eep->str25, sizeof(eep->str25),
                    &cur, ONIE_TLV_0x25_MFG_DATE_LEN);

        syslog(LOG_NOTICE, "Mfg Date type           : 0x%x",
               eep->mfg_date.type);
        syslog(LOG_NOTICE, "Mfg Date length         : %d",
               eep->mfg_date.length);
        syslog(LOG_NOTICE, "Mfg Date                : %s",
               eep->str25);
        break;

      case ONIE_TLV_0x26_DEVICE_VERSION:
        flx_get_uint8(&tmp8, &cur, 1);
        eep->device_version.type   = tl.type;
        eep->device_version.length = tl.length;
        eep->device_version.val    = tmp8;

        syslog(LOG_NOTICE, "Device Version type     : 0x%x",
               eep->device_version.type);
        syslog(LOG_NOTICE, "Device Version length   : %d",
               eep->device_version.length);
        syslog(LOG_NOTICE, "Device Version          : %d",
               eep->device_version.val);
        break;

      case ONIE_TLV_0x28_PLATFORM_NAME:
        eep->platform_name.type   = tl.type;
        eep->platform_name.length = tl.length;
        flx_get_str(eep->str28, sizeof(eep->str28),
                    &cur, ONIE_TLV_0x28_PLAT_NAME_LEN);

        syslog(LOG_NOTICE, "Platform Name type      : 0x%x",
               eep->platform_name.type);
        syslog(LOG_NOTICE, "Platform Name length    : %d",
               eep->platform_name.length);
        syslog(LOG_NOTICE, "Platform Name           : %s",
               eep->str28);
        break;

      case ONIE_TLV_0x29_LOADER_VERS:
        eep->loader_version.type   = tl.type;
        eep->loader_version.length = tl.length;
        flx_get_str(eep->str29, sizeof(eep->str29),
                    &cur, ONIE_TLV_0x29_LOADER_VER_LEN);

        syslog(LOG_NOTICE, "Loader Version type     : 0x%x",
               eep->loader_version.type);
        syslog(LOG_NOTICE, "Loader Version length   : %d",
               eep->loader_version.length);
        syslog(LOG_NOTICE, "Loader Version          : %s",
               eep->str29);
        break;

      case ONIE_TLV_0x2A_MAC_ADDRS:
        flx_get_uint16(&tmp16, &cur, 2);
        eep->mac_addrs.type   = tl.type;
        eep->mac_addrs.length = tl.length;
        eep->mac_addrs.val    = ntohs(tmp16);

        syslog(LOG_NOTICE, "MAC Addresses type      : 0x%x",
               eep->mac_addrs.type);
        syslog(LOG_NOTICE, "MAC Addresses length    : %d",
               eep->mac_addrs.length);
        syslog(LOG_NOTICE, "MAC Addresses           : %d",
               eep->mac_addrs.val);
        break;

      case ONIE_TLV_0x2B_MANUFACTURER:
        eep->manufacturer.type   = tl.type;
        eep->manufacturer.length = tl.length;
        flx_get_str(eep->str2B, sizeof(eep->str2B),
                    &cur, ONIE_TLV_0x2B_MANUFACTURER_LEN);

        syslog(LOG_NOTICE, "Manufacturer type       : 0x%x",
               eep->manufacturer.type);
        syslog(LOG_NOTICE, "Manufacturer length     : %d",
               eep->manufacturer.length);
        syslog(LOG_NOTICE, "Manufacturer            : %s",
               eep->str2B);
        break;

      case ONIE_TLV_0x2C_COUNTRY_CODE:
        eep->country_code.type   = tl.type;
        eep->country_code.length = tl.length;
        flx_get_str(eep->str2C, sizeof(eep->str2C),
                    &cur, ONIE_TLV_0x2C_COUNTRY_CODE_LEN);

        syslog(LOG_NOTICE, "Country Code type       : 0x%x",
               eep->country_code.type);
        syslog(LOG_NOTICE, "Country Code length     : %d",
               eep->country_code.length);
        syslog(LOG_NOTICE, "Country Code            : %s",
               eep->str2C);
        break;

      case ONIE_TLV_0x2D_VENDOR_NAME:
        eep->vendor_name.type   = tl.type;
        eep->vendor_name.length = tl.length;
        flx_get_str(eep->str2D, sizeof(eep->str2D),
                    &cur, ONIE_TLV_0x2D_VENDOR_NAME_LEN);

        syslog(LOG_NOTICE, "Vendor Name type        : 0x%x",
               eep->vendor_name.type);
        syslog(LOG_NOTICE, "Vendor Name length      : %d",
               eep->vendor_name.length);
        syslog(LOG_NOTICE, "Vendor Name             : %s",
               eep->str2D);
        break;

      case ONIE_TLV_0x2E_DIAG_VERSION:
        eep->diag_version.type   = tl.type;
        eep->diag_version.length = tl.length;
        flx_get_str(eep->str2E, sizeof(eep->str2E),
                    &cur, ONIE_TLV_0x2E_DIAG_VERSION_LEN);

        syslog(LOG_NOTICE, "Diag Version type       : 0x%x",
               eep->diag_version.type);
        syslog(LOG_NOTICE, "Diag Version length     : %d",
               eep->diag_version.length);
        syslog(LOG_NOTICE, "Diag Version            : %s",
               eep->str2E);
        break;

      case ONIE_TLV_0x2F_SERVICE_TAG:
        eep->service_tag.type   = tl.type;
        eep->service_tag.length = tl.length;
        flx_get_str(eep->str2F, sizeof(eep->str2F),
                    &cur, ONIE_TLV_0x2F_SERVICE_TAG_LEN);

        syslog(LOG_NOTICE, "Service Tag type        : 0x%x",
               eep->service_tag.type);
        syslog(LOG_NOTICE, "Service Tag length      : %d",
               eep->service_tag.length);
        syslog(LOG_NOTICE, "Service Tag             : %s",
               eep->str2F);
        break;

      case ONIE_TLV_0xFD_VENDOR_EXT:
        flx_get_uint8(&eep->vendor_ext.val[0], &cur, 1);
        flx_get_uint8(&eep->vendor_ext.val[1], &cur, 1);
        flx_get_uint8(&eep->vendor_ext.val[2], &cur, 1);
        flx_get_uint8(&eep->vendor_ext.val[3], &cur, 1);
        flx_get_uint8(&eep->vendor_ext.val[4], &cur, 1);
        eep->vendor_ext.type   = tl.type;
        eep->vendor_ext.length = tl.length;

        syslog(LOG_NOTICE, "Vendor Ext type         : 0x%x",
               eep->vendor_ext.type);
        syslog(LOG_NOTICE, "Vendor Ext length       : %d",
               eep->vendor_ext.length);
        syslog(LOG_NOTICE, "Vendor Ext              : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
               eep->vendor_ext.val[0],
               eep->vendor_ext.val[1],
               eep->vendor_ext.val[2],
               eep->vendor_ext.val[3],
               eep->vendor_ext.val[4]);
        break;

      case ONIE_TLV_0xFE_CRC32:
        LAST_TLV = true;
        flx_get_uint32(&tmp32, &cur, 4);
        eep->crc32.type   = tl.type;
        eep->crc32.length = tl.length;
        eep->crc32.val    = ntohl(tmp32);

        syslog(LOG_NOTICE, "CRC32 type              : 0x%x",
               eep->crc32.type);
        syslog(LOG_NOTICE, "CRC32 length            : %d",
               eep->crc32.length);
        syslog(LOG_NOTICE, "CRC32                   : 0x%X",
               eep->crc32.val);
        break;

      default:
        LAST_TLV = true;
        syslog(LOG_NOTICE, "Invalid TLV type        : 0x%0x",
               tl.type);
        break;
    }
  }
  return rc;
}

static int
display_onie_eeprom(void)
{
  int rc = 0;
  struct onie_display_eeprom eeprom;
  char buf[FBW_EEPROM_SIZE];

  memset(&eeprom, 0, sizeof (struct onie_display_eeprom));

  rc = fread(buf, 1, sizeof(buf), f_onie_eep);
  if (rc < sizeof(buf)) {
    fprintf(stderr, "Failed to complete the read. Only got %d bytes\n", rc);
    rc = ENOSPC;
    goto out;
  }

  rc = parse_onie_buffer((const uint8_t *)buf, sizeof(buf), &eeprom);
  if (rc) {
    syslog(LOG_WARNING, "Failed to parse ONIE EEPROM");
    goto out;
  }

  printf("\nTLV Header Info\n");
  printf(" ID String	: %s\n", eeprom.hdr.idstr);
  printf(" Version	: %d\n", eeprom.hdr.version);
  printf(" Total Length	: %d\n", eeprom.hdr.total_len);

  printf("\nTLV Name             Code   Len   Value\n");
  printf("--------------------------------------------------------\n");
  printf("%-20s 0x%02X    %-3d  %s\n", "Product Name",
         eeprom.product_name.type,
         eeprom.product_name.length,
         eeprom.str21);

  printf("%-20s 0x%02X    %-3d  %s\n", "Part Number",
         eeprom.part_number.type,
         eeprom.part_number.length,
         eeprom.str22);

  printf("%-20s 0x%02X    %-3d  %s\n", "Serial Number",
         eeprom.serial_number.type,
         eeprom.serial_number.length,
         eeprom.str23);

  printf("%-20s 0x%02X    %-3d  %s\n", "Manufacturing Date",
         eeprom.mfg_date.type,
         eeprom.mfg_date.length,
         eeprom.str25);

  printf("%-20s 0x%02X    %-3d  %d\n", "Device Version",
         eeprom.device_version.type,
         eeprom.device_version.length,
         eeprom.device_version.val);

  printf("%-20s 0x%02X    %-3d  %s\n", "Platform Name",
         eeprom.platform_name.type,
         eeprom.platform_name.length,
         eeprom.str28);

  printf("%-20s 0x%02X    %-3d  %s\n", "Loader Version",
         eeprom.loader_version.type,
         eeprom.loader_version.length,
         eeprom.str29);

  printf("%-20s 0x%02X    %-3d  %d\n", "MAC Addresses",
         eeprom.mac_addrs.type,
         eeprom.mac_addrs.length,
         eeprom.mac_addrs.val);

  printf("%-20s 0x%02X    %-3d  %s\n", "Manufacturer",
         eeprom.manufacturer.type,
         eeprom.manufacturer.length,
         eeprom.str2B);

  printf("%-20s 0x%02X    %-3d  %s\n", "Country Code",
         eeprom.country_code.type,
         eeprom.country_code.length,
         eeprom.str2C);

  printf("%-20s 0x%02X    %-3d  %s\n", "Vendor Name",
         eeprom.vendor_name.type,
         eeprom.vendor_name.length,
         eeprom.str2D);

  printf("%-20s 0x%02X    %-3d  %s\n", "Diag Version",
         eeprom.diag_version.type,
         eeprom.diag_version.length,
         eeprom.str2E);

  printf("%-20s 0x%02X    %-3d  %s\n", "Service Tag",
         eeprom.service_tag.type,
         eeprom.service_tag.length,
         eeprom.str2F);

  printf("%-20s 0x%02X    %-3d  0x%x 0x%x 0x%x 0x%x 0x%x\n", "Vendor Extension",
         eeprom.vendor_ext.type,
         eeprom.vendor_ext.length,
         eeprom.vendor_ext.val[0],
         eeprom.vendor_ext.val[1],
         eeprom.vendor_ext.val[2],
         eeprom.vendor_ext.val[3],
         eeprom.vendor_ext.val[4]);

  printf("%-20s 0x%02X    %-3d  0x%X\n\n", "CRC-32",
         eeprom.crc32.type,
         eeprom.crc32.length,
         eeprom.crc32.val);

out:
  return rc;
}

static void
print_usage(void)
{
  fprintf(stderr, "Usage: onie-eeprom [ACTION] [FILE] [OVERWRITE]\n");
  fprintf(stderr, "Display or program the ONIE eeprom\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "ACTION\n");
  fprintf(stderr, "  disp   Display the eeprom (default)\n");
  fprintf(stderr, "  erase  Overwrite the eeprom with 0xFF\n");
  fprintf(stderr, "  prog   Program the eeprom with either the defaults\n");
  fprintf(stderr, "          or a provided file\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "OVERWRITE\n");
  fprintf(stderr, "  Only used with action \"prog\"\n");
  fprintf(stderr, "  n      Use data from the txt file only (default)\n");
  fprintf(stderr, "  y      Overwrite relevant fields with Board ID data\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "FILE\n");
  fprintf(stderr, "  Only used with action \"prog\" to provide an alternate file\n");
  fprintf(stderr, "   containing the TLVs to program into the eeprom\n");
  fprintf(stderr, "  The default file is \"/usr/bin/onie-eeprom.txt\" and only\n");
  fprintf(stderr, "   the TLVs in this file are supported\n");
  fprintf(stderr, "\n");
  return;
}

int
main (int argc, char **argv)
{
  int rc = 0;
  const char *fn = ONIE_EEPROM_FILE; /* using sysfs file */
  const char *ft = NULL;             /* txt file with defaults */
  eeprom_op_t action = EEP_OPER_INVALID;
  bool use_brd_id = false;

  if (argc > 1) { 
    if (strcmp("prog", argv[1]) == 0) {
      if (argc > 2) {
        if (strcmp("y", argv[2]) == 0) {
          use_brd_id = true;
          syslog(LOG_NOTICE, "Using the Board ID EEPROM to fill in relevant data");
        }
        if (argc > 3) {
          ft = argv[3];
          syslog(LOG_NOTICE, "Programming the ONIE eeprom with %s", ft);
        }
      }
      action = EEP_OPER_PROGRAM;

    } else if (strcmp("disp", argv[1]) == 0) {
      syslog(LOG_NOTICE, "Displaying the ONIE eeprom");
      action = EEP_OPER_DISPLAY;

    } else if (strcmp("erase", argv[1]) == 0) {
      syslog(LOG_NOTICE, "Erasing the ONIE eeprom");
      action = EEP_OPER_ERASE;

    } else {
      fprintf(stderr, "Invalid option : %s\n", argv[1]);
      return -1;
    }

  } else {
    syslog(LOG_NOTICE, "Displaying the ONIE eeprom");
    action = EEP_OPER_DISPLAY;
  }

  syslog(LOG_NOTICE, "Opening %s", fn);
  f_onie_eep = fopen(fn, "r+");
  if (f_onie_eep == NULL) {
    rc = errno;
    fprintf(stderr, "Failed to open %s\n", fn);
    goto out;
  }

  switch (action) {
    case EEP_OPER_PROGRAM:
      rc = program_onie_eeprom(ft, use_brd_id);
      break;

    case EEP_OPER_DISPLAY:
      rc = display_onie_eeprom();
      break;

    case EEP_OPER_ERASE:
      rc = erase_onie_eeprom();
      break;

    default:
      // should not happen
      fprintf(stderr, "Invalid action %d\n", action);
      rc = -1;
      break;
  }

  if (rc) {
    fprintf(stderr, "%s FAILED!\n", argv[0]);
    print_usage();
    goto out;
  } else {
    syslog(LOG_NOTICE, "%s SUCCESSFUL", argv[0]);
  }

out:
  if (f_onie_eep) {
    fclose(f_onie_eep);
  }

  return rc;
}

