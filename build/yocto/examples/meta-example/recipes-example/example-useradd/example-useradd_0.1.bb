SUMMARY = "User Creation example recipe"
DESCRIPTION = "User creation example created via Yocto"

# if using a standard license, use the corresponding file name from meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

python do_display_banner() {
    bb.plain("************************************************");
    bb.plain("*                                              *");
    bb.plain("*       Example recipe with user creation      *");
    bb.plain("*                                              *");
    bb.plain("************************************************");
}

inherit useradd

RECIPE_USER = "recipeuser"
RECIPE_GRP = "recipegrp"

# fetch encrypted password, e.g. printf "%q\n" $(mkpasswd -m sha256crypt <password-text>)
RECIPE_USER_PASSWD = "\$5\$Y8nS8gke8teWfqYS\$JzzaGIMgeG1./DK.6.8px94ZStZKueBzGrw/MGfbW95"

# define package which will add the user
USERADD_PACKAGES = "${PN}"

# see: https://linux.die.net/man/8/useradd
USERADD_PARAM:${PN} += "-d /home/${RECIPE_USER} -p '${RECIPE_USER_PASSWD}' ${RECIPE_USER}"

# see: https://linux.die.net/man/8/groupadd
GROUPADD_PARAM:${PN} += "${RECIPE_GRP}"

# see: https://linux.die.net/man/8/groupmems
GROUPMEMS_PARAM:${PN} += "-g ${RECIPE_GRP} -a ${RECIPE_USER}"

# indicate home directory and its contents to be part of the package
FILES:${PN} += "/home/${RECIPE_USER}/*"

# required since this package actually does not contain any files
ALLOW_EMPTY:${PN} = "1"

do_install() {
    # setup the home directory
    install -d ${D}$/home/${RECIPE_USER}
}

addtask display_banner before do_build
