# Copyright 2019-present LinkedIn. All Rights Reserved.
SUMMARY = "Yellowsub Common Library"
DESCRIPTION = "library for von volatile eth config set/get"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://ethcfg.c;beginline=8;endline=20;md5=435eababecd3f367d90616c70e27bdd6"

DEPENDS += "jansson libipmi"

SRC_URI = "file://ethcfg \
          "

S = "${WORKDIR}/ethcfg"

do_install() {
    install -d ${D}${libdir}
    install -m 0644 libethcfg.so ${D}${libdir}/libethcfg.so

    install -d ${D}${includedir}/facebook
    install -m 0644 ethcfg.h ${D}${includedir}/facebook/ethcfg.h
}

FILES_${PN} = "${libdir}/libethcfg.so"
FILES_${PN}-dev = "${includedir}/facebook/ethcfg.h"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"