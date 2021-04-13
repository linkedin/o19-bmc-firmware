SAMHAIN_MODE="server"
INITSCRIPT_PARAMS = "defaults 14 86"

require samhain.inc

DEPENDS = "gmp"

EXTRA_OECONF += "--enable-network=${SAMHAIN_MODE} "

# supports mysql|postgresql|oracle|odbc but postgresql is the only one available

PACKAGECONFG ??= "postgresql"
PACKAGECONFIG[postgres]  = "--with-database=postgresql --enable-xml-log, "", postgresql"
PACKAGECONFIG[suidcheck]  = "--enable-suidcheck","" , "
PACKAGECONFIG[logwatch]  = "--enable-login-watch,"" , "
PACKAGECONFIG[mounts]  = "--enable-mounts-check","" , "
PACKAGECONFIG[userfiles]  = "--enable-userfiles","" , "
PACKAGECONFIG[ipv6]  = "--enable-ipv6,"--disable-ipv6","
PACKAGECONFIG[selinux] = "--enable-selinux, --disable-selinux"
PACKAGECONFIG[acl] = " --enable-posix-acl , --disable-posix-acl"

SRC_URI += "file://samhain-server-volatiles"

EXTRA_OECONF += " \
    --with-config-file=REQ_FROM_SERVER/etc/samhainrc \
    --with-data-file=REQ_FROM_SERVER/var/lib/samhain/samhain_file \
    "

do_install_append() {
    cd ${S}
    install -d ${D}${sysconfdir}/default/volatiles
    install -m 0644 ${WORKDIR}/samhain-server-volatiles \
        ${D}${sysconfdir}/default/volatiles/samhain-server

    install -m 700 samhain-install.sh init/samhain.startLinux \
        init/samhain.startLSB ${D}/var/lib/samhain
}

INSANE_SKIP_${PN} = "already-stripped"

PACKAGES = "${PN} ${PN}-doc ${PN}-dbg"

FILES_${PN} += " \
    ${sbindir}/* \
    /run \
    "

FILES_${PN}-dbg += " \
    ${sbindir}/.debug/* \
    "

RDEPENDS_${PN} += "gmp bash perl"
