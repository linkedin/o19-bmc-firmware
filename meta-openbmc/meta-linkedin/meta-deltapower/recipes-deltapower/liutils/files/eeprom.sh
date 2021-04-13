#!/bin/sh
#
# Copyright 2018-present LinkedIn. All Rights Reserved.
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
#

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/set_control.sh
. /usr/local/bin/ast-functions

usage() {
    echo "Usage: $0 " >&2
}

get_i2c_control 8 124

if [ "$#" -eq 0 ]; then
    # get eeprom info
    check_driver 8 50 24c02 eeprom
    get_i2c_control 8 124
    output=$(/usr/local/bin/weutil)
    echo "$output"
else
   usage
   exit 1
fi
