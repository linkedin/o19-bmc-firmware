FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://rsyslog.conf \
            file://rsyslog.logrotate \
"

do_install_append() {
  install -m 644 ${WORKDIR}/rsyslog.conf ${D}${sysconfdir}/rsyslog.conf
  install -m 644 ${WORKDIR}/rsyslog.logrotate ${D}${sysconfdir}/logrotate.rsyslog
}

