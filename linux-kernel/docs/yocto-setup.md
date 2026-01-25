# Yocto Setup


## Notes

- Ubuntu 24.04 LTS with Yocto **Scarthgap**.
- For details, see [Welcome to the Yocto Project Documentation - The Yocto Project](https://docs.yoctoproject.org).
    - For Yocto concepts, see [Yocto Project Overview and Concepts Manual - The Yocto Project](https://docs.yoctoproject.org/overview-manual/index.html).
    - For Yocto build environment setup, see [Setting up to use the Yocto Project - The Yocto Project](https://docs.yoctoproject.org/dev-manual/start.html).
    -  Use the OE Layer Index to find existing layers. See [OpenEmbedded Layer Index](https://layers.openembedded.org/layerindex/branch/master/layers).
    - For an explanation of Yocto variables see:
        - [Variables Glossary - The Yocto Project](https://docs.yoctoproject.org/ref-manual/variables.html).
        - [Variable Context - The Yocto Project](https://docs.yoctoproject.org/ref-manual/varlocality.html).
    - For an explanation of the default image build options provided by Poky, see [Images - The Yocto Project](https://docs.yoctoproject.org/ref-manual/images.html).
- Poky is Yocto's reference distribution config., and that is used for examples.


## Build Environment Setup

1. Download Yocto build-host dependencies (see documentation).

2. Clone Poky repository, and checkout appropriate branch/tag:

    ```console
    git clone -b scarthgap git://git.yoctoproject.org/poky
    cd /path/to/cloned/repo
    ```

3. Setup the build environment.

    ```console
    # can vary depending on the Yocto distro
    # this will put us under the build directory
    source oe-init-build-env
    ```


## Build Configuration

Configure `conf/local.conf` and/or `conf/bblayers.conf` in the build directory if needed.
Yocto variable for build directory: `TOPDIR`
1. Important variables to modify under `conf/local.conf` (specified for `CONF_VERSION` = 2):
    - `MACHINE` : Build target platform.
    - `DL_DIR` : Downloaded sources are stored at this path.
    - `SSTATE_DIR` : Build cache (to accelerate future builds) is stored at this path.
    - `TMPDIR` : Build workspace. Final image builds are also place here.
    - `DISTRO` : Distribution setting.
    - `PACKAGE_CLASSES` : Packaging format (RPM, DEB, etc) to use for built packages.
        - The rootfs will ultimately be generated from these packagaes.
    - `EXTRA_IMAGE_FEATURES` : Extra packages to add into the target image.
    - `USER_CLASSES` : Enable additional features during build.
        - E.g. `buildstats` can be used to collect statistics about each Yocto build task, for a given recipe. See: `${TMPDIR}/buildstats`
    - `PATCHRESOLVE` : Handle conflict according to specified action when patch step fails.
        - `noop` : Build task will fail with an error.
    - `BB_DISKMON_DIRS` : Specifies safety margin for storage space monitoring during build.
    - `CONF_VERSION` : Version of this file's format.
        - Incremented each time the file format changes incompatibly.
2. Important variables to modify under `conf/bblayers.conf` :
    - `BBLAYERS` : Layers used in current image build.


# Image Build

Build a Yocto image using BitBake: `bitbake <image-name>`
- The built image is present at `${TMPDIR}/deploy/images`.
- For non-QEMU targets, the image will (always?) be a `.wic` file.

```console
bitbake core-image-minimal    # minimal QEMU image in Poky
runqemu nographic slirp       # to run the image with QEMU
```


# Layers & Recipes

1. Build a Yocto recipe using BitBake: `bitbake <recipe-name>`

2. To list all layers: `bitbake-layers show-layers`

3. To list all recipes: `bitbake-layers show-recipes`


# SDK

1. Application development with devtool (eSDK):
    - Setup a recipe to be develop for: `devtool modify <recipe-name> /path/to/copy/recipe/source/into`
    - Yocto will now fetch, extract and patch the recipe at the above-mentioned path, and build it from there.
    - E.g. for using devtool with Linux kernel: `devtool modify linux-yocto /path/to/copy/kernel/source/into`
    - To build the recipe with devtool: `devtool build <recipe-name>`

2. Application development with standard SDK:
    - Prepare the SDK: `bitbake <image-name> -c populate_sdk`
    - Install the SDK (provide install path when prompted): `${TMPDIR}/deploy/sdk/<sdk-install-script>`
    - Setup the SDK build environment by sourcing it's setup script (check inside the install path), and use it.
