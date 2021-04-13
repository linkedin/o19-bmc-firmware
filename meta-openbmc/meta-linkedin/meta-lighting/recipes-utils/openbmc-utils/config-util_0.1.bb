# Copyright 2019-present LinkedIn. All Rights Reserved.
SUMMARY = "Config update"
DESCRIPTION = "Utility to update configuration of the machine"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://config-util.cpp;beginline=4;endline=16;md5=04304dba539f9eec6cf152aa044ced9d"

SRC_URI = "file://Makefile \
           file://config-util.cpp \
           "

S = "${WORKDIR}"

binfiles = "config-util"
pkgdir = "config-util"

DEPENDS += " nlohmann-json"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  install -m 755 config-util ${dst}/config-util
  ln -snf ../fbpackages/${pkgdir}/config-util ${bin}/config-util

}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES_${PN} = "${FBPACKAGEDIR}/config-util ${prefix}/local/bin"