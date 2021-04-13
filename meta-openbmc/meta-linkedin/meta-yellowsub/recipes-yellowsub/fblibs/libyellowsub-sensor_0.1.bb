# Copyright 2015-present Facebook. All Rights Reserved.
SUMMARY = "Yellowsub Sensor Library"
DESCRIPTION = "library for reading various sensors"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://yellowsub_sensor.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"


SRC_URI = "file://yellowsub_sensor \
          "
DEPENDS =+ " libipmi libipmb libbic libyellowsub-common obmc-i2c obmc-pal "

S = "${WORKDIR}/yellowsub_sensor"

do_install() {
	  install -d ${D}${libdir}
    install -m 0644 libyellowsub_sensor.so ${D}${libdir}/libyellowsub_sensor.so

    install -d ${D}${includedir}/facebook
    install -m 0644 yellowsub_sensor.h ${D}${includedir}/facebook/yellowsub_sensor.h
}

FILES_${PN} = "${libdir}/libyellowsub_sensor.so"
FILES_${PN}-dev = "${includedir}/facebook/yellowsub_sensor.h"
INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
