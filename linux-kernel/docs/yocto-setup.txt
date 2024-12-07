Tested on:
	- Ubuntu 24.04 LTS with Yocto "Scarthgap".

Yocto official documentation: https://docs.yoctoproject.org

1) Download Yocto and its dependencies.

2) Setup Bitbake build environment:
	* source oe-init-build-env
	* This can vary depending on a project. The above instruction is for Poky.

3) Build a Yocto image using BitBake: bitbake <image-name>
	* E.g. for a QEMU image in Poky: bitbake core-image-minimal
	* This image can be run with QEMU: runqemu nographic slirp

4) Build a Yocto recipe using BitBake: bitbake <recipe-name>

5) To list all recipes/layers:
	* bitbake-layers show-layers
	* bitbake-layers show-recipes

6) App development with devtool (eSDK):
	* devtool modify <recipe-name> /path/to/copy/recipe/source/into
	* Yocto will now build the recipe with source code at the above-mentioned path.
	* E.g. for using devtool with Linux kernel:
		- devtool modify linux-yocto /path/to/copy/kernel/source/into
	* To build the recipe with devtool: devtool build <recipe-name>

7) App development with standard SDK:
	* Prepare the SDK: bitbake <image-name> -c populate_sdk
	* Install the SDK (provide install path when prompted): ./tmp/deploy/sdk/<sdk-install-script>
	* Setup the SDK build environment by sourcing it's setup script (check inside the install path), and use it.
