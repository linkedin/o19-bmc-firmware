DESCRIPTION = "This package contains the IP.pm module with friends."

SECTION = "libs"
LICENSE = "Artistic-1.0 | GPL-1.0+"

LIC_FILES_CHKSUM = "file://Copying;md5=cde580764a0fbc0f02fafde4c65d6227"

DEPENDS += "perl"

SRC_URI = "http://search.cpan.org/CPAN/authors/id/M/MI/MIKER/NetAddr-IP-${PV}.tar.gz"

SRC_URI[md5sum] = "7721135fcea390327f75421a6b701144"
SRC_URI[sha256sum] = "96739afc484eca1597c4f4b520864b342169c8fdeef486778511e5a1527ba4e7"

S = "${WORKDIR}/NetAddr-IP-${PV}"

EXTRA_CPANFLAGS = "EXPATLIBPATH=${STAGING_LIBDIR} EXPATINCPATH=${STAGING_INCDIR}"

inherit cpan

do_compile() {
	cpan_do_compile
}
BBCLASSEXTEND = "native"

PNBLACKLIST[libnetaddr-ip-perl] = "BROKEN: doesn't build | make[2]: *** No rule to make target `config.h', needed by `Util.c'.  Stop."
