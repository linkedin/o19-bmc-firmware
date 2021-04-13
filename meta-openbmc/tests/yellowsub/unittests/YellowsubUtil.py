#!/usr/bin/env python
#
# Copyright 2018-present Linkedin. All Rights Reserved.
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

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import sys
import os
import re
try:
    currentPath = os.getcwd()
    commonPath = currentPath.replace('yellowsub/unittests', 'common')
    sys.path.insert(0, commonPath)
except Exception:
    pass
import BaseUtil

class YellowsubUtil(BaseUtil.BaseUtil):

    # Sensors
    SensorCmd = '/usr/local/bin/sensor-util all'

    # Fans
    GetFanCmd = '/usr/local/bin/fan-util --get'
    SetFanCmd = '/usr/local/bin/fan-util --set 35'
    KillControlCmd = ['/usr/bin/sv stop fscd']
    StartControlCmd = '/usr/bin/sv start fscd'

    # Host Mac
    HostMacCmd = '/usr/local/bin/wedge_us_mac.sh'

    def get_speed(self, info):
        """
        Supports getting fan speed
        """
        info = info.decode('utf-8')

        for sdata in info.split('\n'):
            tdata = sdata.split(':', 1)
            if (len(tdata) < 2):
                continue

            data = tdata[1].split(' ')
            #print("get_speed info: {}".format(data))
            pwm_str = re.sub("[^0-9]", "", data[1])
            print("{}: {}".format(tdata[0], tdata[1]))

            if int(pwm_str) == 0:
                return 1
        return 0

    def get_fan_test(self, info):
        info = info.decode('utf-8')
        info = info.split('\n')
        goodRead = ['Fan', 'RPM', '%']
        for line in info:
            if len(line) == 0:
                continue
            for word in goodRead:
                if word not in line:
                    return False
            val = line.split(':')[1].lstrip().split(' ')
            if len(val) != 3:
                return False
        return True

    # EEPROM
    ProductName = ['YELLOW SUBMARINE']
    EEPROMCmd = '/usr/local/bin/weutil'

    # Power Cycle CPUs
    PowerCmdOn = '/usr/local/bin/power-util slot1 on'
    PowerCmdStatus = '/usr/local/bin/power-util slot1 status'
    PowerCmdOff = '/usr/local/bin/power-util slot1 off'
    PowerCmdReset = '/usr/local/bin/power-util slot1 reset'

    # sol
    solCmd = '/usr/local/bin/sol.sh slot1'
    solCloseConnection = ['\r', 'CTRL-x']
    def solConnectionClosed(self, info):
        if 'Exit from SOL sessio' in info:
            return True
        else:
            return False
