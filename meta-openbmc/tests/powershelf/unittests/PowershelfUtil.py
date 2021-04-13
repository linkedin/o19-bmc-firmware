#!/usr/bin/env python
#
# Copyright 2018-present Facebook. All Rights Reserved.
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
    commonPath = currentPath.replace('powershelf/unittests', 'common')
    sys.path.insert(0, commonPath)
except Exception:
    pass
import BaseUtil


class PowershelfUtil(BaseUtil.BaseUtil):

    # Fans
    GetFanCmd = '/usr/local/bin/get_fan_speed.sh'
    #set fan not supported for power shelf
    SetFanCmd = '/usr/local/bin/get_fan_speed.sh'
    KillControlCmd = ['/usr/local/bin/wdtcli stop', '/usr/bin/sv stop fscd']
    StartControlCmd = '/usr/bin/sv start fscd'

    # Host Mac
    HostMacCmd = '/usr/local/bin/wedge_us_mac.sh'

    def get_speed(self, info):
        """
        Supports getting fan speed
        """
        data = info.decode('utf-8')
        data = data.split(':')[1].split('\n')
        pwm_str = re.sub("[^0-9]", "", data[0])
        return int(pwm_str)

    def get_fan_test(self, info):
        str = info.decode("utf-8")
        data=str.split('\n')[:4]
        goodRead = ['Fan', 'RPMs:']
        for line in data:
            if len(line) == 0:
                continue
            for word in goodRead:
                if word not in line:
                    return False
            val = line.split(':')[0].split(' ')
            if len(val) != 3:
                return False
        return True

    # EEPROM
    ProductName = ['DeltaPowerShelf']
    EEPROMCmd = '/usr/local/bin/eeprom.sh'

    # Power Cycle eFuses
    PowerCmdOn = '/usr/local/bin/eFuse_on.sh all'
    PowerCmdStatus = '/usr/local/bin/eFuse-util.sh all'
