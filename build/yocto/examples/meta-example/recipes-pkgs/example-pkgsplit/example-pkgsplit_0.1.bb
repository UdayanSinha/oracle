SUMMARY = "Package-splitting example recipe"
DESCRIPTION = "autotools-based package-splitting example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("*************************************************");
    bb.plain("*                                               *");
    bb.plain("*     Example recipe with package-splitting     *");
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
           file://pkg1.c \
           file://pkg2.c \
          "
S = "${WORKDIR}"

inherit autotools

# define additional package
PACKAGES =+ "${PN}-extra"

# define files to be included in the additional package
FILES:${PN}-extra = "${bindir}/pkgsplit2"

addtask display_banner before do_build
