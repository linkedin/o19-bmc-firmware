#!/usr/bin/env python
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

from ctypes import *
import json
import ssl
import socket
import os
from node_api import get_node_api
from node_bmc import get_node_bmc
from node_fruid import get_node_fruid
from node_sensors import get_node_sensors
from node_logs import get_node_logs
from node_sys import get_node_sys
from node_efuses import get_node_efuses
from node_efuse import get_node_efuse
from node_swver import get_node_swver
from node_meminfo import get_node_meminfo
from node_fan import get_node_fan
from node_inet import get_node_inet
from node_psu  import get_node_psu
from node_efuseall import get_node_efuseall
from node_bulkinfo import get_node_bulkinfo
#from node_i2cread import i2c_read
from tree import tree
from pal import *

# Initialize Platform specific Resource Tree
def init_plat_tree():

    # Create /api end point as root node
    r_api = tree("api", data = get_node_api())

    # Add /api/sys to represent BMC system
    r_sys = tree("sys", data = get_node_sys())
    r_api.addChild(r_sys)

    # Add /api/sys/bulkinfo for BMC bulk API
    r_bulkinfo = tree("bulkinfo", data = get_node_bulkinfo())
    r_sys.addChild(r_bulkinfo)

    # Add /api/sys/swVer
    r_temp = tree("swVersion", data = get_node_swver())
    r_sys.addChild(r_temp)

    # Add /api/sys/bmc
    r_temp = tree("bmc", data = get_node_bmc())
    r_sys.addChild(r_temp)

    # Add /api/sys/meminfo
    r_temp = tree("meminfo", data = get_node_meminfo())
    r_sys.addChild(r_temp)

    # Add /api/sys/fan
    r_temp = tree("fan", data = get_node_fan())
    r_sys.addChild(r_temp)

    # Add /api/sys/inet
    r_temp = tree("inet", data = get_node_inet())
    r_sys.addChild(r_temp)

    # Add /api/sys/fruid
    r_temp = tree("fruid", data = get_node_fruid())
    r_sys.addChild(r_temp)

    # Add /api/sys/psu
    for i in range(1, 7):
       r_temp  = tree("psu" + repr(i), data = get_node_psu(i))
       r_sys.addChild(r_temp)

    # Add /api/sys/efuses
    r_efuses = tree("efuses", data = get_node_efuses())
    r_sys.addChild(r_efuses)

    # Add /api/sys/efuses/efuseall
    r_efuseall = tree("efuseall", data = get_node_efuseall())
    r_efuses.addChild(r_efuseall)

    # Add /api/sys/efuses/efuse#
    for i in range(1, 51):
       r_temp  = tree("efuse" + repr(i), data = get_node_efuse(i))
       r_efuses.addChild(r_temp)

    # /api/sys/logs end point
#    r_temp = tree("logs", data = get_node_logs("sys"))
#    r_sys.addChild(r_temp)

    # /api/mezz/sensors end point
#    r_temp = tree("sensors", data = get_node_sensors("nic"))
#    r_mezz.addChild(r_temp)

    return r_api
