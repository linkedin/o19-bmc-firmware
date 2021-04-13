SUMMARY = "Tripwire: A system integrity assessment tool (IDS)"
DESCRIPTION = "Open Source Tripwire® software is a security and data \
integrity tool useful for monitoring and alerting on specific file change(s) on a range of systems"
HOMEPAGE="http://sourceforge.net/projects/tripwire"
SECTION = "security Monitor/Admin"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=1c069be8dbbe48e89b580ab4ed86c127"

SRC_URI = "${SOURCEFORGE_MIRROR}/project/${BPN}/${BPN}-src/${BPN}-${PV}/${BPN}-${PV}-src.tar.bz2 \
	   file://tripwire.cron \
	   file://tripwire.sh \
	   file://tripwire.txt \
	   file://twcfg.txt \
	   file://twinstall.sh \
	   file://twpol-yocto.txt \
	   file://tripwire_add_ppc64.patch \
       file://tripwire_add_aarch64_to_configure.patch \
       file://add_armeb_arch.patch \
       "
SRC_URI[md5sum] = "2462ea16fb0b5ae810471011ad2f2dd6"
SRC_URI[sha256sum] = "e09a7bdca9302e704cc62067399e0b584488f825b0e58c82ad6d54cd2e899fad"

S = "${WORKDIR}/tripwire-${PV}-src"

inherit autotools-brokensep update-rc.d

INITSCRIPT_NAME = "tripwire"
INITSCRIPT_PARAMS = "start 40 S ."
TRIPWIRE_HOST = "${HOST_SYS}"
TRIPWIRE_TARGET = "${TARGET_SYS}"

CXXFLAGS += "-fno-strict-aliasing"
EXTRA_OECONF = "--disable-openssl  --enable-static --sysconfdir=/etc/tripwire"

do_configure () {
    #
    # sed bits taken from http://www.linuxfromscratch.org/blfs/view/svn/postlfs/tripwire.html
    #
    sed -i -e 's/!Equal/!this->Equal/' src/cryptlib/algebra.h                &&
    sed -i -e '/stdtwadmin.h/i#include <unistd.h>' src/twadmin/twadmincl.cpp &&
    sed -i -e 's/eArchiveOpen e\([^)]*)\)/throw ( eArchiveOpen\1 )/' \
    -e '/throw e;/d' src/core/archive.cpp                             &&

    oe_runconf
}

do_install () {
    install -d ${D}${libdir} ${D}${datadir} ${D}${base_libdir}
    install -d ${D}${sysconfdir} ${D}${mandir} ${D}${sbindir}
    install -d ${D}${sysconfdir}/${PN}
    install -d ${D}${localstatedir}/lib/${PN} ${D}${localstatedir}/lib/${BPN}/report
    install -d ${D}${mandir}/man4 ${D}${mandir}/man5 ${D}${mandir}/man8
    install -d ${D}${docdir}/${BPN} ${D}${docdir}/${BPN}/templates
    install -d ${D}${sysconfdir}/init.d

    install -m 0755 ${S}/bin/* ${D}${sbindir}
    install -m 0644 ${S}/lib/* ${D}${base_libdir}
    install -m 0644 ${S}/lib/* ${D}${localstatedir}/lib/${PN}
    install -m 0755 ${WORKDIR}/tripwire.cron ${D}${sysconfdir}
    install -m 0755 ${WORKDIR}/tripwire.sh ${D}${sysconfdir}/init.d/tripwire
    install -m 0755 ${WORKDIR}/twinstall.sh ${D}${sysconfdir}/${PN}
    install -m 0644 ${WORKDIR}/twpol-yocto.txt ${D}${sysconfdir}/${PN}/twpol.txt
    install -m 0644 ${WORKDIR}/twcfg.txt ${D}${sysconfdir}/${PN}

    install -m 0644 ${S}/man/man4/* ${D}${mandir}/man4
    install -m 0644 ${S}/man/man5/* ${D}${mandir}/man5
    install -m 0644 ${S}/man/man8/* ${D}${mandir}/man8
    install -m 0644 ${S}/policy/templates/* ${D}${docdir}/${BPN}/templates
    install -m 0644 ${S}/policy/*txt ${D}${docdir}/${BPN}
    install -m 0644 ${S}/COPYING ${D}${docdir}/${BPN}
    install -m 0644 ${S}/TRADEMARK ${D}${docdir}/${BPN}
    install -m 0644 ${WORKDIR}/tripwire.txt ${D}${docdir}/${BPN}
}


FILES_${PN} += "${libdir} ${docdir}/${PN}/*"
FILES_${PN}-dbg += "${sysconfdir}/${PN}/.debug"
FILES_${PN}-staticdev += "${localstatedir}/lib/${PN}/lib*.a"


RDEPENDS_${PN} += " perl nano msmtp cronie"
