# Copyright 2015-present Facebook. All Rights Reserved.
SUMMARY = "Platform Abstraction Library"
DESCRIPTION = "library for communicating with Platform"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://pal.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://pal \
          "
DEPENDS += "libbic libyellowsub-common libyellowsub-fruid libyellowsub-sensor libkv libedb libipmi libipmb obmc-pal libpam "

S = "${WORKDIR}/pal"

do_install() {
	  install -d ${D}${libdir}
    install -m 0644 libpal.so ${D}${libdir}/libpal.so

    install -d ${D}${includedir}/openbmc
    install -m 0644 pal.h ${D}${includedir}/openbmc/pal.h
}

FILES_${PN} = "${libdir}/libpal.so"
FILES_${PN}-dev = "${includedir}/openbmc/pal.h"

RDEPENDS_${PN} += " libyellowsub-common libyellowsub-fruid libkv libedb libpam"
INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
