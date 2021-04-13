#!/bin/sh
# 
# Copyright 2018-present linkedin. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

#WDT2 Timeout Status Register: WDT30 DT address 0x1E785000 + 0x30
#bit 1:   0 - default boot code
#         1 - second boot code
DEVMEM=/sbin/devmem
WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE=2

boot=$($DEVMEM 0x1E785030)
boot=$((boot&$WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE))

SW_VERSION="v1.4.0"
echo "BMC Software Version: Delta powershelf Dual BMC $SW_VERSION"

# Check if boot code source is flash1
if [ $boot -eq $WDT2_TMOUT_STAT_REG_2ND_BOOT_CODE ]; then
   echo "boot flash: flash1"
else
   echo "boot flash: flash0"
fi
