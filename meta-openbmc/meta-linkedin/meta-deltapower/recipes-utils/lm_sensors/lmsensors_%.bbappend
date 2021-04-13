
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://deltapower.conf \
           "

do_install_board_config() {
    install -d ${D}${sysconfdir}/sensors.d
    install -m 644 ../deltapower.conf ${D}${sysconfdir}/sensors.d/deltapower.conf
}
