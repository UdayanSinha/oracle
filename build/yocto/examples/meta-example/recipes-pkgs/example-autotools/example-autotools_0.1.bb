SUMMARY = "autotools example recipe"
DESCRIPTION = "autotools-based C program example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("*************************************************");
    bb.plain("*                                               *");
    bb.plain("* Example recipe with autotools-based C Program *");
    bb.plain("*                                               *");
    bb.plain("*************************************************");
}

SRC_URI = "file://AUTHORS \
           file://COPYING \
           file://ChangeLog \
           file://NEWS \
           file://README.md \
           file://configure.ac \
           file://Makefile.am \
           file://main.c \
          "
S = "${WORKDIR}"

inherit autotools

# Specify package aliases to resolve runtime dependencies elsewhere
RPROVIDES:${PN} = "hello-world-autotools"

# equivalent to passing command-line args to configure (do_configure)
# EXTRA_OECONF = ""

# equivalent to passing command-line args to make (do_compile)
# EXTRA_OEMAKE = ""

addtask display_banner before do_build
