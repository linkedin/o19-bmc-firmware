SUMMARY = "system security services daemon"
DESCRIPTION = "SSSD is a system security services daemon"
HOMEPAGE = "https://fedorahosted.org/sssd/"
SECTION = "base"
LICENSE = "GPLv3+"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

DEPENDS = "openldap cyrus-sasl libtdb ding-libs libpam c-ares krb5"
DEPENDS += "libldb dbus libtalloc libpcre glib-2.0 popt e2fsprogs libtevent"

SRCREV = "70862d81a9a1228ad27ad35c9e99cc24b77940c6"

PV = "1.13.3+git${SRCPV}"

SRC_URI = "git://git.fedorahosted.org/git/sssd.git;branch='sssd-1-13' \
            file://sssd.conf "

S = "${WORKDIR}/git"

inherit autotools pkgconfig gettext update-rc.d python-dir

CACHED_CONFIGUREVARS = "ac_cv_member_struct_ldap_conncb_lc_arg=no \
    ac_cv_path_PYTHON2=${PYTHON_DIR} ac_cv_prog_HAVE_PYTHON3=${PYTHON_DIR} \
    "

PACKAGECONFIG ?="nss"
PACKAGECONFIG += "${@base_contains('DISTRO_FEATURES', 'selinux', 'selinux', '', d)}"

PACKAGECONFIG[ssh] = "--with-ssh, --with-ssh=no, "
PACKAGECONFIG[samba] = "--with-samba, --with-samba=no, samba"
PACKAGECONFIG[selinux] = "--with-selinux, --with-selinux=no --with-semanage=no, libselinux"
PACKAGECONFIG[manpages] = "--with-manpages, --with-manpages=no"
PACKAGECONFIG[python2] = "--with-python2-bindings, --without-python2-bindings"
PACKAGECONFIG[python3] = "--with-python3-bindings, --without-python3-bindings"
PACKAGECONFIG[nss] = "--with-crypto=nss, , nss"
PACKAGECONFIG[cyrpto] = "--with-crypto=libcrypto, , libcrypto"
PACKAGECONFIG[nscd] = "--with-nscd=, --without-nscd, "
PACKAGECONFIG[nl] = "--with-libnl, --with-libnl=no, libnl"
PACKAGECONFIG[systemd] = "--with-systemdunitdir=${systemd_unitdir}/system/, --with-systemdunitdir="
PACKAGECONFIG[systemd] = "--with-systemdconfdir=${systemd_unitdir}/system/, --with-systemdconfdir="

EXTRA_OECONF += "--disable-rpath --disable-config-lib --disable-cifs-idmap-plugin --without-nfsv4-idmapd-plugin --without-ipa-getkeytab"

do_configure_prepend() {
    mkdir -p ${AUTOTOOLS_AUXDIR}/build
    cp ${STAGING_DATADIR_NATIVE}/gettext/config.rpath ${AUTOTOOLS_AUXDIR}/build/

    # libresove has host path, remove it
    sed -i -e "s#\$sss_extra_libdir##" ${S}/src/external/libresolv.m4
}

do_install () {
    oe_runmake install  DESTDIR="${D}"
    rmdir --ignore-fail-on-non-empty "${D}/${bindir}"
    install -d ${D}/${sysconfdir}/${BPN}
    install -m 600 ${WORKDIR}/${BPN}.conf ${D}/${sysconfdir}/${BPN}
}

CONFFILES_${PN} = "${sysconfdir}/${BPN}/${BPN}.conf"

INITSCRIPT_NAME = "sssd"
INITSCRIPT_PARAMS = "start 02 5 3 2 . stop 20 0 1 6 ."
SYSTEMD_SERVICE_${PN} = "${BPN}.service"
SYSTEMD_AUTO_ENABLE = "disable"

FILES_${PN} += "${libdir} ${datadir} /run ${libdir}/*.so* "
FILES_${PN}-dev = " ${includedir}/* ${libdir}/*la ${libdir}/*/*la"

# The package contains symlinks that trip up insane
INSANE_SKIP_${PN} = "dev-so"

RDEPENDS_${PN} += "bind dbus"
