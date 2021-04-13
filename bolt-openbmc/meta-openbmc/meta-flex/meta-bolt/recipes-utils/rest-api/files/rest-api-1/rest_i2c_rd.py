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
from subprocess import *

def i2c_rd_help():
   result = {
               "USAGE": "i2c_rd I2CBUS CHIP-ADDRESS DATA-ADDRESS",
               "   I2CBUS": "integer",
               "   CHIP-ADDRESS": "hex address w/0 0x (0x03 - 0x77)",
               "   DATA-ADDRESS": "hex address w/0 0x",
             }

   return result

# Endpoint for reading i2c devices
def i2c_rd(data):

    cmd = "/usr/sbin/i2cget -f -y"
    bus  = data["bus"]
    chip  = data["chip"]
    daddr = data["addr"]

    xchip = int(chip, 16)
    xchip = hex(xchip)
    
    xaddr = int(daddr, 16)
    xaddr = hex(xaddr)
 
    i2c_cmd = "%s %d %s %s" % (cmd, bus, xchip, xaddr)

    (ret, _) = Popen(i2c_cmd, \
                     shell=True, stdout=PIPE).communicate()
    status = ret.rsplit()[-1]

    result = {
                 "I2C Read Reg": {"Bus": bus, "Chip":xchip, "Data Address":xaddr, "Data": status},
             }

    return result


if(__name__=="__main__"):
    print('{}'.format(i2c_rd_help()))


