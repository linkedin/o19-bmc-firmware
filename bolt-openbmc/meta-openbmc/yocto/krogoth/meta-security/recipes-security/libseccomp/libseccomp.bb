SUMMARY = "interface to seccomp filtering mechanism"
DESCRIPTION = "The libseccomp library provides and easy to use, platform independent,interface to the Linux Kernel's syscall filtering mechanism: seccomp."
SECTION = "security"
LICENSE = "LGPL-2.1"
LIC_FILES_CHKSUM = "file://LICENSE;beginline=0;endline=1;md5=8eac08d22113880357ceb8e7c37f989f"

SRCREV = "7f3ae6e6a12390bd38f0787b242f60c47ad076c3"

PV = "2.2.3+git${SRCPV}"

SRC_URI = "git://github.com/seccomp/libseccomp.git \
        "

S = "${WORKDIR}/git"

inherit autotools-brokensep pkgconfig

PACKAGECONFIG ??= ""
PACKAGECONFIG[python] = "--enable-python, --disable-python, python"

do_compile_append() {
    oe_runmake -C tests check-build
}

do_install_append() {
    install -d ${D}/${libdir}/${PN}/tests
    install -d ${D}/${libdir}/${PN}/tools
    for file in $(find tests/* -executable -type f); do
        install -m 744 ${S}/${file} ${D}/${libdir}/${PN}/tests
    done
    for file in $(find tests/*.tests -type f); do
        install -m 744 ${S}/${file} ${D}/${libdir}/${PN}/tests
    done
    for file in $(find tools/* -executable -type f); do
        install -m 744 ${S}/${file} ${D}/${libdir}/${PN}/tools
    done
}

PACKAGES += " ${PN}-tests"
FILES_${PN} = "${bindir} ${libdir}/${PN}.so*"
FILES_${PN}-tests = "${libdir}/${PN}/tools ${libdir}/${PN}/tests"
FILES_${PN}-dbg += "${libdir}/${PN}/tests/.debug/* ${libdir}/${PN}/tools/.debug"

RDEPENDS_${PN} = "bash"
RDEPENDS_${PN}-tests = "bash"
