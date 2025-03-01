# Buildroot Setup


## Notes

- Tested on Ubuntu 24.04 LTS, Fedora 41.
- Tested using Buildroot release 2024.11.2.
- For details, see [buildroot.org](https://buildroot.org).


## Setup

1. Download Buildroot and its dependencies.

2. Compile a Linux image:
    - Buildroot uses a defconfig-based setup similar to Linux Kernel.
    - To list available defconfig files: `make list-defconfigs`
    - Select a defconfig and use it to build the image.

        ```console
        make qemu_x86_64_defconfig    # using QEMU x86_64 defconfig as an example
        make menuconfig               # to make changes to selected defconfig
        make
        ```

3. The compile output is located at: `/path/to/buildroot-dir/output`
    - `images/` : Kernel, rootfs, bootloader images to put on the target.
    - `build/` : Directory where all the components were built, one sub-directory per component.
    - `host/` : SDK i.e. sysroot + host-tools to compile target SW.
    - `staging/` : `host/` symlink.
    - `target/` : Near-complete rootfs, but should NOT be confused with the one under `images/`

4. A ready-made script is provided for starting QEMU with the compiled rootfs and kernel.

    ```console
    /path/to/buildroot-dir/output/images/start-qemu.sh serial-only
    ```

5. If an initramfs is needed, it should be enabled in the defconfig.
See `BR2_TARGET_ROOTFS_CPIO`

6. To override the source code location for a given package:
    - An override file is used whose path is provided by `BR2_PACKAGE_OVERRIDE_FILE`
    - Default value of `BR2_PACKAGE_OVERRIDE_FILE` : `$(CONFIG_DIR)/local.mk`
        - `CONFIG_DIR` is where the `.config` file is located.
    - In this file, the override for a given package is specified as: `<PKG>_OVERRIDE_SRCDIR = /path/to/pkg/`
        - E.g. To override for the kernel source code: `LINUX_OVERRIDE_SRCDIR = /path/to/linux-kernel-src/`
    - Buildroot will not use the default location to fetch, extract and patch an overriden package.
        - It will copy the source code from the provided override path and use that instead.
            - These will be copied to: `/path/to/buildroot-dir/output/build/<pkg>-custom`
        - Note that `make clean` will not touch the directories of overridden packages.
    - If the source code of an overriden package is modified afterwards:
        - `<pkg>-rebuild` can be used to trigger a re-copy and compile the (new) source code again.
        - E.g. To re-copy and compile for the kernel source code: `make linux-rebuild all`
