SUMMARY = "Hello World example recipe"
DESCRIPTION = "Hello World C program example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Example recipe with Hello World C Program  *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

# SRC_URI is relative to FILESPATH when using file://
# If using git://, SRC_URI provides the git repository URL and branch information.
# SRC_REV specifies the specific commit to checkout from the git repository.
#
# E.g.
# SRC_URI = "git://github.com/rhinstaller/efivar.git;branch=main;protocol=https"
# SRCREV = "c47820c37ac26286559ec004de07d48d05f3308c"
#
# Use SRC_URI:append to apply patches.
# E.g. SRC_URI:append = "file://example-patchfile.patch"
#
# Alternatively, specify patches directly in SRC_URI w/o append.
# E.g. SRC_URI = "\
#     git://github.com/rhinstaller/efivar.git;branch=main;protocol=https \
#     file://example-patchfile.patch \
#     "
SRC_URI = "file://hello-world.c"
S = "${WORKDIR}/build"

# Specify package aliases
RPROVIDES:${PN} = "hello-world-c"

# Specify runtime package conflicts
RCONFLICTS:${PN} = "git"

# Yocto tasks from this point
# Typically do_fetch() and do_patch() are not explicitly needed
# as they will be handled by the build system based on the SRC_URI.
do_configure(){
    echo "#define SLEEP_TIME_US (250 * 1000)" > ${WORKDIR}/hello-world.h
}

do_compile() {
    # using default compiler and flags provided by the build system
    ${CC} ${CFLAGS} ${LDFLAGS} -o hello-world ${WORKDIR}/hello-world.c
}

do_install() {
    # create the target directory and install the compiled binary
    # bindir is a standard variable for the target binary directory
    # see meta/conf/bitbake.conf for more standard variables
    install -d ${D}${bindir}/
    install -m 0755 hello-world ${D}${bindir}/
}

addtask display_banner before do_build
