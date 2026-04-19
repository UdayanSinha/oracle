# Example of nested class usage

inherit task-info

do_between_compile_and_install_info() {
    bbplain "************************************************"
    bbplain "*                                              *"
    bbplain "*      Nested log for ${PN}-${PV}-${PR}        *"
    bbplain "*                                              *"
    bbplain "************************************************"
}

addtask do_between_compile_and_install_info before do_install_info
