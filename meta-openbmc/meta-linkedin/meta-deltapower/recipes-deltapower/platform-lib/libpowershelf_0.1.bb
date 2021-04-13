# Copyright 2019-present Linkedin. All Rights Reserved.
SUMMARY = "Platform Abstraction Library"
DESCRIPTION = "library for communicating with Platform"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://powershelf_efuse.c;beginline=8;endline=20;md5=b6724ae95f90ead7457f9e839aa4cd52"

SRC_URI = "file://powershelf \
          "

S = "${WORKDIR}/powershelf"

do_install() {
	install -d ${D}${libdir}
    install -m 0644 libpowershelf.so ${D}${libdir}/libpowershelf.so

    install -d ${D}${includedir}/facebook
    install -m 0644 powershelf.h ${D}${includedir}/facebook/powershelf.h
}

DEPENDS += "obmc-i2c libgpio"
RDEPENDS_${PN} += "libpal libgpio"
INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

FILES_${PN} = "${libdir}/libpowershelf.so"
FILES_${PN}-dev = "${includedir}/facebook/powershelf.h"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
