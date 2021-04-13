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

SUMMARY = "Delta Power shelf eFuse utility"
DESCRIPTION = "Util for Delta Power shelf eFuse"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://eFuse-util.c;beginline=4;endline=16;md5=da35978751a9d71b73679307c4d296ec"

SRC_URI = "file://eFuse-util \
          "

S = "${WORKDIR}/eFuse-util"

do_install() {
    localbindir="${D}/usr/local/bin"
    install -d ${localbindir}
    install -m 0755 eFuse-util ${localbindir}/eFuse-util
}

FILES_${PN} += "/usr/local"
DEPENDS += "libpal libgpio libpowershelf"
RDEPENDS_${PN} += "libpal libgpio libpowershelf"

# Inhibit complaints about .debug directories

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
