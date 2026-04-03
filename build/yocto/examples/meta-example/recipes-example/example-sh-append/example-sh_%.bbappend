SRC_URI:append = " file://hello-world-append.sh"

# Need to add example-sh/ directory with new files or recipe build will fail
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

do_configure:append() {
    echo "hello-world-append.sh" >> ${S}/hello-world.sh
}

do_install:append() {
    install -d ${D}${bindir}/
    install -m 0755 ${S}/hello-world-append.sh ${D}${bindir}/
}
