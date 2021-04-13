SUMMARY = "network auditing tool"
DESCRIPTION = "Nmap ("Network Mapper") is a free and open source (license) utility for network discovery and security auditing.\nGui support via appending to IMAGE_FEATURES x11-base in local.conf"
SECTION = "security"
LICENSE = "GPL-2.0"

LIC_FILES_CHKSUM = "file://COPYING;beginline=7;endline=12;md5=51f7052ac85aaf1a2127f7803de1261e"

SRC_URI = "http://nmap.org/dist/${BP}.tar.bz2"

SRC_URI[md5sum] = "0764f4dabe7cccda3c49fc3990b62a8a"
SRC_URI[sha256sum] = "63df082a87c95a189865d37304357405160fc6333addcf5b84204c95e0539b04"

inherit autotools-brokensep pkgconfig python-dir distro_features_check

PACKAGECONFIG ?= "ncat nping ndiff pcap"
PACKAGECONFIG += " ${@bb.utils.contains('IMAGE_FEATURES', 'x11-base', 'zenmap', '', d)}"

PACKAGECONFIG[pcap] = "--with-pcap=linux, --without-pcap, libpcap, libpcap"
PACKAGECONFIG[ssl] = "--with-openssl=${STAGING_LIBDIR}/.., --without-openssl, openssl, openssl"

#disable/enable packages
PACKAGECONFIG[nping] = ",--without-nping,"
PACKAGECONFIG[ncat] = ",--without-ncat,"
PACKAGECONFIG[ndiff] = ",--without-ndiff,"

PACKAGECONFIG[pcre] = "--with-libpcre=${STAGING_LIBDIR}/.., --with-libpcre=included, libpre"

#Add gui
PACKAGECONFIG[zenmap] = "--with-zenmap, --without-zenmap, gtk+ python-core python-codecs python-io python-logging python-unittest python-xml python-netclient python-doctest python-subprocess python-pygtk, python-core python-codecs python-io python-logging python-netclient python-xml python-unittest python-doctest python-subprocess  python-pygtk gtk+"

EXTRA_OECONF = "--with-libdnet=included --with-liblinear=included --without-subversion --with-liblua=included"

do_configure() {
    # strip hard coded python2#
    sed -i -e 's=python2\.*=python=g'  ${S}/configure.ac
    sed -i -e 's=python2\.*=python=g'  ${S}/configure
    autoconf
    oe_runconf
}

do_install_append () {
   # remove python dir, its not used or installed
   rm -fr ${D}/${libdir}
}

PACKAGES += "${@bb.utils.contains('PACKAGECONFIG', 'zenmap', '${PN}-zenmap', '', d)}"

FILES_${PN}-zenmap = "${@bb.utils.contains("PACKAGECONFIG", "zenmap", "${bindir}/*zenmap ${bindir}/xnmap ${datadir}/applications/*  ${bindir}/nmapfe ${datadir}/zenmap/* ${PYTHON_SITEPACKAGES_DIR}/radialnet/* ${PYTHON_SITEPACKAGES_DIR}/zenmap*", "", d)}"

RDEPENDS_${PN} = "python"
RDEPENDS_${PN}-zenmap = "nmap"
