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
#. /usr/local/bin/set_control.sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

usage() {
    echo "Usage: $0 " >&2
}

set -e

if [ "$#" -eq 0 ]; then
   id_file=/mnt/data/kv_store/bmc_id
   if [ ! -f $id_file ]; then
       #bmc_id file does not exist
       #check if booting from flash1
       id_file1=/mnt/data1/kv_store/bmc_id
       if [ -f $id_file1 ]; then
           id_bmc=$(cat $id_file1)
           echo $id_bmc
       else
           echo "default"
       fi
   else
       bmc_id --get
   fi
else
   usage
   exit 1
fi
