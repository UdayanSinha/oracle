SUMMARY = "Hello World example Kernel Module"
DESCRIPTION = "Hello World C Linux Kernel Module example created via Yocto"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("*********************************************************");
    bb.plain("*                                                       *");
    bb.plain("* Example recipe with Hello World C Linux Kernel Module *");
    bb.plain("*                                                       *");
    bb.plain("*********************************************************");
}

SRC_URI = "file://hello_world.c \
           file://Makefile \
          "
S = "${WORKDIR}"

inherit module

RPROVIDES:${PN} += "kernel-module-hello_world"

addtask display_banner before do_build
