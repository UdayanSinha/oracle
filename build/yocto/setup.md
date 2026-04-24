# Yocto Setup


## Notes

- Tested on Ubuntu 24.04 LTS with Yocto **Scarthgap**.
- See [Welcome to the Yocto Project Documentation - The Yocto Project](https://docs.yoctoproject.org).
    - Yocto concepts: [Yocto Project Overview and Concepts Manual - The Yocto Project](https://docs.yoctoproject.org/overview-manual/index.html).
    - Yocto build environment setup: [Setting up to use the Yocto Project - The Yocto Project](https://docs.yoctoproject.org/dev-manual/start.html).
    -  OE Layer Index to find existing layers: [OpenEmbedded Layer Index](https://layers.openembedded.org/layerindex/branch/master/layers).
    - Yocto variables:
        - [Variables Glossary - The Yocto Project](https://docs.yoctoproject.org/ref-manual/variables.html).
        - [Variable Context - The Yocto Project](https://docs.yoctoproject.org/ref-manual/varlocality.html).
    - Syntax: [Syntax and Operators - Bitbake Documentation](https://docs.yoctoproject.org/bitbake/2.2/bitbake-user-manual/bitbake-user-manual-metadata.html)
    - Debugging guidelines: [Debugging Tools & Techniques - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/debugging.html#debugging-tools-and-techniques).
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

Configure `build/conf/local.conf` and/or `build/conf/bblayers.conf` in the build directory if needed.
Yocto variable for project build directory: `TOPDIR`

1. Important variables to modify under `build/conf/local.conf` (specified for `CONF_VERSION` = 2):
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
2. Important variables to modify under `build/conf/bblayers.conf` :
    - `BBLAYERS` : Layers used in current image build.

Note that the above 2 files affect local builds only, generally for every image that is built.
If there is a need for the settings to persist, some of them are possible to use in `.conf` files of layers.


## Image Build

As Yocto builds can be storage-intensive, it is highly recommended to add this line in `build/conf/local.conf`: `INHERIT += "rm_work"`
<br>See [Conserving Disk Space - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/disk-space.html).

Build a Yocto image using BitBake: `bitbake <image-name>`. The built image is present at `${TMPDIR}/deploy/images`.

For details of default image build options provided by Poky, see [Images - The Yocto Project](https://docs.yoctoproject.org/ref-manual/images.html).

E.g. for building and running an image for QEMU:

```console
bitbake core-image-full-cmdline
runqemu nographic slirp
```

To remove all previous builds: `bitbake -c cleanall world`


## Image Customization

See [Customizing Images - The Yocto Project](https://docs.yoctoproject.org/dev-manual/customizing-images.html).

### Using `build/conf/local.conf`

Note that some of the below-mentioned variables may be used in `.conf` files of layers, if there is a need for them to persist.

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

3. Add an existing layer in the build config. (will update `BBLAYERS` in `build/conf/bblayers.conf`): `bitbake-layers add-layer /path/to/meta-<layer-name>`

4. Remove a layer from the build config. (will update `BBLAYERS` in `build/conf/bblayers.conf`): `bitbake-layers remove-layer layer`
    - Top-level layer (e.g. `meta` in Poky) should not be removed.
    - If that is done, it will have to be added back by manually editing `build/conf/bblayers.conf`.


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

7. Use development shell (devshell) to aid in recipe debugging, if needed: `bitbake -c devshell <recipe-name>`
    - Allows access to Bitbake task execution environment for manual operations e.g. manually running `configure` or `make`.
    - While Bitbake will auto-detect a suitable terminal program for the shell, `OE_TERMINAL` can be set in `build/conf/local.conf` to explicitly specify a program.
    - If running on a remote system via SSH, recommend having tmux/screen installed in that system.
    - There may be issues to start devshell if recipe is already built i.e. it may be required to do `bitbake -c <cleanall,cleansstate> <recipe-name>`
    - See [Using a Development Shell - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/development-shell.html#using-a-development-shell).


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


## Classes

1. Provides a mechanism for re-using functions among recipe files.
    - E.g. Classes for handling different build systems (make, autotools, etc).
2. Implemented in `.bbclass` files and put under `meta-<layer-name>/classes`.
3. Need to be inherited by the recipe via use of `inherit <class-name>` prior to use.
    - In most cases, this is sufficient to begin using features of the class.
4. Classes themselves may also inherit from other classes.
5. Via use of `INHERIT`, classes may be inherited in `.conf` files of layers (persistent) and `build/conf/local.conf` (local builds) too.
    - E.g. `INHERIT += "rm_work"`
6. See [Classes - The Yocto Project](https://docs.yoctoproject.org/dev/ref-manual/classes.html#classes).


## devtool

1. devtool has its own workspace layer where recipes need to be imported to, and changes need to be exported from (to the usual layers).
    - The layer is automatically added to `build/conf/bblayers.conf` so it can be used for image builds.
2. To create a new recipe in workspace: `devtool add <recipe-name> <src-info>`
    - E.g. `devtool add crun-example https://github.com/containers/crun.git --srcbranch=main`
    - If `--srcbranch` is not used for git sources, devtool will default to `master` (and fail if there is no `master` branch).
    - Generally devtool will try to detect the build system (make, autotools, etc) and prepare a boilerplate recipe file accordingly.
3. Setup an existing recipe modify source for: `devtool modify <recipe-name>`
    - Yocto will now fetch, extract and patch the recipe at the above-mentioned path, and build it from there.
4. To build the recipe with devtool: `devtool build <recipe-name>`
5. To export recipe to a layer (i.e. a layer in `BBLAYERS` other than workspace layer): `devtool finish <recipe-name> /path/to/meta-<layer-name>`
    - If the recipe does not exist in any layer (`devtool add ...`), the recipe will be created in specified layer.
    - If the recipe exists in a layer (`devtool modify ...`):
        - If it is not the specified layer, the changes will be appended in specified layer (`.bbappend`).
        - If it is the specified layer, the recipe will be updated.
        - A typical patching workflow would look like this:

            ```console
            devtool modify <recipe-name>

            # modify files in devtool source directory and make a git commit out of it

            devtool finish <recipe-name> /path/to/meta-<layer-name>
            ```

6. Find a recipe file (searches in all layers specified by `BBLAYERS`): `devtool find-recipe <recipe-name>`
7. To delete recipe files (will not remove source code) from workspace: `devtool reset <recipe-name>`
8. devtool facilitates development testing of the recipe by deploying/un-deploying files to target via SSH:
    - The files that are deployed are the same ones that are installed in `do_install` task.
    - Runtime dependencies of the recipe must already be present on target.
    - Deploy to target: `devtool deploy-target <recipe-name> <user>@<hostname>`
    - Un-deploy from target: `devtool undeploy-target <recipe-name> <user>@<hostname>`
9. See [devtool - The Yocto Project](https://docs.yoctoproject.org/dev/ref-manual/devtool-reference.html).


## Standard SDK

1. Prepare the SDK: `bitbake <image-name> -c populate_sdk`
2. Install the SDK (provide install path when prompted): `${TMPDIR}/deploy/sdk/<sdk-install-script>`
3. Setup the SDK build environment by sourcing it's setup script (check inside the install path), and use it.


## oe-pkgdata-utils

Allows inspection of built packages. Useful commands include:

1. List packages: `oe-pkgdata-util list-pkgs`
2. List package files: `oe-pkgdata-util list-pkg-files <package-name>`
3. Find recipe corresponding to a package: `oe-pkgdata-util lookup-recipe <package-name>`
4. Get package information: `oe-pkgdata-util package-info <package-name>`

See [Package Information - The Yocto Project](https://docs.yoctoproject.org/dev/dev-manual/debugging.html#viewing-package-information-with-oe-pkgdata-util).


## Linux Kernel Development

1. Use devshell for tweaking Linux kernel config. via `menuconfig` task: `bitbake -c menuconfig linux-yocto`
2. The kernel will now be built with the changes made via the `menuconfig` task.
3. The above-mentioned approach may be used to build locally, but it will not be saved into the recipe automatically. There are 2 ways to do that:
    - Defconfig:
        - Tweak kernel config. via `menuconfig` task.
        - Save entire config. via `savedefconfig` task: `bitbake -c savedefconfig linux-yocto`
        - Use config. by appending to `linux-yocto` recipe.
        - See [Creating a defconfig - The Yocto Project](https://docs.yoctoproject.org/dev/kernel-dev/common.html#creating-a-defconfig-file).
    - Config. fragments:
        - Tweak kernel config. via `menuconfig` task.
        - Use `diffconfig` task to create fragment: `bitbake -c diffconfig linux-yocto`
        - Use fragment by appending to `linux-yocto` recipe.
        - See [Creating configuration fragments- The Yocto Project](https://docs.yoctoproject.org/dev/kernel-dev/common.html#creating-configuration-fragments).
    - Both of the above-mentioned methods can be used together, if needed.
        - Bitbake applies config. fragments after applying defconfig.
4. To incorporate out-of-tree (OOT) kernel modules in recipes, use `meta-skeleton/recipes-kernel/hello-mod` as the template.
    - See [Incorporating OOT modules - The Yocto Project](https://docs.yoctoproject.org/dev/kernel-dev/common.html#incorporating-out-of-tree-modules).


## Systemd Service Development

To support systemd services for a recipe, see [systemd Class - The Yocto Project](https://docs.yoctoproject.org/dev/ref-manual/classes.html#systemd).


## User & Group Management

There are 2 ways to add users and groups:

1. `useradd` class:
    - Added as a recipe that needs to be included in the image.
    - See [useradd Class - The Yocto Project](https://docs.yoctoproject.org/3.4.1/ref-manual/classes.html?highlight=useradd#useradd-bbclass);
2. `extrausers` class:
    - Added in `.conf` files like `build/conf/local.conf`.
    - E.g. shows how to add a new user and a password to root user via `build/conf/local.conf` :

        ```
        # debug-tweaks will prevent setting of root password if used in
        # EXTRA_IMAGE_FEATURES or IMAGE_FEATURES
        EXTRA_IMAGE_FEATURES:remove = " debug-tweaks"

        # Not using debug-tweaks will also remove root-login
        EXTRA_IMAGE_FEATURES:append = " allow-root-login"

        INHERIT += "extrausers"

        # fetch encrypted password, e.g.: printf "%q" $(mkpasswd -m sha256crypt <password-plain-text>)
        TESTERPASSWD = "\$5\$rVTscnZtyqKZA59G\$GrLOjFzVkB8UT7VT1KBqsbmg3Hxu8ofuWZgnkC6koP9"
        ROOTPASSWD = "\$5\$hciPQWGuqNT/NIqX\$pQazKrfEQF6.6vbQVR2IV2DY/BXAKEd0jnHs3tjLNrB"

        # see: https://linux.die.net/man/8/useradd
        # see: https://linux.die.net/man/8/usermod
        EXTRA_USERS_PARAMS = "\
            useradd -p '${TESTERPASSWD}' tester; \
            usermod -p '${ROOTPASSWD}' root; \
        "
        ```

    - See [extrausers Class - The Yocto Project](https://docs.yoctoproject.org/3.4.1/ref-manual/classes.html?highlight=useradd#extrausers-bbclass).
