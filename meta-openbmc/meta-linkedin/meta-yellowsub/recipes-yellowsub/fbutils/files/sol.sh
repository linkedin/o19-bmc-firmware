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
#

CONSOLE_SH=/usr/local/bin/us_console.sh
FILE=/etc/us_pseudo_tty
TTY=/dev/ttyS0

if [ -a $FILE ]
  then
    read -r TTY<$FILE
fi

if [ "$1" == "slot1" ] || [ "$1" == "slot2" ] || [ "$1" == "slot3" ] || [ "$1" == "slot4" ]
then
  SLOT=$1
else
  echo "Usage: sol-util [ slot1 | slot2 | slot3 | slot4 ]"
  exit -1
fi

$CONSOLE_SH connect

if [ "$1" == "slot1" ]; then
   echo "sol CPU1"
   VALUE=0x00
elif [ "$1" == "slot2" ]; then
   echo "sol CPU2"
   VALUE=0x01
elif [ "$1" == "slot3" ]; then
   echo "sol CPU3"
   VALUE=0x02
elif [ "$1" == "slot4" ]; then
   echo "sol CPU4"
   VALUE=0x03
fi

   i2cset -f -y 12 0x31 0x25 $VALUE

#$CONSOLE_SH connect

echo "You are in SOL session."
echo "Use ctrl-x to quit."
echo "-----------------------"
echo

trap '"$CONSOLE_SH" disconnect' INT TERM QUIT EXIT

#/usr/bin/microcom -s 9600 $TTY
if [ "$2" == "--bic" ]; then
   echo " --bic"
   bic-util slot1 --set_gpio 29 0
   /usr/bin/microcom -s 115200 $TTY
else
   bic-util slot1 --set_gpio 29 1
   /usr/bin/microcom -s 9600 $TTY
fi

echo
echo
echo "-----------------------"
echo "Exit from SOL session."
