# Copyright 2019-present LinkedIn. All Rights Reserved.
SUMMARY = "Fan control"
DESCRIPTION = "Daemon controlling fan speed in regards to temperature"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://fan-control.cpp;beginline=4;endline=16;md5=04304dba539f9eec6cf152aa044ced9d"

SRC_URI = "file://Makefile \
           file://setup-fan-control.sh \
           file://run_fan-control \
           file://fan-control.cpp \
           file://fan-control-config.json \
           "

S = "${WORKDIR}"

binfiles = "fan-control"
pkgdir = "fan-control"

DEPENDS += " libgpio libpowershelf nlohmann-json update-rc.d-native"
RDEPENDS_${PN} += " libgpio libpowershelf"

do_install() {
  bin="${D}/usr/local/bin"
  install -d $bin
  for f in ${binfiles}; do
    install -m 755 $f ${bin}/$f
  done
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/sv
  install -d ${D}${sysconfdir}/sv/fan-control
  install -m 644 fan-control-config.json ${D}${sysconfdir}/fan-control-config.json
  install -m 755 run_fan-control ${D}${sysconfdir}/sv/fan-control/run
  install -m 755 ${WORKDIR}/setup-fan-control.sh ${D}/usr/local/bin/setup-fan-control.sh
  install -m 755 setup-fan-control.sh ${D}${sysconfdir}/init.d/setup-fan-control.sh
  update-rc.d -r ${D} setup-fan-control.sh start 70 5 .

}

FILES_${PN} += "/usr/local"
DEPENDS_append = " update-rc.d-native"

# Inhibit complaints about .debug directories

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
