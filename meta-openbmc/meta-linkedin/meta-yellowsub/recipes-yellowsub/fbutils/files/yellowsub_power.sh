#!/bin/bash
#
# Copyright 2015-present Facebook. All Rights Reserved.
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

. /usr/local/fbpackages/utils/ast-functions

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

LPS_PATH=/mnt/data/power/por/last_state

prog="$0"

usage() {
    echo "Usage: $prog <slot#> <command> [command options]"
    echo
    echo "Commands:"
    echo "  status: Get the current 1S server power status"
    echo
    echo "  on: Power on 1S server if not powered on already"
    echo "    options:"
    echo "      -f: Re-do power on sequence no matter if 1S server has "
    echo "          been powered on or not."
    echo
    echo "  off: Power off 1S server ungracefully"
    echo
    echo
}

do_status() {
    if [ $(is_server_prsnt $slot) == "0" ]; then
      echo  "The given slot is Empty"
      return 0
    fi

    echo -n "1S Server power for slot#$slot is "
    if [ $(yellowsub_is_server_on $slot) -eq 1 ] ; then
        echo "on"
    else
        echo "off"
    fi
    return 0
}

do_on() {
    echo -n "Power on slot#$slot server ..."
#    if [ $force -eq 0 ]; then
        # need to check if 1S Server is on or not
#        if [ $(yellowsub_is_server_on $slot) -eq 1 ]; then
#            echo " Already on. Skip!"
#            return 1
#        fi
#    fi

    # TODO: State the power state change
    echo "on $(date +%s)" > $LPS_PATH

    # first make sure, GPIO is high
    i2cset -f -y $bus 0x77 0x1   # open SoC CPLD I2C
    echo "making power button low"
    i2cset -f -y $bus 0x31 0x30 0x10  # making power button low by writing to CPLD register 0x30
    sleep 1
    echo "making power button high"
    i2cset -f -y $bus 0x31 0x30 0x11  # making power button high by writing to CPLD register 0x30

    # Turn on the power LED
#    /usr/local/bin/power_led.sh $slot on
    echo " Done"
    return 0
}

do_off() {
    echo -n "Power off slot#$slot server ..."

    #TODO: State the power state change
    echo "off $(date +%s)" > $LPS_PATH

    i2cset -f -y $bus 0x77 0x1  # open SoC CPLD I2C
    i2cset -f -y $bus 0x31 0x30 0x10  # making power button low by writing to CPLD register 0x30

    var1=0x00
    while :
    do
        var=`i2cget -f -y $bus 0x31 0x11` # read CPLD register 0x11 and check whether PLTRST_N is low
        if [ $(printf "%d" ${var}) -eq $(printf "%d" ${var1}) ]
    then
        echo "Platform reset low"
        echo "Making power button high"
        i2cset -f -y $bus 0x31 0x30 0x11   # making power button high by writing to CPLD register 0x30
        break
    else
        echo "Platform reset high"
    fi
    done

    # Turn off the power LED
#    /usr/local/bin/power_led.sh $slot off
    echo " Done"
    return 0
}


# Slot1: bus 7, Slot2: bus 0, Slot3: bus 4, Slot4: bus 5
slot=$1

case $slot in
  1)
    bus=7
    ;;
  2)
    bus=0
    ;;
  3)
    bus=4
    ;;
  4)
    bus=5
    ;;
  *)
    bus=1
    ;;
esac

command="$2"
shift
shift

case "$command" in
    status)
        do_status $@
        ;;
    on)
        do_on $@
        ;;
    off)
        do_off $@
        ;;
    *)
        usage
        exit -1
        ;;
esac

exit $?
