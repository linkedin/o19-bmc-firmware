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
from aiohttp import web
from rest_utils import get_endpoints
import rest_usb2i2c_reset
import rest_i2cflush
import rest_sw_vers
import rest_inet
import rest_fan
import rest_cpuinfo
import rest_meminfo
import rest_i2c_rd
import rest_mdio_rd
import rest_temp
import rest_platform
from rest_utils import dumps_bytestr, get_endpoints


class boardApp_Handler:

    # Disable the endpoint in BMC until we root cause cp2112 issues.
    # Handler to reset usb-to-i2c
    #async def rest_usb2i2c_reset_hdl(self,request):
        #return rest_usb2i2c_reset.set_usb2i2c()

    async def rest_sw_vers_hdl(self,request):
        return web.json_response(rest_sw_vers.get_vers())

    async def rest_inet_hdl(self,request):
        return web.json_response(rest_inet.get_inet(), dumps=dumps_bytestr)

    async def rest_fan_hdl(self,request):
        return web.json_response(rest_fan.get_fan(), dumps=dumps_bytestr)

    async def rest_cpuinfo_hdl(self,request):
        return web.json_response(rest_cpuinfo.cpuinfo(), dumps=dumps_bytestr)

    async def rest_meminfo_hdl(self,request):
        return web.json_response(rest_meminfo.meminfo(), dumps=dumps_bytestr)

    async def rest_i2c_rd_hdl(self,request):
        data = await request.json()
        return web.json_response(rest_i2c_rd.i2c_rd(data), dumps=dumps_bytestr)
    async def rest_onie_hdl(self,request):
        return web.FileResponse('/sys/bus/i2c/devices/4-0050/eeprom')

    async def rest_mdio_rd_hdl(self,request):
        data = await request.json()
        return web.json_response(rest_mdio_rd.mdio_rd(data), dumps=dumps_bytestr)
    async def rest_temp_hdl(self,request):
        return web.json_response(rest_temp.get_temp(), dumps=dumps_bytestr)

    async def rest_platform_hdl(self,request):
        return web.json_response(rest_platform.get_platform(), dumps=dumps_bytestr)

    async def rest_mdio_rd_hlp_hdl(self,request):
        return web.json_response(rest_mdio_rd.mdio_rd_help(), dumps=dumps_bytestr)
    async def rest_i2c_rd_hlp_hdl(self,request):
        return web.json_response(rest_i2c_rd.i2c_rd_help(), dumps=dumps_bytestr)
