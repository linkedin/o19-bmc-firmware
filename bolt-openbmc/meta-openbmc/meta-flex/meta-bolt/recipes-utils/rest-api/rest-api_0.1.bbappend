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

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://rest-api-1/rest_i2cflush.py \
            file://board_endpoint.py \
            file://boardroutes.py \
            file://board_setup_routes.py \
            file://rest-api-1/rest_sw_vers.py \
            file://rest-api-1/rest_inet.py \
            file://rest-api-1/rest_fan.py \
            file://rest-api-1/rest_cpuinfo.py \
            file://rest-api-1/rest_meminfo.py \
            file://rest-api-1/rest_i2c_rd.py \
            file://rest-api-1/rest_i2c_wr.py \
            file://rest-api-1/rest_mdio_rd.py \
            file://rest-api-1/rest_mdio_wr.py \
            file://rest-api-1/rest_dev_reset.py \
            file://rest-api-1/rest_temp.py \
            file://rest-api-1/rest_platform.py \
            file://rest-api-1/0001-Adding-dual-stack-support-for-HTTP-server.patch \
           "

binfiles1 += "rest_i2cflush.py \
              rest_sw_vers.py \
              rest_inet.py \
              rest_fan.py \
              rest_cpuinfo.py \
              rest_meminfo.py \
              rest_i2c_rd.py \
              rest_i2c_wr.py \
              rest_mdio_rd.py \
              rest_mdio_wr.py \
              rest_dev_reset.py \
              rest_temp.py \
              rest_platform.py \
             "

binfiles += "board_endpoint.py \
             boardroutes.py \
             board_setup_routes.py \
            "
