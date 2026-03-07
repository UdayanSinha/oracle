SUMMARY = "Library example recipe"
DESCRIPTION = "C library example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*        Example recipe with C Library.       *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

# Specify package aliases to resolve build-time dependencies elsewhere
PROVIDES = "hello-world-lib"

# Specify package aliases to resolve runtime dependencies elsewhere
RPROVIDES:${PN} = "hello-world-lib"

SRC_URI = "file://example-lib.c \
           file://example-lib.h"

S = "${WORKDIR}/build"

do_compile() {
    # Need to set SONAME for shared library
    ${CC} ${CFLAGS} -fPIC -c ${WORKDIR}/example-lib.c
    ${CC} ${LDFLAGS} -shared -Wl,-soname,libexample.so.${PV} -o libexample.so.${PV} example-lib.o
}

do_install() {
    # install shared library + header

    install -d ${D}${includedir}/
    install -m 0644 ${WORKDIR}/example-lib.h ${D}${includedir}/

    # shared library must be named as lib<name>.so.<version>
    # a symlink without version is also needed for linking
    install -d ${D}${libdir}/
    install -m 0755 libexample.so.${PV} ${D}${libdir}/
    ln -s libexample.so.${PV} ${D}${libdir}/libexample.so
}

addtask display_banner before do_build
