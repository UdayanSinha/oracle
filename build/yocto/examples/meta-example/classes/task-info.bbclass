# Example Bitbake class

# bbplain is one of Yocto's recipe logging mechanisms.
# See https://docs.yoctoproject.org/dev/dev-manual/debugging.html#recipe-logging-mechanisms

do_fetch_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*        Fetching for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

do_unpack_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*       Unpacking for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

do_patch_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*        Patching for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

do_configure_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*      Configuring for ${PN}-${PV}-${PR}       *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

do_compile_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*       Compiling for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

do_install_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*      Installing for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

addtask do_fetch_info before do_fetch
addtask do_unpack_info before do_unpack
addtask do_patch_info before do_patch
addtask do_configure_info before do_configure
addtask do_compile_info before do_compile
addtask do_install_info before do_install
