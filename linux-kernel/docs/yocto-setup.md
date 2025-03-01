# Yocto Setup


## Notes

- Ubuntu 24.04 LTS with Yocto **Scarthgap**.
- For details, see [Yocto Project Documentation](https://docs.yoctoproject.org).


## Setup

1. Download Yocto and its dependencies. Poky is Yocto's reference distribution,
and that is used for examples.

2. Setup Bitbake environment.

    ```console
    source oe-init-build-env    # can vary depending on the Yocto distro
    ```

3. Build a Yocto image using BitBake: `bitbake <image-name>`

    ```console
    bitbake core-image-minimal    # minimal QEMU image in Poky
    runqemu nographic slirp       # to run the image with QEMU
    ```

4. Build a Yocto recipe using BitBake: `bitbake <recipe-name>`

5. To list all layers: `bitbake-layers show-layers`

6. To list all recipes: `bitbake-layers show-recipes`

7. App development with devtool (eSDK):
    - Setup a recipe to be develop for: `devtool modify <recipe-name> /path/to/copy/recipe/source/into`
    - Yocto will now fetch, extract and patch the recipe at the above-mentioned path, and build it from there.
    - E.g. for using devtool with Linux kernel: `devtool modify linux-yocto /path/to/copy/kernel/source/into`
    - To build the recipe with devtool: `devtool build <recipe-name>`

8. App development with standard SDK:
    - Prepare the SDK: `bitbake <image-name> -c populate_sdk`
    - Install the SDK (provide install path when prompted): `./tmp/deploy/sdk/<sdk-install-script>`
    - Setup the SDK build environment by sourcing it's setup script (check inside the install path), and use it.
