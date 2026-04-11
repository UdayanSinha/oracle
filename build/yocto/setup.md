# Yocto Setup


## Notes

- Tested on Ubuntu 24.04 LTS with Yocto **Scarthgap**.
- For details, see [Welcome to the Yocto Project Documentation - The Yocto Project](https://docs.yoctoproject.org).
    - Yocto concepts: [Yocto Project Overview and Concepts Manual - The Yocto Project](https://docs.yoctoproject.org/overview-manual/index.html).
    - Yocto build environment setup: [Setting up to use the Yocto Project - The Yocto Project](https://docs.yoctoproject.org/dev-manual/start.html).
    -  OE Layer Index to find existing layers: [OpenEmbedded Layer Index](https://layers.openembedded.org/layerindex/branch/master/layers).
    - Yocto variables:
        - [Variables Glossary - The Yocto Project](https://docs.yoctoproject.org/ref-manual/variables.html).
        - [Variable Context - The Yocto Project](https://docs.yoctoproject.org/ref-manual/varlocality.html).
    - Syntax: [Syntax and Operators - Bitbake dev documentation](https://docs.yoctoproject.org/bitbake/2.2/bitbake-user-manual/bitbake-user-manual-metadata.html)
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
    # this will put us under the project build directory
    source oe-init-build-env
    ```


## Build Configuration

Configure `conf/local.conf` and/or `conf/bblayers.conf` in the build directory if needed.
Yocto variable for project build directory: `TOPDIR`

1. Important variables to modify under `conf/local.conf` (specified for `CONF_VERSION` = 2):
    - `MACHINE` : Build target platform.
    - `DL_DIR` : Downloaded sources are stored at this path.
    - `SSTATE_DIR` : Build cache (to accelerate future builds) is stored at this path.
    - `TMPDIR` : Build workspace. Final image builds are also place here.
    - `DISTRO` : Distribution setting.
    - `PACKAGE_CLASSES` : Packaging format (RPM, DEB, etc) to use for built packages.
        - The rootfs will ultimately be generated from these packagaes.
    - `USER_CLASSES` : Enable additional features during build.
        - E.g. `buildstats` can be used to collect statistics about each Yocto build task, for a given recipe. See: `${TMPDIR}/buildstats`
    - `PATCHRESOLVE` : Handle conflict according to specified action when patch step fails.
        - `noop` : Build task will fail with an error.
    - `BB_DISKMON_DIRS` : Specifies safety margin for storage space monitoring during build.
    - `CONF_VERSION` : Version of this file's format.
        - Incremented each time the file format changes incompatibly.
2. Important variables to modify under `conf/bblayers.conf` :
    - `BBLAYERS` : Layers used in current image build.

Note that the above 2 files affect local builds only, generally for every image that is built.
If there is a need for the settings to persist, some of them are possible to use elsewhere.


## Image Build

As Yocto builds can be storage-intensive, it is highly recommended to add this line in `conf/local.conf`: `INHERIT += "rm_work"`
<br>For details, see [Conserving Disk Space - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/disk-space.html).

Build a Yocto image using BitBake: `bitbake <image-name>`. The built image is present at `${TMPDIR}/deploy/images`.

For details of default image build options provided by Poky, see [Images - The Yocto Project](https://docs.yoctoproject.org/ref-manual/images.html).

E.g. for building and running an image for QEMU:

```console
bitbake core-image-full-cmdline
runqemu nographic slirp
```

To remove all previous builds: `bitbake -c cleanall world`


## Image Customization

For details, see [Customizing Images - The Yocto Project](https://docs.yoctoproject.org/dev-manual/customizing-images.html).

### Using `conf/local.conf`

Note that some of the below-mentioned variables may be used elsewhere, if there is a need for them to persist.

1. `EXTRA_IMAGE_FEATURES` : This enables multiple packages or configurations to be add into the image.
    - E.g. `EXTRA_IMAGE_FEATURES += "tools-sdk"` will enable debug tools (like GDB and strace) to be built into the image.
    - See [Image Features - The Yocto Project](https://docs.yoctoproject.org/ref-manual/features.html#image-features) for a complete list of allowed image features in Poky.

2. `IMAGE_INSTALL` with `:append`: Add a specified package into the image:
    - Note that the space is needed before `<package-name>`.
    - Add a package for all locally-built images: `IMAGE_INSTALL:append = " <package-name>"`.
        - E.g. `IMAGE_INSTALL:append = " git"`

3. `PREFERRED_PROVIDERS` : Specify which recipe to pick, if there are multiple recipes providing the same package.

4. `PREFERRED_VERSION` : Specify which recipe version to pick, if there are multiple versions of the same recipe available.

5. `BBMASK` : Specify recipes and recipe append files to be prevented from BitBake processing.

6. To customize the init manager, see: [Selecting an Init Manager - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/init-manager.html).

7. To customize the device manager, see: [Selecting a Device Manager - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/device-manager.html).


## Layers

1. Create a new layer: `bitbake-layers create-layer /path/to/meta-<layer-name>`
    - Layers are named `meta-<layer-name>` by convention, although the `meta-` prefix is not mandatory.
    - Layers are typically created in the top-level Yocto directory.
    - Priority is set within `/path/to/meta-<layer-name>/conf/layer.conf` using `BBFILE_PRIORITY_<layer-name>` variable.

2. To list all layers added to build config.: `bitbake-layers show-layers`
    - Priority values control which layer takes precedence if there are recipe files with the same name in multiple layers.
        - `PREFERRED_PROVIDERS` and `PREFERRED_VERSION` will override this.
    - Higher number => higher priority.
    - The priority can be specified manually for a layer, or generated by the build system by default.
    - See [Prioritizing Your Layer - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/layers.html#prioritizing-your-layer).

3. Add an existing layer in the build config. (will update `BBLAYERS` in `conf/bblayers.conf`): `bitbake-layers add-layer /path/to/meta-<layer-name>`

4. Remove a layer from the build config. (will update `BBLAYERS` in `conf/bblayers.conf`): `bitbake-layers remove-layer layer`
    - Top-level layer (e.g. `meta` in Poky) should not be removed.
    - If that is done, it will have to be added back by manually editing `conf/bblayers.conf`.


## Recipes

1. Recipes are put in directories under the root directory of the corresponding layer i.e. a layer may have one or more recipes.
    - Recipe directories are conventionally named `recipe-<recipe-dirname>`.
    - A recipe directory may itself contain several recipes (described by recipe files) in sub-directories.

2. Recipe files are usually named: `<PN>_<PV>_<PR>.bb`
    - E.g. `example_6.6_1.0`
    - `PN` : Package Name.
        - Used as `<recipe-name>` in examples here.
        - Conventionally also the sub-directory name in the recipe directory.
    - `PV` : Package Version.
    - `PR` : Package Revision.
    - It is possible to skip `PV` and `PR` in the file naming, in which case Yocto will assign default values to them internally.
    - The values (applies to other Yocto variables too) can be grepped by running Bitbake with `-e` : `bitbake -e <recipe-name> | grep ...`
        - E.g. `bitbake -e git | grep ^PN=`

3. To list all recipes (along with corresponding version and layer): `bitbake-layers show-recipes`
    - Layer must be present in the build config.
    - To list/check for a specific recipe: `bitbake-layers show-recipes <recipe-name>`

4. Other key variables related to Yocto build of a recipe:
    - `WORKDIR` : Build directory used for a recipe.
    - `S` : Recipe source code is unpacked here.
    - `D` : Compiled recipe binaries are stored here by the `do_install()` task.
    - `B` : Intermediate build objects of the recipe are placed here during the build process.
    - `DEPENDS` : Manage build-time dependencies for the package.
    - `PROVIDES` : Manage package aliases for resolving build-time dependencies.
    - `RDEPENDS` : Manage runtime dependencies for the package.
    - `RPROVIDES` : Manage package aliases for resolving runtime dependencies.
    - `RCONFLICTS` : Specify runtime conflicts between packages.

5. Build a Yocto recipe using BitBake (layer must be present in the build config.): `bitbake <recipe-name>`
    - To remove build of a recipe (e.g. to do a clean build): `bitbake -c cleanall <recipe-name>`

6. Customize recipes with `.bbappend` file:
    - See [Appending Other Layers Metadata With Your Layer - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/layers.html#appending-other-layers-metadata-with-your-layer).
    - Named in the same way as the recipe file being appended to, except with a `.bbappend` extension.
    - Generally this could mean that the append is applied to a specific recipe version/revision, unless the file name is tweaked.
        - If recipe is named `example_6.6.bb`, `example_%.bbappend` will be applied to all `PV` values for the recipe.
    - Use `FILESEXTRAPATHS` in the `.bbappend` file to allow finding new files being added by the changes.


## Tasks

1. Tasks are units of execution in Bitbake.
    - See [Tasks - The Yocto Project](https://docs.yoctoproject.org/dev/ref-manual/tasks.html).

2. Following are some of the common tasks (specified in order of execution):
    - `do_fetch()` : Fetches the source code into `DL_DIR`.
    - `do_unpack()` : Unpacks the source code into the recipe `S`.
    - `do_patch()` : Applies available patch files to the source code.
        - For creating patch-files, see [devtool](#devtool).
    - `do_configure()` : Runs available configuration steps to the source code prior to build.
    - `do_compile` : Runs compilation steps on the source code.
    - `do_install` : Copies built binaries from the compilation directory to `D`.

3. Run specific tasks for a given recipe: `bitbake -c <task> <recipe-name>`
    - E.g. Run the `do_fetch` task for git recipe in meta layer: `bitbake -c do_fetch git`
    - Note that pre-requisite tasks will also be automatically run, if not already done.
        - E.g. `do_unpack()` will also run `do_fetch()`.

4. To list all tasks for a given recipe: `bitbake -c listtasks <recipe-name>`


## devtool

1. Setup a recipe to be develop for: `devtool modify <recipe-name> /path/to/copy/recipe/source/into`
2. Yocto will now fetch, extract and patch the recipe at the above-mentioned path, and build it from there.
3. E.g. for using devtool with Linux kernel: `devtool modify linux-yocto /path/to/copy/kernel/source/into`
4. To build the recipe with devtool: `devtool build <recipe-name>`
5. Find a recipe file: `devtool find-recipe <recipe-name>`
6. Patching workflow:
    ```console
    devtool modify <recipe-name> /path/to/copy/recipe/source/into

    # modify files in devtool source directory and make a git commit out of it

    devtool update-recipe <recipe-name>    # create patch and update recipe
    devtool reset <recipe-name>
    ```

## Standard SDK

1. Prepare the SDK: `bitbake <image-name> -c populate_sdk`
2. Install the SDK (provide install path when prompted): `${TMPDIR}/deploy/sdk/<sdk-install-script>`
3. Setup the SDK build environment by sourcing it's setup script (check inside the install path), and use it.
