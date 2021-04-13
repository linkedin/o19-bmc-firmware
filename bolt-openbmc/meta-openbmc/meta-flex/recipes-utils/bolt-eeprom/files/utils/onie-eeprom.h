#ifndef FLX_ONIE_EEPROM_H
#define FLX_ONIE_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <facebook/flex_eeprom.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <endian.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define ONIE_EEPROM_FILE "/sys/class/i2c-adapter/i2c-4/4-0050/eeprom"
#define ONIE_EEPROM_TXT  "/usr/local/bin/onie-eeprom.txt"
#define ONIE_EEPROM_EXAMPLE_TXT  "/usr/local/bin/onie-eeprom.example"
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

typedef struct _onie_tlv_hdr_for_disp{
  char     idstr[ONIE_EEPROM_TLV_HDR_IDSTR_SZ+1];
  uint8_t  version;
  uint16_t total_len;
} onie_tlv_hdr_for_disp;

typedef struct _onie_tlv_ext{
  uint8_t  type;
  uint8_t  length;
  uint8_t  val[ONIE_TLV_0xFD_VENDOR_EXT_LEN];
} onie_tlv_ext;

struct onie_display_eeprom {
  onie_tlv_hdr_for_disp hdr;
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

#ifdef __cplusplus
}
#endif

#endif /* FLX_ONIE_EEPROM_H */
