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

SUMMARY = "Delta Power shelf PSU Utilities"
DESCRIPTION = "Util for Delta Power shelf PSU"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;beginline=4;endline=16;md5=09782fd1da3eecbb4465f089e2507b82"

DEPENDS_append = "openssl update-rc.d-native"

SRC_URI = "file://get_fan_speed.sh \
           file://get_temp.sh \
           file://psu_on.sh \
           file://psu_off.sh \
           file://eFuse_on.sh \
           file://eFuse_off.sh \
           file://eFuse_test.sh \
           file://get_sw_version.sh \
           file://eFuse-util.sh \
           file://psu-util.sh \
           file://psu-util.sh \
           file://fan-util.sh \
           file://eeprom.sh \
           file://psu-eeprom.sh \
           file://gpio-util.sh \
           file://power-lun.sh \
           file://led_ctrl.sh \
           file://reset_peer_bmc.sh \
           file://tests.sh \
           file://COPYING \
"
pkgdir = "utils"

S = "${WORKDIR}"

LIUTILS_BIN_FILES += "get_fan_speed.sh \
                      get_temp.sh \
                      psu_on.sh \
                      psu_off.sh \
                      eFuse_on.sh \
                      eFuse_off.sh \
                      eFuse-util.sh \
                      eFuse_test.sh \
                      get_sw_version.sh \
                      psu-util.sh \
                      fan-util.sh \
                      psu-eeprom.sh \
                      eeprom.sh \
                      gpio-util.sh \
                      power-lun.sh \
                      led_ctrl.sh \
                      reset_peer_bmc.sh \
                      tests.sh \
"

do_install() {
    dst="${D}/usr/local/fbpackages/${pkgdir}"
    install -d $dst
#    install -m 644 ast-functions ${dst}/ast-functions
    localbindir="${D}/usr/local/bin"
    install -d ${localbindir}
    for f in ${LIUTILS_BIN_FILES}; do
        install -m 0755 $f ${localbindir}/$f
    done
}

FILES_${PN} += "/usr/local"

# Inhibit complaints about .debug directories

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
