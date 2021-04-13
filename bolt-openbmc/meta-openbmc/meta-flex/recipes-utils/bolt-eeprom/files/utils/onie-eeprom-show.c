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

#include "onie-eeprom.h"


FILE *f_onie_eep;


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
                    &cur, eep->product_name.length);
//                    &cur, ONIE_TLV_0x21_PROD_NAME_LEN);

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
                    &cur, eep->part_number.length);
//                    &cur, ONIE_TLV_0x22_PART_NUM_LEN);

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
                    &cur, eep->serial_number.length);
//                    &cur, ONIE_TLV_0x23_SERIAL_NUM_LEN);

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
                    &cur, eep->mfg_date.length);
//                    &cur, ONIE_TLV_0x25_MFG_DATE_LEN);

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
                    &cur, eep->platform_name.length);
//                    &cur, ONIE_TLV_0x28_PLAT_NAME_LEN);

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
                    &cur, eep->loader_version.length);
//                    &cur, ONIE_TLV_0x29_LOADER_VER_LEN);

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
                    &cur, eep->manufacturer.length);
//                    &cur, ONIE_TLV_0x2B_MANUFACTURER_LEN);

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
                    &cur, eep->country_code.length);
//                    &cur, ONIE_TLV_0x2C_COUNTRY_CODE_LEN);

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
                    &cur, eep->vendor_name.length);
//                    &cur, ONIE_TLV_0x2D_VENDOR_NAME_LEN);

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
                    &cur, eep->diag_version.length);
//                    &cur, ONIE_TLV_0x2E_DIAG_VERSION_LEN);

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
                    &cur, eep->service_tag.length);
//                    &cur, ONIE_TLV_0x2F_SERVICE_TAG_LEN);

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

  rc = display_onie_eeprom();
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

