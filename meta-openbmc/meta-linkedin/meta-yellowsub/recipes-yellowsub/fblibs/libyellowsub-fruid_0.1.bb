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

SUMMARY = "Yellowsub Fruid Library"
DESCRIPTION = "library for reading all yellowsub fruids"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://yellowsub_fruid.c;beginline=6;endline=18;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://yellowsub_fruid \
          "

DEPENDS += " libyellowsub-common "

S = "${WORKDIR}/yellowsub_fruid"

do_install() {
    install -d ${D}${libdir}
    install -m 0644 libyellowsub_fruid.so ${D}${libdir}/libyellowsub_fruid.so

    install -d ${D}${includedir}
    install -d ${D}${includedir}/facebook
    install -m 0644 yellowsub_fruid.h ${D}${includedir}/facebook/yellowsub_fruid.h
}

DEPENDS =+ " libipmi libipmb libbic libyellowsub-common obmc-i2c obmc-pal "
RDEPENDS_${PN} += " libyellowsub-common "

FILES_${PN} = "${libdir}/libyellowsub_fruid.so"
FILES_${PN}-dev = "${includedir}/facebook/yellowsub_fruid.h"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
