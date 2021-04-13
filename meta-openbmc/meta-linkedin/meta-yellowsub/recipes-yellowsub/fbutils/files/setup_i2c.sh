#!/bin/sh
#
# Copyright 2014-present Facebook. All Rights Reserved.
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
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

. /usr/local/bin/openbmc-utils.sh

# Bus 1 NIC temp sensor
i2c_device_add 1 0x1f tmp421

# Bus 2 Power Manangement
i2c_device_add 2 0x3a pwr1014a

# Bus 3 temp sensors
i2c_device_add 3 0x48 tmp75
i2c_device_add 3 0x49 tmp75
i2c_device_add 3 0x4a tmp75
i2c_device_add 3 0x4b tmp75
i2c_device_add 3 0x4c tmp75

# Bus 6 FRU eeprom
i2c_device_add 6 0x51 24c64          # FRU EEPROM

# Bus 9 SoC
i2c_device_add 9 0x77 pca954x
i2cset -f -y 9 0x77 0x1
i2c_device_add 9 0x31 soccpld

#Bus 10 NVMe
i2c_device_add 10 0x70, 0x71, 0x72, and 0x73 pca954x
i2c_device_add 10 0x70 pca954x
i2c_device_add 10 0x71 pca954x
i2c_device_add 10 0x72 pca954x
i2c_device_add 10 0x73 pca954x

# Bus 11 adm1275 HSC
#i2c_device_add 11 0x10 adm1275

# Bus 11 adm1278 HSC
i2c_device_add 11 0x10 adm1278

# Bus 12 syscpld
i2c_device_add 12 0x31 syscpld
