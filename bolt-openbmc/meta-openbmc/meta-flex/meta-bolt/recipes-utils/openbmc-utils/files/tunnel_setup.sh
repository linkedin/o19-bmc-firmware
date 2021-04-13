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
# File: tunnel_setup.sh
#
# Author: Tiju Jacob (Tiju.Jacob@flextronics.com)
#

bmc_ip=192.168.0.1
comE_ip=192.168.0.2

cpu=`cat /proc/cpuinfo 2>/dev/null|awk '/ARM/ {print $4}'`


if [[ $cpu == *"ARM"* ]]
then
#  ifconfig usb0:1 $bmc_ip up;
  ln -s /mnt/data/etc/ssh /home/root/.ssh

# For fancontrol noise - only during dev phase
#  killall fand
#  /usr/local/bin/watchdog_ctrl.sh off 
#  /usr/local/bin/set_fan_speed.sh 55

#  mv /usr/local/bin/authorized_keys /mnt/data/etc/ssh/
#  mv /usr/local/bin/id_dsa /mnt/data/etc/ssh/
#  mv /usr/local/bin/id_dsa.pub /mnt/data/etc/ssh/
#  mv /usr/local/bin/known_hosts /mnt/data/etc/ssh/
#  cp /mnt/data/etc/remote_cpu_access.sh /usr/local/bin/
#  cp /mnt/data/etc/wemanuf /usr/local/bin/
#  ssh-keygen -t dsa -f ~/.ssh/id_dsa -q -P ""
#  ssh-keygen -t dsa -N "" -f /home/root/.ssh/id_dsa.pub
#  echo -e 'y\n'|ssh-keygen -q -t dsa -N "" -f /home/root/.ssh/id_dsa
#  cat .ssh/id_dsa.pub | ssh admin@192.168.0.2 'cat >> ~/.ssh/authorized_keys'
#  cat .ssh/ssh_host_dsa_key.pub | ssh admin@192.168.0.2 'cat >> ~/.ssh/authorized_keys'
else
#  dhclient
#  sh install_mods.sh
#  ifconfig usb0:1 $comE_ip up;
#  echo -e 'y\n'|ssh-keygen -q -t dsa -N "" -f ~/.ssh/id_dsa
#  cat .ssh/id_dsa.pub | ssh root@192.168.0.1 'cat >> ~/.ssh/authorized_keys'
   echo
fi
