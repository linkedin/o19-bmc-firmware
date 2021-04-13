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

def mdio_wr_help():
   result = {
               "USAGE": "mdio_wr MAC PHY write ADDRESS VAL",
               "   MAC": "integer",
               "   PHY": "hex value w/o 0x",
               "   ADDRESS": "hex address w/o 0x",
               "   VAL": "hex value to write",
             }

   return result

# Endpoint for writing to MDIO devices
def mdio_wr(data):
    cmd = "/usr/local/bin/ast-mdio.py"
    mac  = data["mac"]
    phy  = data["phy"]
    addr = data["addr"]
    val  = data["val"]

    xphy = int(phy, 16)
    xphy = hex(xphy)

    xaddr = int(addr, 16)
    xaddr = hex(xaddr)

    xval = int(val, 16)
    xval = hex(xval)

    mdio_cmd = "%s --mac %d --phy %s write %s %s" % (cmd, mac, xphy, xaddr, xval)
    print("MDIO cmd: %s" % mdio_cmd)

    (ret, _) = Popen(mdio_cmd, \
                     shell=True, stdout=PIPE).communicate()

    status = "success"
    result = {
                 "MDIO Write Reg": {"MAC": mac, "PHY":xphy, "Reg Address":xaddr, "Status": status},
             }

    return result


if(__name__=="__main__"):
    print('{}'.format(mdio_wr_help()))


