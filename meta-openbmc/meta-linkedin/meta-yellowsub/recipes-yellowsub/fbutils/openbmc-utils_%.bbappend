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

SRC_URI += " \
           file://disable_watchdog.sh \
           file://us_console.sh \
           file://sol.sh \
           file://reset_usb.sh \
           file://reset_cpu.sh \
           file://setup-gpio.sh \
           file://eth0_mac_fixup.sh \
           file://power-on.sh \
           file://poweroff_cpu.sh \
           file://poweron_cpu.sh \
           file://setup_i2c.sh \
          "

OPENBMC_UTILS_FILES += " \
  disable_watchdog.sh \
  us_console.sh sol.sh \
  poweroff_cpu.sh \
  poweron_cpu.sh \
  reset_cpu.sh \
  setup_i2c.sh \
  "

DEPENDS_append = " update-rc.d-native"

do_install_board() {
  # init
  # setup i2c and sensors
  install -m 755 setup_i2c.sh ${D}${sysconfdir}/init.d/setup_i2c.sh
  update-rc.d -r ${D} setup_i2c.sh start 60 S .
  # create VLAN intf automatically
  install -d ${D}/${sysconfdir}/network/if-up.d

  install -m 0755 ${WORKDIR}/disable_watchdog.sh ${D}${sysconfdir}/init.d/disable_watchdog.sh
  update-rc.d -r ${D} disable_watchdog.sh start 99 2 3 4 5 .
}

do_install_append() {
  do_install_board
}

FILES_${PN} += "${sysconfdir}"
