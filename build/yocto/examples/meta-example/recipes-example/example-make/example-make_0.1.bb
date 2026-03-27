SUMMARY = "Makefile example recipe"
DESCRIPTION = "make-based C program example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Example recipe with make-based C Program   *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

SRC_URI = "file://main.c \
           file://example-func.c \
           file://Makefile \
          "
S = "${WORKDIR}"

# Specify package aliases to resolve runtime dependencies elsewhere
RPROVIDES:${PN} = "hello-world-make"

do_compile() {
    oe_runmake
}

do_install() {
    oe_runmake install DESTDIR=${D} BINDIR=${bindir}
}

addtask display_banner before do_build

