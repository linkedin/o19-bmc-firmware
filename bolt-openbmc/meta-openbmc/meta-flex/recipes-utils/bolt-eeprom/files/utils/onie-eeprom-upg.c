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
#include <stdlib.h>
#include "onie-eeprom.h"

FILE *f_onie_eep, *ftxt;


static uint32_t
flx_crc32(uint32_t crc, char *buf, size_t len)
{
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
read_tlv_hdr(onie_tlv_hdr *hdr, uint32_t *unswapped)
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
  *unswapped = atoi(strtok(NULL, ","));
  hdr->total_len = htons(*unswapped);
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
  uint32_t len_unswapped = 0;

  read_tlv_hdr(&eeprom->hdr, &len_unswapped);
  syslog(LOG_NOTICE, "ONIE hdr ID addr        : %p", &eeprom->hdr);
  syslog(LOG_NOTICE, "ONIE hdr ID str         : %s", eeprom->hdr.idstr);
  syslog(LOG_NOTICE, "ONIE hdr version        : %d", eeprom->hdr.version);
  syslog(LOG_NOTICE, "ONIE hdr total len      : %d", len_unswapped);

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
program_onie_eeprom(void)
{
  int rc = 0;
  const char* onie_fn = ONIE_EEPROM_EXAMPLE_TXT;
  struct bolt_onie_eeprom_gn eeprom;
  struct wedge_eeprom_st id_eeprom;

  memset(&eeprom, 0, sizeof (struct bolt_onie_eeprom_gn));
  memset(&id_eeprom, 0, sizeof (struct wedge_eeprom_st));

  syslog(LOG_NOTICE, "Opening %s", onie_fn);
  ftxt = fopen(onie_fn, "r");
  if (ftxt == NULL) {
      rc = errno;
      fprintf(stderr, "Failed to open %s\n", onie_fn);
      goto out;
  }

  syslog(LOG_NOTICE, "Initializing eeprom data from %s", onie_fn);
  rc = init_eeprom_data(&eeprom);
  if (rc) {
    goto out;
  } 
  syslog(LOG_NOTICE, "Done initializing eeprom data");

  syslog(LOG_NOTICE, "Now copy the data from ID eeprom");

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

  uint32_t crc32;
  ssize_t sz = sizeof (struct bolt_onie_eeprom_gn);
  size_t crc_len;
  /* calculate CRC32 based on all bytes but crc itself */
  /* validate on all bytes, resulting in crc value 0 */
  crc_len              = sz - ONIE_TLV_0xFE_CRC32_LEN;

  eeprom.crc32.type    = ONIE_TLV_0xFE_CRC32;
  eeprom.crc32.length  = ONIE_TLV_0xFE_CRC32_LEN;
  crc32                = flx_crc32(0, (char*)&eeprom, crc_len);

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

int
main (int argc, char **argv)
{
  int rc = 0;
  const char *fn = ONIE_EEPROM_FILE; /* using sysfs file */

  syslog(LOG_NOTICE, "Opening %s", fn);
  f_onie_eep = fopen(fn, "r+");
  if (f_onie_eep == NULL) {
    rc = errno;
    fprintf(stderr, "Failed to open %s\n", fn);
    goto out;
  }

  rc = program_onie_eeprom();
  if (rc) {
    fprintf(stderr, "%s FAILED!\n", argv[0]);
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

