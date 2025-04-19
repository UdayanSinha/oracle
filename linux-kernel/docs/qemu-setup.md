# QEMU Setup For Kernel Development


## Notes

- Tested on Ubuntu 24.04 LTS, Fedora Workstation 40 KDE.
- For details, see [qemu.org](https://www.qemu.org).


## Setup

### Install QEMU and its dependencies

```console
sudo apt install qemu-system qemu-kvm libvirt-daemon libvirt-clients bridge-utils virt-manager debootstrap    # Ubuntu
sudo dnf install qemu qemu-kvm libvirt-daemon libvirt-client bridge-utils virt-manager debootstrap            # Fedora
```

### Install necessary dependencies for compiling the kernel

See [Minimal requirements to compile the Kernel](https://docs.kernel.org/process/changes.html).

### Download the kernel source code

See [The Linux Kernel Archives](https://kernel.org).

### Select a defconfig and use it to build the kernel

```console
make x86_64_defconfig        # using default x86_64 as an example
make ARCH=arm64 defconfig    # using default ARM64 as an example
make menuconfig              # to make changes to selected defconfig
make
```

- To list available defconfigs: `make ARCH=x86 help | grep defconfig`
- To load a specific defconfig via menuconfig: `export KBUILD_CONFIG=/path/to/file.config`

### Create a QEMU disk image and run QEMU with compiled kernel

```console
export KERN=/path/to/kernel/image    # bzImage, Image, etc
export IMG=/path/to/disk.img
export MNT=/mnt/qemufs    # create directory if it does not already exist
qemu-img create ${IMG} 8G
mkfs.ext4 ${IMG}
sudo mount -o loop ${IMG} ${MNT}
sudo debootstrap --merged-usr stable ${MNT} http://deb.debian.org/debian
sudo chroot ${MNT} apt clean
sudo chroot ${MNT} /usr/bin/passwd
sudo umount ${MNT}
CPU="max"    # if KVM is available, "host" can also be used
RAM="256M"
SMP=2

# x86_64 (if KVM is available, add -enable-kvm):
sudo qemu-system-x86_64 -cpu ${CPU} -smp ${SMP} -m ${RAM} \
-nographic -machine q35 -kernel ${KERN} -hda ${IMG} \
-append "root=/dev/sda rw console=ttyS0"

# ARM64 (if KVM is available, add -enable-kvm):
sudo qemu-system-arm64 -cpu ${CPU} -smp ${SMP} -m ${RAM} -nographic -machine virt \
-kernel ${KERN} -append "root=/dev/vda rw console=ttyAMA0" \
-drive if=none,file=${IMG},format=raw,id=hd0 \
-device virtio-blk-device,drive=hd0
```

### Setup network interfaces

1. See [Documentation/Networking - QEMU](https://wiki.qemu.org/Documentation/Networking).
2. Run QEMU (host port 5555 forwarded to target port 22 for SSH access).

    ```console
    # x86_64:
    sudo qemu-system-x86_64 -enable-kvm -cpu ${CPU} -smp ${SMP} -m ${RAM} \
    -nographic -machine q35 -kernel ${KERN} -hda ${IMG} \
    -append "root=/dev/sda rw console=ttyS0" \
    --device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22

    # ARM64:
    sudo qemu-system-arm64 -cpu ${CPU} -smp ${SMP} -m ${RAM} -nographic -machine virt \
    -kernel ${KERN} -append "root=/dev/vda rw console=ttyAMA0" \
    -drive if=none,file=${IMG},format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    --device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22
    ```

3. Network interfaces may need to be set up manually on the QEMU target.

    ```console
    # check network interface name, example assumes enp0s2
    ip link set enp0s2 up
    dhclient enp0s2
    ip link set enp0s2 promisc on
    ```
