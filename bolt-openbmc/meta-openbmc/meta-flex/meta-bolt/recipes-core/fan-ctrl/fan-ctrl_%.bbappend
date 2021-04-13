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

DEPENDS_append = " update-rc.d-native"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://Makefile \
            file://fanmon.cpp \
            file://watchdog.h \
            file://watchdog.cpp \
            file://get_fan_speed.sh \
            file://set_fan_speed.sh \
            file://setup-fan.sh \
           "

S = "${WORKDIR}"

binfiles = "		\
    get_fan_speed.sh	\
    set_fan_speed.sh	\
    "

fanbinfiles = "                                    \
    flexmon                                     \
    "

CXXFLAGS_prepend = "-DCONFIG_WEDGE100 "

pkgdir = "flex-mon"

do_install() {
  bin="${D}/usr/local/bin"
  install -d ${bin}
  dst="${D}/usr/local/flexpackages/${pkgdir}"
  install -d ${dst}
  for f in ${fanbinfiles}; do
    install -m 755 $f ${dst}/${f}
    ln -snf ../flexpackages/${pkgdir}/$f ${bin}/$f
  done
  for f in ${binfiles}; do
    install -m 755 $f ${bin}/$f
  done
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -m 755 setup-fan.sh ${D}${sysconfdir}/init.d/setup-fan.sh
  update-rc.d -r ${D} setup-fan.sh start 91 S .
}

FLEXPACKAGEDIR = "${prefix}/local/flexpackages"

RDEPENDS_${PN} = "libflex-eeprom "

DEPENDS += "libflex-eeprom "

FILES_${PN} = "${FLEXPACKAGEDIR}/flex-mon ${prefix}/local/bin /etc"

# Inhibit complaints about .debug directories for the fand binary:
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

