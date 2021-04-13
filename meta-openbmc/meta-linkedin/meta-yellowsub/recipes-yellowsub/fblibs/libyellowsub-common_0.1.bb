# Copyright 2015-present Facebook. All Rights Reserved.
SUMMARY = "Yellowsub Common Library"
DESCRIPTION = "library for common Yellowsub information"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://yellowsub_common.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://yellowsub_common \
          "

S = "${WORKDIR}/yellowsub_common"

do_install() {
	  install -d ${D}${libdir}
    install -m 0644 libyellowsub_common.so ${D}${libdir}/libyellowsub_common.so

    install -d ${D}${includedir}
    install -d ${D}${includedir}/facebook
    install -m 0644 yellowsub_common.h ${D}${includedir}/facebook/yellowsub_common.h
}

FILES_${PN} = "${libdir}/libyellowsub_common.so"
FILES_${PN}-dev = "${includedir}/facebook/yellowsub_common.h"
INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
