/*
 * bootoption-util
 *
 * Copyright 2018-present LinkedIn. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <openbmc/pal.h>
#include <openbmc/ipmi.h>

#define BOOTOPTION_PXE        0x4
#define BOOTOPTION_DISK       0x8
#define BOOTOPTION_BIOS       0x18

#define BOOTOPTION_SIZE       256

static void
print_usage_help(void) {
  printf("       bootoption-util <slot1|slot2|slot3|slot4> <pxe|bios|disk>\n");
}

int
main(int argc, char **argv) {

  uint8_t slot_id;
  char tstr[64] = {0};
  char pbuff[BOOTOPTION_SIZE];

  if (argc < 3) {
    goto err_exit;
  }

  if (!strcmp(argv[1], "slot1")) {
    slot_id = 1;
  } else if (!strcmp(argv[1] , "slot2")) {
    slot_id = 2;
  } else if (!strcmp(argv[1] , "slot3")) {
    slot_id = 3;
  } else if (!strcmp(argv[1] , "slot4")) {
    slot_id = 4;
  } else {
    goto err_exit;
  }

  memset(pbuff, 0, BOOTOPTION_SIZE);

  /*
   * Set System Boot Options (IPMI/Section 28)
   */
  pbuff[0] = 0;
  pbuff[1] = 0;
  pbuff[2] = 0;
  pbuff[3] = 0;
  pbuff[4] = 1;
  pbuff[5] = 1;
  pbuff[6] = 0x80;

  if (!strcmp(argv[2], "pxe")) {
    syslog(LOG_INFO, "bootoption-util: bootoption PXE for slot%d\n", slot_id);
    pbuff[7] = BOOTOPTION_PXE;
  }
  else if (!strcmp(argv[2], "bios")) {
    syslog(LOG_INFO, "bootoption-util: bootoption BIOS for slot%d\n", slot_id);
    pbuff[7] = BOOTOPTION_BIOS;
  }
  else if (!strcmp(argv[2], "disk")) {
    syslog(LOG_INFO, "bootoption-util: bootoption disk for slot%d\n", slot_id);
    pbuff[7] = BOOTOPTION_DISK;
  } else {
    goto err_exit;
  }

  pal_save_boot_option(slot_id, pbuff);
  return 0;
err_exit:
  print_usage_help();
  return -1;
}
