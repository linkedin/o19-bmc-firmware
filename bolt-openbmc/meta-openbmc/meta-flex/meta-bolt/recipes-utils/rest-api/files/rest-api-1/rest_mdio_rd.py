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

def mdio_rd_help():
   result = {
               "USAGE": "mdio_rd MAC PHY ADDRESS",
               "   MAC": "integer",
               "   PHY": "hex numeral",
               "   ADDRESS": "hex numeral",
             }

   return result

# Endpoint for reading MDIO devices
def mdio_rd(data):

    cmd = "/usr/local/bin/ast-mdio.py"
    mac  = data["mac"]
    phy  = data["phy"]
    addr = data["addr"]

    xphy = int(phy, 16)
    xphy = hex(xphy)
    
    xaddr = int(addr, 16)
    xaddr = hex(xaddr)
 
    mdio_cmd = "%s -m %d -p %s read %s" % (cmd, mac, xphy, xaddr)
    (ret, _) = Popen(mdio_cmd, \
                     shell=True, stdout=PIPE).communicate()
    ret = ret.decode()
    rval = ret.split(": ")[-1]

    result = {
                 "MDIO Read Reg": {"MAC": mac, "PHY":xphy, "Data Address":xaddr, "Data": rval},
             }

    return result


if(__name__=="__main__"):
    print('{}'.format(mdio_rd_help()))


