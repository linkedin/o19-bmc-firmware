SAMHAIN_MODE="client"
INITSCRIPT_PARAMS = "defaults 15 85"

require samhain.inc

#Let the default Logserver be 127.0.0.1
EXTRA_OECONF += " \
        --with-logserver=${SAMHAIN_SERVER} \
        --with-port=${SAMHAIN_PORT} \
        --with-config-file=/etc/samhainrc \
        --with-data-file=/var/samhain/samhain.data \
        --with-pid-file=/var/samhain/samhain.pid \
        "


INSANE_SKIP_${PN} = "already-stripped"

FILES_${PN} += "\
    /run \
    "

RDEPENDS_${PN} = "acl zlib attr bash"
