# Copyright 2015-present Facebook. All Rights Reserved.
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
SRC_URI += "file://plat_tree.py \
           file://node_api.py \
           file://node_bmc.py \
           file://node_fruid.py \
           file://node_sensors.py \
           file://node_logs.py \
           file://node_config.py \
           file://node_sys.py \
           file://node_swver.py \
           file://node_cpuinfo.py \
           file://node_meminfo.py \
           file://node_fan.py \
           file://node_inet.py \
           file://node_psu.py \
           file://node_efuses.py \
           file://node_efuse.py \
           file://node_efuseall.py \
           file://node_bulk.py \
           file://node_bulkinfo.py \
          "
