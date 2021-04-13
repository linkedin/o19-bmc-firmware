#!/bin/bash
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

### BEGIN INIT INFO
# Provides:          gpio-setup
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description:  Set up GPIO pins as appropriate
### END INIT INFO

# This file contains definitions for the GPIO pins that were not otherwise
# defined in other files.  We should probably move some more of the
# definitions to this file at some point.

# The commented-out sections are generally already defined elsewhere,
# and defining them twice generates errors.

# The exception to this is the definition of the GPIO H0, H1, and H2
# pins, which seem to adversely affect the rebooting of the system.
# When defined, the system doesn't reboot cleanly.  We're still
# investigating this.

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin
. /usr/local/bin/openbmc-utils.sh

#enable flash rom CS1
devmem_set_bit $(scu_addr 88) 24

# Set up to read the board revision pins, Y1(193), Y2(194)
devmem_set_bit $(scu_addr 70) 19
devmem_clear_bit $(scu_addr a4) 9
devmem_clear_bit $(scu_addr a4) 10
gpio_export Y1
gpio_export Y2

# SCC_STBY_PWR_EN: F4 (44), PS
# To use GPIOF4, LHCR[0], SCU90[30], and SCU80[28] must be 0
#devmem_clear_bit $(lpc_addr A0) 0
#devmem_clear_bit $(scu_addr 90) 30
devmem_clear_bit $(scu_addr 80) 28
gpio_export F4

#set global power on (MSERV_POWERUP)
gpio_set F4 0

# This is GPIO G5(53)
devmem_clear_bit $(scu_addr 70) 23
devmem_clear_bit $(scu_addr 84) 5
echo 53 > /sys/class/gpio/export

# GPIOJ2(74)
devmem_clear_bit $(scu_addr 84) 10
gpio_export J2

# GPIOJ4(76)
devmem_clear_bit $(scu_addr 84) 12
gpio_export J4

# GPIOJ4(78)
devmem_clear_bit $(scu_addr 84) 14
gpio_export J6
gpio_set J6 0

#GPIOF1 (41) GPIOF2
devmem_clear_bit $(scu_addr 80) 25
devmem_clear_bit $(scu_addr 80) 26
devmem_clear_bit $(scu_addr a4) 12
devmem_clear_bit $(scu_addr a4) 13
gpio_export F1
gpio_export F2

#DualColor LED – RED:      GPIOP2
#To use GPIOP2:  set SCU90[5:4]=2 & SCU88[18]=1
#DualColor LED – GREEN:        GPIOP0
#TO use GPIOP0:  set SCU90[5:4]=2 & SCU88[16]=1
devmem_set_bit $(scu_addr 90) 4
devmem_set_bit $(scu_addr 90) 5
devmem_set_bit $(scu_addr 88) 18
devmem_set_bit $(scu_addr 88) 16
gpio_export P2
gpio_export P0
#set LED to green
gpio_set P0 1
gpio_set P2 0

#Cross-reset BMC:   GPIOC0
# To use GPIOC0 clear SCU90[0] SCU90[23]
devmem_clear_bit $(scu_addr 90) 0
devmem_clear_bit $(scu_addr 90) 23
gpio_export C0

#PS_ON/L PSU1…6
#GPIO: F0, O1, P6, G4, M0, M1
# To use GPIOP6(126), SCU88[22] must be add
devmem_clear_bit $(scu_addr 88) 22
gpio_export P6

# To use GPIOO1(113), SCU88[9] must be 0
devmem_clear_bit $(scu_addr 88) 9

gpio_export O1

# To use GPIOF0, SCU90[30], and SCU80[24] must be 0
devmem_clear_bit $(scu_addr 90) 30
devmem_clear_bit $(scu_addr 80) 24

gpio_export F0

# To use GPIOG4 SCU2C[1] and SCU84[4] must be 0
devmem_clear_bit $(scu_addr 2C) 1
devmem_clear_bit $(scu_addr 84) 4
gpio_export G4

# GPIOM0|M1
# SCU84[24|25] must be 0 to disable UART2 function pins.
devmem_clear_bit $(scu_addr 84) 24
devmem_clear_bit $(scu_addr 84) 25

gpio_export M0
gpio_export M1

#AC_OK_PSU1…6
#GPIO: F5, F6, F7, L5, L6, L7
# To use GPIOF5, LHCR[0], SCU90[30], and SCU80[29] must be 0
#devmem_clear_bit $(lpc_addr A0) 0
#devmem_clear_bit $(lpc_addr A4) 15
#devmem_clear_bit $(scu_addr 90) 30
devmem_clear_bit $(scu_addr 80) 29

gpio_export F5

# To use GPIOF6, LHCR[0], and SCU80[30] must be 0
#devmem_clear_bit $(lpc_addr A0) 0
devmem_clear_bit $(scu_addr 80) 30

gpio_export F6

# To use GPIOF7, LHCR[0], SCU90[30], and SCU80[31] must be 0
#devmem_clear_bit $(lpc_addr A0) 0
devmem_clear_bit $(scu_addr 90) 30
devmem_clear_bit $(scu_addr 80) 31

gpio_export F7

# use GPIOL5, GPIOL6 and GPIOL7
devmem_clear_bit $(scu_addr 84) 21
devmem_clear_bit $(scu_addr 84) 22
devmem_clear_bit $(scu_addr 84) 23
gpio_export L5
gpio_export L6
gpio_export L7

#Heartbeat Out GPIOC1, SCU90[0] and SCU90[23] must be 0
#Heartbeat In GPIOY0, SCUA4[8] must be 0
devmem_clear_bit $(scu_addr 90) 0
devmem_clear_bit $(scu_addr 90) 23
devmem_clear_bit $(scu_addr A4) 8
gpio_export C1
gpio_export Y0

#set LED to green
gpio_set C0 0
gpio_set Y2 1
