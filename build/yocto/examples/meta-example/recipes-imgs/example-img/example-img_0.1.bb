SUMMARY = "Example image recipe"
DESCRIPTION = "Image recipe example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*         Example recipe for an image         *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

inherit core-image

IMAGE_FEATURES += "splash ssh-server-openssh package-management"

IMAGE_INSTALL = "\
    packagegroup-core-boot \
    packagegroup-core-full-cmdline \
    ${CORE_IMAGE_EXTRA_INSTALL} \
    hello-world-c \
    hello-world-lib \
    hello-world-sh \
    hello-world-make \
    hello-world-cmake \
    hello-world-autotools \
    hello-world-sysd-service \
    kernel-module-hello_world \
    example-useradd \
    example-pkgsplit \
    example-pkgsplit-extra \
    "

addtask display_banner before do_build
