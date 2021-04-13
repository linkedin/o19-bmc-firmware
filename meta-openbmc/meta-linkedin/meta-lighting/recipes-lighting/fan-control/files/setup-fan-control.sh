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

### BEGIN INIT INFO
# Provides:          setup fan-control
# Required-Start:    board-id
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: start fan-control
### END INIT INFO

echo -n "start fan-control ... "

gpio_val=$(gpio-util.sh --get J4)

if [[ "$gpio_val" == 1 ]]; then  
    echo BMC 1 - not running fan-control
else
    runsv /etc/sv/fan-control > /dev/null 2>&1 &
fi
echo "done."
