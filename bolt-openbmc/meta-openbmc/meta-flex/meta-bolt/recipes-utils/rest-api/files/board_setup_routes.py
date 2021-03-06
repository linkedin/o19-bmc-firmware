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
from board_endpoint import boardApp_Handler
from boardroutes import *

def setup_board_routes(app):
    bhandler = boardApp_Handler()
    app.router.add_get(board_routes[0], bhandler.rest_sw_vers_hdl)
    app.router.add_get(board_routes[1], bhandler.rest_inet_hdl)
    app.router.add_get(board_routes[2], bhandler.rest_fan_hdl)
    app.router.add_get(board_routes[3], bhandler.rest_cpuinfo_hdl)
    app.router.add_get(board_routes[4], bhandler.rest_meminfo_hdl)
    app.router.add_post(board_routes[5], bhandler.rest_i2c_rd_hdl)
    app.router.add_get(board_routes[6], bhandler.rest_onie_hdl)
    app.router.add_post(board_routes[7], bhandler.rest_mdio_rd_hdl)
    app.router.add_get(board_routes[8], bhandler.rest_temp_hdl)
    app.router.add_get(board_routes[9], bhandler.rest_platform_hdl)
    app.router.add_get(board_routes[10], bhandler.rest_mdio_rd_hlp_hdl)
    app.router.add_get(board_routes[11], bhandler.rest_i2c_rd_hlp_hdl)
