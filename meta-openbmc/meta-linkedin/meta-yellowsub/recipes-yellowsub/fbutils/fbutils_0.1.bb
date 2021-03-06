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
SUMMARY = "Utilities"
DESCRIPTION = "Various utilities"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=eb723b61539feef013de476e68b5c50a"

SRC_URI = "file://ast-functions \
           file://sol-util \
           file://power-on.sh \
           file://fw_env_config.sh \
           file://get_sw_version.sh \
           file://dhclient.sh \
           file://COPYING \
          "

pkgdir = "utils"

S = "${WORKDIR}"

binfiles = "ast-functions sol-util power-on.sh get_sw_version.sh dhclient.sh"

DEPENDS_append = "update-rc.d-native"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  install -d $dst
  install -m 644 ast-functions ${dst}/ast-functions
  localbindir="${D}/usr/local/bin"
  install -d ${localbindir}
  for f in ${binfiles}; do
      install -m 755 $f ${dst}/${f}
      ln -s ../fbpackages/${pkgdir}/${f} ${localbindir}/${f}
  done

  # init
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -m 0755 ${WORKDIR}/fw_env_config.sh ${D}${sysconfdir}/init.d/fw_env_config.sh
  update-rc.d -r ${D} fw_env_config.sh start 05 S .
  install -m 755 power-on.sh ${D}${sysconfdir}/init.d/power-on.sh
  update-rc.d -r ${D} power-on.sh start 70 5 .
  install -m 755 dhclient.sh ${D}${sysconfdir}/init.d/dhclient.sh
  update-rc.d -r ${D} dhclient.sh start 70 5 .
}

RDEPENDS_${PN} += "bash"
FILES_${PN} += "/usr/local ${sysconfdir}"
