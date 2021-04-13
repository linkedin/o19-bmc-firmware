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

def i2c_wr_help():
   result = {
               "USAGE": "i2c_wr BUS CHIP ADDRESS VAL",
               "   BUS": "integer",
               "   CHIP": "hex value w/o 0x",
               "   ADDRESS": "hex address w/o 0x",
               "   VAL": "hex value to write",
             }

   return result

# Endpoint for writing to MDIO devices
def i2c_wr(data):
    cmd  = "/usr/sbin/i2cset"
    bus  = data["bus"]
    chip = data["chip"]
    addr = data["addr"]
    val  = data["val"]

    xchip = int(chip, 16)
    xchip = hex(xchip)

    xaddr = int(addr, 16)
    xaddr = hex(xaddr)

    xval = int(val, 16)
    xval = hex(xval)

    i2c_cmd = "%s -f -y %d %s %s %s" % (cmd, bus, xchip, xaddr, xval)
    print("MDIO cmd: %s" % i2c_cmd)

    (ret, _) = Popen(i2c_cmd, \
                     shell=True, stdout=PIPE).communicate()

    status = "success"
    result = {
                 "I2C Write Reg": {"Bus": bus, "Chip":xchip, "Reg Address":xaddr, "Status": status},
             }

    return result


if(__name__=="__main__"):
    print('{}'.format(i2c_wr_help()))

