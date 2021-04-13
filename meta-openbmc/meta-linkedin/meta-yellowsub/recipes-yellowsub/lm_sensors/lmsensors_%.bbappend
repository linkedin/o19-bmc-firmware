
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://yellowsub.conf \
           "

do_install_append() {
    install -d ${D}${sysconfdir}/sensors.d
    install -m 644 ../yellowsub.conf ${D}${sysconfdir}/sensors.d/yellowsub.conf
}
