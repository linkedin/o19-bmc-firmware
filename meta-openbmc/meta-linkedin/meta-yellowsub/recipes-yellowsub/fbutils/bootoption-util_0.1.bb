# Copyright 2018-present Linkedin. All Rights Reserved.
SUMMARY = "Boot options Utility"
DESCRIPTION = "Util for setting boot options on Yellowsub"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://bootoption-util.c;beginline=4;endline=16;md5=b71084aaae952aa25b2f3c810287abd2"

SRC_URI = "file://bootoption-util \
          "

S = "${WORKDIR}/bootoption-util"

do_install() {
	install -d ${D}${bindir}
    install -m 0755 bootoption-util ${D}${bindir}/bootoption-util
}

DEPENDS += " libpal"
RDEPENDS_${PN} += "libpal"

FILES_${PN} = "${bindir}"
