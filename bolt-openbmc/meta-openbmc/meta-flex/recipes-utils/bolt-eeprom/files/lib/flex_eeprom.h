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
#ifndef FBW_EEPROM_H
#define FBW_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#define FBW_EEPROM_F_MAGIC 4
#define FBW_EEPROM_F_VERSION 2
#define FBW_EEPROM_F_PRODUCT_NAME 16
#define FBW_EEPROM_F_PRODUCT_NUMBER 16
#define FBW_EEPROM_F_ASSEMBLY_NUMBER 12
#define FBW_EEPROM_F_FACEBOOK_PCBA_NUMBER 14
#define FBW_EEPROM_F_FACEBOOK_PCB_NUMBER 14
#define FBW_EEPROM_F_ODM_PCBA_NUMBER 14
#define FBW_EEPROM_F_ODM_PCBA_SERIAL 24 
#define FBW_EEPROM_F_PRODUCT_STATE 2
#define FBW_EEPROM_F_PRODUCT_VERSION 1
#define FBW_EEPROM_F_PRODUCT_SUBVERSION 1
#define FBW_EEPROM_F_PRODUCT_SERIAL 28 
#define FBW_EEPROM_F_PRODUCT_ASSET 12
#define FBW_EEPROM_F_SYSTEM_MANUFACTURER 8
#define FBW_EEPROM_F_SYSTEM_MANU_DATE 8
#define FBW_EEPROM_F_PCB_MANUFACTURER 8
#define FBW_EEPROM_F_ASSEMBLED 8
#define FBW_EEPROM_F_LOCAL_MAC 12
#define FBW_EEPROM_F_EXT_MAC_BASE 12
#define FBW_EEPROM_F_EXT_MAC_SIZE 2
#define FBW_EEPROM_F_LOCATION 8
#define FBW_EEPROM_F_CRC8 1

#define FBW_EEPROM_F_SYSTEM_MANU_DATE_v2 8
#define FBW_EEPROM_F_SYSTEM_MANU_DATE_v3 12
#define FBW_EEPROM_F_SYSTEM_MANU_DATE_v4 14

#define FBW_EEPROM_MAGIC_NUM	0xFBFB
#define FBW_EEPROM_MAGIC_BYTE0	0xFB
#define FBW_EEPROM_MAGIC_BYTE1	0xFB
#define FBW_EEPROM_CRC8_CSUM	0xED

#define FLEX_EEPROM_CRC8_CSUM	0
#define FLEX_EEPROM_F_PROD_VERS 8
#define FLEX_EEPROM_F_HW_REV    8
#define FLEX_EEPROM_MAGIC_NUM   0xBEBE
#define FLEX_EEPROM_MAGIC_BYTE0 0xBE
#define FLEX_EEPROM_MAGIC_BYTE1 0xBE

#define FBW_EEPROM_VERSION0 0
#define FBW_EEPROM_V0_SIZE 162
#define FBW_EEPROM_VERSION1 1
#define FBW_EEPROM_V1_SIZE 174
#define FBW_EEPROM_VERSION2 2
#define FBW_EEPROM_V2_SIZE 196
#define FBW_EEPROM_VERSION5 5
#define FBW_EEPROM_V5_SIZE 202 /* sizeof (struct wedge_eeprom_gn) */

#define FBW_OLD_VERSION_SIZE 1

/*
 * EEPROM Version # is being reset to 1 for production
 * This poses some problems for the pre production units
 * that already have versions of 2 or 5.  However these
 * should get cleared out with this release (6).
 */
#define LI_EEPROM_VERSION1 1
#define LI_EEPROM_V1_SIZE  199

#define LI_EEPROM_MAGIC_NUM   "LI19"
#define LI_EEPROM_MAGIC_BYTE0 "L"
#define LI_EEPROM_MAGIC_BYTE1 "I"
#define LI_EEPROM_MAGIC_BYTE2 "1"
#define LI_EEPROM_MAGIC_BYTE3 "9"

#define LI_EEPROM_VERSION2     3 
#define LI_EEPROM_VERSION2_STR "02"
#define LI_EEPROM_V2_SIZE      223

/*
 * The eeprom size is 8K, we only use 157 bytes for v1 format.
 * Read 256 for now.
 */
#define FBW_EEPROM_SIZE 256

#ifndef FBW_EEPROM_FILE
#define FBW_EEPROM_FILE "/sys/class/i2c-adapter/i2c-6/6-0051/eeprom"
#endif

#define FBW_EEPROM_BOOT_WITH_BAD_MAGIC_NUM

struct wedge_eeprom_st {
  /* version number of the eeprom. Must be the first element */
  char fbw_version[FBW_EEPROM_F_VERSION + 1];

  /* Product Name */
  char fbw_product_name[FBW_EEPROM_F_PRODUCT_NAME + 1];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char fbw_product_number[FBW_EEPROM_F_PRODUCT_NUMBER + 2];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char fbw_assembly_number[FBW_EEPROM_F_ASSEMBLY_NUMBER + 2];

  /* Facebook PCBA Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcba_number[FBW_EEPROM_F_FACEBOOK_PCBA_NUMBER + 2];

  /* Facebook PCB Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcb_number[FBW_EEPROM_F_FACEBOOK_PCB_NUMBER + 3];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_number[FBW_EEPROM_F_ODM_PCBA_NUMBER + 2];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_serial[FBW_EEPROM_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  char fbw_production_state[FBW_EEPROM_F_PRODUCT_STATE + 1];

  /* Product Version */
  char fbw_product_version[FBW_EEPROM_F_PRODUCT_VERSION + 1];

  /* Product Sub Version */
  char fbw_product_subversion[FBW_EEPROM_F_PRODUCT_SUBVERSION + 1];

  /* Product Serial Number: XXXXXXXX */
  char fbw_product_serial[FBW_EEPROM_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char fbw_product_asset[FBW_EEPROM_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char fbw_system_manufacturer[FBW_EEPROM_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date: mm-dd-yyyy */
  char fbw_system_manufacturing_date[FBW_EEPROM_F_SYSTEM_MANU_DATE + 3];

  /* PCB Manufacturer: XXXXXXXXX */
  char fbw_pcb_manufacturer[FBW_EEPROM_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char fbw_assembled[FBW_EEPROM_F_ASSEMBLED + 1];

  /* Local MAC Address */
  uint8_t fbw_local_mac[6];

  /* Extended MAC Address */
  uint8_t fbw_mac_base[6];

  /* Extended MAC Address Size */
  uint16_t fbw_mac_size;

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char fbw_location[FBW_EEPROM_F_LOCATION + 1];

  /* CRC8 */
  uint8_t fbw_crc8;
};

/* describe the actual eeprom content (minus the 4-byte magic number header) */
/* wedge_eeprom_parse() expects this field ordering and sizing */
struct wedge_eeprom_gn {
  /* version number of the eeprom. Must be the first element */
  char fbw_version[FBW_EEPROM_F_VERSION];

  /* Product Name */
  char fbw_product_name[FBW_EEPROM_F_PRODUCT_NAME];

  /* Top Level 20 - Product Part Number: XX-XXXXXX */
  char fbw_product_number[FBW_EEPROM_F_PRODUCT_NUMBER];

  /* System Assembly Part Number XXX-XXXXXX-XX */
  char fbw_assembly_number[FBW_EEPROM_F_ASSEMBLY_NUMBER];

  /* Bolt PCBA Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcba_number[FBW_EEPROM_F_FACEBOOK_PCBA_NUMBER];

  /* Bolt PCB Part Number: XXX-XXXXXXX-XX */
  char fbw_facebook_pcb_number[FBW_EEPROM_F_FACEBOOK_PCB_NUMBER];

  /* ODM PCB Part Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_number[FBW_EEPROM_F_ODM_PCBA_NUMBER];

  /* ODM PCB Serial Number: XXXXXXXXXXXX */
  char fbw_odm_pcba_serial[FBW_EEPROM_F_ODM_PCBA_SERIAL];

  /* Product Production State */
  char fbw_production_state[FBW_EEPROM_F_PRODUCT_STATE];

  /* Product Version */
  char fbw_product_version[FBW_EEPROM_F_PRODUCT_VERSION];

  /* Product Sub Version */
  char fbw_product_subversion[FBW_EEPROM_F_PRODUCT_SUBVERSION];

  /* Product Serial Number: XXXXXXXX */
  char fbw_product_serial[FBW_EEPROM_F_PRODUCT_SERIAL];

  /* Product Asset Tag: XXXXXXXX */
  char fbw_product_asset[FBW_EEPROM_F_PRODUCT_ASSET];

  /* System Manufacturer: XXXXXXXX */
  char fbw_system_manufacturer[FBW_EEPROM_F_SYSTEM_MANUFACTURER];

  /* System Manufacturing Date:  MM/DD/YYYY */
  char fbw_system_manufacturing_date[FBW_EEPROM_F_SYSTEM_MANU_DATE];

  /* PCB Manufacturer: XXXXXXXXX */
  char fbw_pcb_manufacturer[FBW_EEPROM_F_PCB_MANUFACTURER];

  /* Assembled At: XXXXXXXX */
  char fbw_assembled[FBW_EEPROM_F_ASSEMBLED];

  /* Local MAC Address */
  char fbw_local_mac[FBW_EEPROM_F_LOCAL_MAC];

  /* Extended MAC Address */
  char fbw_mac_base[FBW_EEPROM_F_EXT_MAC_BASE];

  /* Extended MAC Address Size */
  uint16_t fbw_mac_size;

  /* Location on Fabric: "LEFT"/"RIGHT", "WEDGE", "LC" */
  char fbw_location[FBW_EEPROM_F_LOCATION];

  /* CRC8 */
  uint8_t fbw_crc8;
};

void copy_data (char *src, char *dst, int bytes);
void read_data (FILE *fd, char *str, int size);
int fbw_fill_struct_from_file (FILE *fd, struct wedge_eeprom_gn *eeprom);
int wedge_eeprom_parse(const char *fn, struct wedge_eeprom_st *eeprom);
uint8_t flx_crc8(const uint8_t *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
