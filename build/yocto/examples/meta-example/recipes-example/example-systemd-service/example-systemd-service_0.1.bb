SUMMARY = "systemd service example recipe"
DESCRIPTION = "systemd service example created via Yocto"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*     Example recipe with systemd service     *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

SRC_URI = "file://hello-world.service"
S = "${WORKDIR}"

# Specify build-time package dependencies for the target package
RDEPENDS:${PN} = "hello-world-c"

# Specify package aliases to resolve runtime dependencies elsewhere
RPROVIDES:${PN} = "hello-world-sysd-service"

# Must enable systemd in image build to be able to use the class
inherit systemd

# indicate service file for the package (main package in this case)
SYSTEMD_SERVICE:${PN} = "hello-world.service"

# service enabled by default
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
    # install systemd unit file(s)
    install -d ${D}${systemd_unitdir}/system/
    install -m 0644 ${S}/hello-world.service ${D}${systemd_unitdir}/system/
}

addtask display_banner before do_build
