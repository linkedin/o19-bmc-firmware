#!/bin/bash
# Copyright 2016-present Flex. All Rights Reserved.
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
# File: remote_cpu_access.sh
#
# Author: Tiju Jacob (Tiju.Jacob@flextronics.com)
#

bmc_ip=192.168.0.1
comE_ip=192.168.0.2

cpu=`cat /proc/cpuinfo 2>/dev/null|awk '/ARM/ {print $3}'`

if [[ $cpu == *"ARM"* ]]
then
PS3='Please enter your choice: '
options=("Read sensor values" "Login BCM shell" "Quit")
select opt in "${options[@]}"
do
    case $opt in
        "Read sensor values")
            echo "Read sensor values";
	    command="sensors";
	    commandPath="/usr/bin/";
	    ssh admin@$comE_ip $commandPath$command
	    ;;
        "Login BCM shell")
	    command="launcher restart";
	    commandPath="/mnt/application/";
	    ssh root@$comE_ip $commandPath$command
            ;;
        "Quit")
            break
            ;;
        *) echo invalid option;;
    esac
#	ssh admin@$comE_ip $commandPath$command
done
else
#options=("Read sensor values" "Read Temperature sensor values" "Get Fan Speed" "Set Fan Speed" "I2C detect" "I2C Get" "I2C Dump" "Reboot" "Command" "Quit")
options=("Read sensor values" "Read Temperature sensor values" "Get Fan Speed" "Set Fan Speed" "I2C detect" "I2C Get" "I2C Dump" "Command" "Quit")
PS3='Please enter your choice: '
select opt in "${options[@]}"
do
    case $opt in
        "Read sensor values")
            echo "Read sensor values";
	    command="sensors";
	    commandPath="/usr/bin/";
	    ;;
        "Read Temperature sensor values")
            echo "Read Temperature sensor values";
	    command="sensors tmp75-*";
	    commandPath="/usr/bin/";
	    ;;
        "Get Fan Speed")
	    command=get_fan_speed.sh
	    commandPath="/usr/local/bin/";
            ;;
        "Set Fan Speed")
    	    echo -n "Enter Fan Unit (1..5) and press [ENTER]: "
	    read unit
	    echo -n "Enter speed percent (0..100) and press [ENTER]: "
	    read speed

	    commandPath="/usr/local/bin/";
	    command="set_fan_speed.sh $speed $unit"
            ;;
        "I2C detect")
	    echo -n "Enter i2c bus number and press [ENTER]: "
	    read i2cbus

	    commandPath="/usr/sbin/";
	    command="i2cdetect -y $i2cbus"
            ;;
        "I2C Get")
	    echo -n "Enter i2c bus number and press [ENTER]: "
	    read i2cbus
	    echo -n "Enter i2c chip address and press [ENTER]: "
	    read chipaddr
	    echo -n "Enter i2c data address and press [ENTER]: "
	    read dataaddr

	    commandPath="/usr/sbin/";
	    command="i2cget -f -y $i2cbus $chipaddr $dataaddr"
            ;;
        "I2C Dump")
	    echo -n "Enter i2c bus number and press [ENTER]: "
	    read i2cbus
	    echo -n "Enter i2c chip address and press [ENTER]: "
	    read chipaddr
	    echo -n "Enter i2c mode and press [ENTER]: "
	    read mode

	    commandPath="/usr/sbin/";
	    command="i2cdump -f -y $i2cbus $chipaddr $mode"
            ;;
#        "Reboot")
#	    commandPath="/sbin/";
#	    command="reboot"
#            ;;
        "Command")
	    echo -n "Enter command and press [ENTER]: "
	    read command
	    echo -n "Enter path of command and press [ENTER]: "
	    read commandPath;
            ;;
        "Quit")
            break
            ;;
        *) echo invalid option;;
    esac
	ssh root@$bmc_ip $commandPath$command
done

fi

exit
