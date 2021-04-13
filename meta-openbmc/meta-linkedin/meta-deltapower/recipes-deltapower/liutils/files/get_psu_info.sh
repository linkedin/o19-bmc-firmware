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
 
usage() {
    echo "Usage: $0 [psu unit [1..6]]" >&2
}

DIR1=/sys/class/i2c-adapter/

psu_addr[0]=$DIR1/i2c-12/12-0040
psu_addr[1]=$DIR1/i2c-1/1-0040
psu_addr[2]=$DIR1/i2c-2/2-0040
psu_addr[3]=$DIR1/i2c-3/3-0040
psu_addr[4]=$DIR1/i2c-6/6-0040
psu_addr[5]=$DIR1/i2c-7/7-0040

TMPS="vin vout pin pout iin iout"

set -e

if [ "$#" -eq 0 ]; then
    echo "all PSUs"
    PSUS="${psu_addr[0]} ${psu_addr[1]}  ${psu_addr[2]} ${psu_addr[3]} ${psu_addr[4]} ${psu_addr[5]}"
elif [ "$#" -eq 1 ]; then
    if [ $1 -lt 1 -o $1 -gt 6 ]; then
        echo "PSU unit out of range"
        usage
        exit 1
    fi

    case "$1" in
    "1"|"2"|"3"|"4"|"5"|"6")
        PSUS=${psu_addr[$1 - 1]}
        ;;
    *)
        usage
        exit 1
        ;;
    esac
else
    usage
    exit 1
fi

for psu in $PSUS; do
    echo "PSU#: $1"
    for tmp in $TMPS; do
        tmp_var=$psu/$tmp
        tmp_val=$(cat $tmp_var)
        echo "$tmp: :    $tmp_val"
    done
done
