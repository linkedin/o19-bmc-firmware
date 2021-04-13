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
SUMMARY = "Hardware health monitor controller"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://healthmon.c;beginline=4;endline=18;md5=5633eee6ef536ee1730e83ce117a7b26"

SRC_URI = "file://Makefile \
           file://run_healthmon \
           file://setup-healthmon.sh \
           file://healthmon.c \
          "

S = "${WORKDIR}"

binfiles = "                       \
            healthmon              \
           "

CFLAGS_prepend = "-DCONFIG_DELTAPOWER"

DEPENDS_append = "update-rc.d-native"

do_install() {
  bin="${D}/usr/local/bin"
  install -d $bin
  for f in ${binfiles}; do
    install -m 755 $f ${bin}/$f
  done
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/sv
  install -d ${D}${sysconfdir}/sv/healthmon
  install -m 755 run_healthmon ${D}${sysconfdir}/sv/healthmon/run
  install -m 755 ${WORKDIR}/setup-healthmon.sh ${D}/usr/local/bin/setup-healthmon.sh
  install -m 755 setup-healthmon.sh ${D}${sysconfdir}/init.d/setup-healthmon.sh
  update-rc.d -r ${D} setup-healthmon.sh start 91 S .
}

DEPENDS += "libpowershelf libpal "
RDEPENDS_${PN} += "libpowershelf libpal "
FILES_${PN} += "/usr/local"
DEPENDS_append = " update-rc.d-native"

# Inhibit complaints about .debug directories

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
