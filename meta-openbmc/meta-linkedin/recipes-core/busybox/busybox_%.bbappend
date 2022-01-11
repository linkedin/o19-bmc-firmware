FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://busybox.cfg \
            file://setup_crond.sh \
            file://reimage \
            file://dhcp \
           "

DEPENDS_append = " update-rc.d-native"

do_install_append() {
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/cron
  install -d ${D}${sysconfdir}/cron/crontabs
  install -d ${D}${sysconfdir}/cron.reimage
  install -d ${D}${sysconfdir}/cron.dhcp
  install -m 755 ../setup_crond.sh ${D}${sysconfdir}/init.d/setup_crond.sh
  install -m 755 ../reimage ${D}${sysconfdir}/cron.reimage
  install -m 755 ../dhcp ${D}${sysconfdir}/cron.dhcp
  update-rc.d -r ${D} setup_crond.sh start 97 5 .
}
