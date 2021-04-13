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

#include <errno.h>
#include <stdio.h>

#include <facebook/flex_eeprom.h>
#include <openbmc/log.h>

/*
 * Following fruid info translates to thrift structures in Fboss
 * and has a dependency in FBNet
 */

int main(int argc, const char *argv[])
{
  const char *fn;
  struct wedge_eeprom_st eeprom;
  int rc;

  if (argc >= 2) {
    fn = argv[1];
  } else {
    fn = NULL;
  }

  rc = wedge_eeprom_parse(fn, &eeprom);
  if (rc) {
    fprintf(stderr, "Failed to parse %s EEPROM\n", fn ? fn : "default");
    return -1;
  }

  printf("%-28s : %d\n", "Bolt ID EEPROM", eeprom.fbw_version);
  printf("%-28s : %s\n", "Product Name", eeprom.fbw_product_name);
  printf("%-28s : %s\n", "Product Part Number", eeprom.fbw_product_number);
  printf("%-28s : %s\n", "System Assembly Part Number", eeprom.fbw_assembly_number);
  printf("%-28s : %s\n", "LinkedIn PCBA Part Number", eeprom.fbw_facebook_pcba_number);
  printf("%-28s : %s\n", "LinkedIn PCB Part Number", eeprom.fbw_facebook_pcb_number);
  printf("%-28s : %s\n", "Flex PCBA Part Number", eeprom.fbw_odm_pcba_number);
  printf("%-28s : %s\n", "Flex PCBA Serial Number", eeprom.fbw_odm_pcba_serial);
  printf("%-28s : %d\n", "Production State", eeprom.fbw_production_state);
  printf("%-28s : %d\n", "Product Version", eeprom.fbw_product_version);
  printf("%-28s : %d\n", "Product Sub Version", eeprom.fbw_product_subversion);
  printf("%-28s : %s\n", "System Serial Number", eeprom.fbw_product_serial);
  printf("%-28s : %s\n", "Product Asset Tag", eeprom.fbw_product_asset);
  printf("%-28s : %s\n", "System Manufacturer", eeprom.fbw_system_manufacturer);
  printf("%-28s : %s\n", "System Manufacturing Date", eeprom.fbw_system_manufacturing_date);
  printf("%-28s : %s\n", "PCB Manufacturer", eeprom.fbw_pcb_manufacturer);
  printf("%-28s : %s\n", "Assembled At", eeprom.fbw_assembled);
  printf("%-28s : %02X:%02X:%02X:%02X:%02X:%02X\n", "BMC MAC",
           eeprom.fbw_local_mac[0], eeprom.fbw_local_mac[1],
           eeprom.fbw_local_mac[2], eeprom.fbw_local_mac[3],
           eeprom.fbw_local_mac[4], eeprom.fbw_local_mac[5]);
  printf("%-28s : %02X:%02X:%02X:%02X:%02X:%02X\n", "Extended MAC",
           eeprom.fbw_mac_base[0], eeprom.fbw_mac_base[1],
           eeprom.fbw_mac_base[2], eeprom.fbw_mac_base[3],
           eeprom.fbw_mac_base[4], eeprom.fbw_mac_base[5]);
  printf("%-28s : %d\n", "Extended MAC size", eeprom.fbw_mac_size);
  printf("%-28s : %s\n", "Reserved", eeprom.fbw_location);
  printf("%-28s : %x\n", "CRC8", eeprom.fbw_crc8);

  return 0;
}
