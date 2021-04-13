FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += " \
            file://syslog \
            file://syslog.conf \
            file://syslog-startup.conf \
           "
