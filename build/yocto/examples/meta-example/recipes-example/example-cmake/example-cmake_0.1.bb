SUMMARY = "Cmake example recipe"
DESCRIPTION = "cmake-based C program example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Example recipe with cmake-based C Program  *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

SRC_URI = "file://main.c \
           file://CMakeLists.txt \
          "
S = "${WORKDIR}"

inherit cmake

# Specify package aliases to resolve runtime dependencies elsewhere
RPROVIDES:${PN} = "hello-world-cmake"

# equivalent to passing command-line args to cmake
# EXTRA_OECMAKE = ""

addtask display_banner before do_build
