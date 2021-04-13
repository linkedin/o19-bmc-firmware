/*
 * Copyright 2018-present Linkedin. All Rights Reserved.
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
#ifndef LINKEDIN_EEPROM_H
#define LINKEDIN_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#ifndef LI_EEPROM_FILE
#define LI_EEPROM_FILE "/sys/class/i2c-adapter/i2c-8/8-0050/eeprom"
#endif

#define EEPROM_ADDR   0x50
#define EEPROM_BUS    8
#define EEPROM_MUX    0x7c

/*
 * Linkedin eeprom Magic word:  LI19
 */
#define LI_EEPROM_MAGIC_WORD     0x3931494c

#define LI_EEPROM_F_MAGIC 4
#define LI_EEPROM_F_VERSION 2
#define LI_EEPROM_F_PRODUCT_NAME 16
#define LI_EEPROM_F_PRODUCT_NUMBER 16
#define LI_EEPROM_F_ASSEMBLY_NUMBER 12
#define LI_EEPROM_F_PCBA_NUMBER 14
#define LI_EEPROM_F_PCB_NUMBER 14
#define LI_EEPROM_F_ODM_PCBA_NUMBER 14
#define LI_EEPROM_F_ODM_PCBA_SERIAL 24
#define LI_EEPROM_F_PRODUCT_STATE 2
#define LI_EEPROM_F_PRODUCT_VERSION 1
#define LI_EEPROM_F_PRODUCT_SUBVERSION 1
#define LI_EEPROM_F_PRODUCT_SERIAL 28
#define LI_EEPROM_F_PRODUCT_ASSET 12
#define LI_EEPROM_F_SYSTEM_MANUFACTURER 8
#define LI_EEPROM_F_SYSTEM_MANU_DATE 8
#define LI_EEPROM_F_PCB_MANUFACTURER 8
#define LI_EEPROM_F_ASSEMBLED 8
#define LI_EEPROM_F_LOCAL_MAC 12
#define LI_EEPROM_F_EXT_MAC_BASE 12
#define LI_EEPROM_F_EXT_MAC_SIZE 2
#define LI_EEPROM_F_RESERVED 8
#define LI_EEPROM_F_CRC8 1

#define __MAX(a, b) (((a) > (b)) ? (a) : (b))

#define LI_EEPROM_MAGIC_NUM   "LI19"
#define LI_EEPROM_MAGIC_BYTE0 "L"
#define LI_EEPROM_MAGIC_BYTE1 "I"
#define LI_EEPROM_MAGIC_BYTE2 "1"
#define LI_EEPROM_MAGIC_BYTE3 "9"

#define LI_EEPROM_VERSION0 0
#define LI_EEPROM_V0_SIZE 162
#define LI_EEPROM_VERSION1 1
#define LI_EEPROM_V1_SIZE 174
#define LI_EEPROM_VERSION2 2
#define LI_EEPROM_VERSION2_STR "02"
#define LI_EEPROM_V2_SIZE      227

 /* The eeprom size is 8K, we only use 157 bytes for v1 format.
  * Read 256 for now.
  */
#define LI_EEPROM_SIZE 256
#define LI_EEPROM_CRC8_CSUM   0

/*
 * offset of local MAC address
 */
#define LI_LOCAL_MAC_OFFSET_V0    134
#define LI_EXT_MAC_OFFSET_V0      146
#define LI_LOCAL_MAC_OFFSET_V1    156

/*
 * Current LinkedIn system eeprom version is version: 02
 */
struct li_eeprom_st {
  /* version number of the eeprom. Must be the first element */
  char li_version[LI_EEPROM_F_VERSION + 1];

  /* Product Name: 16 characters XX-XXXXXX */
  char li_product_name[LI_EEPROM_F_PRODUCT_NAME + 1];

  /* Product Part Number: 16 characters XXXXXXXXXXXXXXXX */
  char li_product_number[LI_EEPROM_F_PRODUCT_NUMBER + 1];

  /* System Assembly Part Number: 12 characters XXXXXXXXXXXX */
  char li_assembly_number[LI_EEPROM_F_ASSEMBLY_NUMBER + 2];

  /* linkedin PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcba_number[LI_EEPROM_F_PCBA_NUMBER + 2];

  /* Linkedin PCB Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcb_number[LI_EEPROM_F_PCB_NUMBER + 3];

  /* ODM PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_odm_pcba_number[LI_EEPROM_F_ODM_PCBA_NUMBER + 2];

  /* ODM PCBA Serial Number: 12 characters XXXXXXXXXXXX */
  char li_odm_pcba_serial[LI_EEPROM_F_ODM_PCBA_SERIAL + 1];

  /* Product Production State */
  char li_production_state[LI_EEPROM_F_PRODUCT_STATE + 1];

  /* Product Version */
  char li_product_version[LI_EEPROM_F_PRODUCT_VERSION + 1];

  /* Product Sub Version */
  char li_product_subversion[LI_EEPROM_F_PRODUCT_SUBVERSION + 1];

  /* Product Serial Number: 16 characters XXXXXXXXXXXXXXXX */
  char li_product_serial[LI_EEPROM_F_PRODUCT_SERIAL + 1];

  /* Product Asset Tag: XXXXXXXX */
  char li_product_asset[LI_EEPROM_F_PRODUCT_ASSET + 1];

  /* System Manufacturer: XXXXXXXX */
  char li_system_manufacturer[LI_EEPROM_F_SYSTEM_MANUFACTURER + 1];

  /* System Manufacturing Date: mm-dd-yyyy */
  uint8_t li_system_manufacturing_date[LI_EEPROM_F_SYSTEM_MANU_DATE + 3];

  /* PCB Manufacturer: XXXXXXXXX */
  char li_pcb_manufacturer[LI_EEPROM_F_PCB_MANUFACTURER + 1];

  /* Assembled At: XXXXXXXX */
  char li_assembled[LI_EEPROM_F_ASSEMBLED + 1];

  /* Local MAC Address */
  uint8_t li_local_mac[6];

  /* Extended MAC Address */
  uint8_t li_mac_base[6];

  /* Extended MAC Address Size */
  uint16_t li_mac_size;

  /* Reserved field: 8 characters */
  char li_reserved[LI_EEPROM_F_RESERVED + 1];

  /* CRC8 */
  uint8_t li_crc8;
};

/*
 * describe the actual eeprom content (minus the 4-byte magic number header)
 * li_eeprom_parse() expects this field ordering and sizing
 */
struct li_eeprom_gn {
  /* version number of the eeprom. Must be the first element, 2 characters */
  char li_version[LI_EEPROM_F_VERSION];

  /* Product Name: 16 characters XX-XXXXXX */
  char li_product_name[LI_EEPROM_F_PRODUCT_NAME];

  /* Product Part Number: 16 characters XXXXXXXXXXXXXXXX */
  char li_product_number[LI_EEPROM_F_PRODUCT_NUMBER];

  /* System Assembly Part Number: 12 characters XXXXXXXXXXXX */
  char li_assembly_number[LI_EEPROM_F_ASSEMBLY_NUMBER];

  /* linkedin PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcba_number[LI_EEPROM_F_PCBA_NUMBER];

  /* Linkedin PCB Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_pcb_number[LI_EEPROM_F_PCB_NUMBER];

  /* ODM PCBA Part Number: 14 characters XXXXXXXXXXXXXX */
  char li_odm_pcba_number[LI_EEPROM_F_ODM_PCBA_NUMBER];

  /* ODM PCBA Serial Number: 24 characters XXXXXXXXXXXX */
  char li_odm_pcba_serial[LI_EEPROM_F_ODM_PCBA_SERIAL];

  /* Product Production State: 2 characters  */
  char li_production_state[LI_EEPROM_F_PRODUCT_STATE];

  /* Product Version: 1 character */
  char li_product_version[LI_EEPROM_F_PRODUCT_VERSION];

  /* Product Sub Version: 1 character */
  char li_product_subversion[LI_EEPROM_F_PRODUCT_SUBVERSION];

  /* Product Serial Number: 28 characters XXXXXXXXXXXXXXXX */
  char li_product_serial[LI_EEPROM_F_PRODUCT_SERIAL];

  /* Product Asset Tag: 12 characters XXXXXXXX */
  char li_product_asset[LI_EEPROM_F_PRODUCT_ASSET];

  /* System Manufacturer: 8 characters XXXXXXXX */
  char li_system_manufacturer[LI_EEPROM_F_SYSTEM_MANUFACTURER];

  /* System Manufacturing Date: 8 characters */
  uint8_t li_system_manufacturing_date[LI_EEPROM_F_SYSTEM_MANU_DATE];

  /* PCB Manufacturer: 8 characters XXXXXXXXX */
  char li_pcb_manufacturer[LI_EEPROM_F_PCB_MANUFACTURER];

  /* Assembled At: 8 characters XXXXXXXX */
  char li_assembled[LI_EEPROM_F_ASSEMBLED];

  /* Local MAC Address: 12 characters */
  char li_local_mac[LI_EEPROM_F_LOCAL_MAC];

  /* Extended MAC Address: 12 characters */
  char li_mac_base[LI_EEPROM_F_EXT_MAC_BASE];

  /* Extended MAC Address Size: 2 bytes integer */
  uint16_t li_mac_size;

  /* Reserved field: 8 characters */
  char li_reserved[LI_EEPROM_F_RESERVED];

  /* CRC8 */
  uint8_t li_crc8;
};

int li_eeprom_parse(const char *fn, struct li_eeprom_st *eeprom);
void copy_data (char *src, char *dst, int bytes);
void read_data (FILE *fd, char *str, int size);
int li_fill_struct_from_file (FILE *fd, struct li_eeprom_gn *eeprom);
unsigned char li_crc8(unsigned char* buf, int length);

#ifdef __cplusplus
}
#endif

#endif
