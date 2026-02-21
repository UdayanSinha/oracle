SUMMARY = "BASH example recipe"
DESCRIPTION = "BASH script example created via Yocto"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*       Example recipe with BASH script       *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

SRC_URI = "file://hello-world.sh"
S = "${WORKDIR}"

# Specify runtime dependencies for the target package
RDEPENDS:${PN} = "bash"

# Specify package aliases
RPROVIDES:${PN} = "hello-world-sh"

do_install() {
    install -d ${D}${bindir}/
    install -m 0755 ${S}/hello-world.sh ${D}${bindir}/
}

addtask display_banner before do_build
