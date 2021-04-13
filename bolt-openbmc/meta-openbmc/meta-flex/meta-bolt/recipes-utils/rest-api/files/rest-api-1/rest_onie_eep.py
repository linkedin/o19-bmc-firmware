#!/usr/bin/env python3
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

import os
import subprocess
import binascii
from subprocess import Popen, PIPE

def get_data(loc):
#    cmd = "cat /sys/bus/i2c/devices/4-0050/eeprom"
#    data = Popen(cmd, shell=True, stdout=PIPE).stdout.read()
#    return data

    f = open(loc, 'rb')
    for chunk in f.read(167):
        yield chunk
        break
    f.close()
#    with open(loc, 'rb') as f:
#        return f.read(148)

#        data = f.read(128)
#        return data.decode('unicode_escape')

#        for chunk in iter(lambda: f.read(148), b''):
#            return chunk
#            return binascii.hexlify(chunk)


# Endpoint for retreiving ONIE EEPROM data
def onie_eep():
    path = '/sys/bus/i2c/devices/4-0050/eeprom'

#    eep = "test"
#    return eep
    eep = get_data(path)
    return { eep}

if(__name__=="__main__"):
    print('{}'.format(onie_eep()))
