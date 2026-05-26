# Kernel Development - Device Drivers


## Notes

1. [The Linux Kernel Documentation](https://docs.kernel.org)
2. [Linux source code - Bootlin Elixir Cross Referencer](https://elixir.bootlin.com/linux/latest/source)
    - v6.18.X used as baseline.


## Device Types

Typically divided into 3 basic types:

1. Character devices
    - Read/write access is in terms of (typically sequential) byte streams.
    - Accessed using file operation system calls (`open()` , `read()` , `write()` , `close()`).
    - E.g. UART devices.
2. Block devices
    - Read/write access is in block-sized multiples (typically random access is permitted).
    - Filesystems can typically be mounted on the device.
    - E.g. Storage devices.
3. Network devices
    - Read/write access is in terms of packets of data.
    - Accessed typically via socket interfaces.
    - E.g. Network Interface Controller (NIC) devices.

Character and block devices typically have nodes under `devtmpfs` whereas network devices do not have such a node.
They are identified by an interface name instead.

The diagram below shows the interaction of applications with these devices via the kernel:

```
    +------------------------------------------------------------------+
    |                           Application                            |
    +------------------------------------------------------------------+
          |                    |                       |
          v                    v                       v
    +-------------+     +-------------+         +------------+
    |  character  |     | block node  |         |   socket   |
    |    node     |     |             |         |            |
    +-------------+     +-------------+         +------------+
          |                    |                       |            User-space
----------|--------------------|-----------------------|-----------------------
          |                    |                       |           Kernel-space
          +--------------------+                       |
                     |                                 |
                     v                                 |
            +----------------------------------+       |
            |     Virtual Filesystem (VFS)     |       |
            +----------------------------------+       |
                  |                    |               |
                  v                    v               v
         +----------------+   +----------------+   +----------------+
         |    character   |   |   filesystem   |   | network layers |
         |     driver     |   |     driver     |   | (TCP/IP, etc)  |
         +----------------+   +----------------+   +----------------+
                  |                    |                    |
                  |                    v                    v
                  |            +---------------+   +----------------+
                  |            |  block layer  |   | network driver |
                  |            +---------------+   +----------------+
                  |                    |                    |
                  |                    v                    |
                  |           +----------------+            |
                  |           |  block driver  |            |
                  |           +----------------+            |
                  |                    |                    |
                  v                    v                    v
              +-------------------------------------------------+
              |                     Hardware                    |
              +-------------------------------------------------+
```

In addition to these device types, the driver may additionally incorporate other kernel device controller interfaces.
E.g. a network driver for a NIC may make use of the PCIe subsystem of the kernel if the NIC is a PCIe endpoint device.

Applications interact with device drivers via system calls.
In addition, the kernel may also interact with device drivers for the purpose of IRQ handling, power management, etc.


## Mechanism vs Policy

*Mechanism* provides the means to access HW.
*Policy* provides directives on how the mechanisms should be utilized.

Generally, a device driver should aim to provide mechanisms, not dictate policy (unless there is a HW limitation or similar).
The reason for this is that the use-cases may evolve/change over time and the original policies may become irrelevant.
In practice, the device driver may be provided with a user-space program or similar that uses the driver mechanisms to help in dictating policy.


## Modules

1. Device drivers may be built as loadable modules or may be built-in.
2. `module_driver()` family of macros are preferred for device drivers instead of standard `module_init()` and `module_exit` interfaces.
    - Avoids repetition of boilerplate initialization and clean-up code.
    - Each driver type has its own variant. E.g. `module_pci_driver()` , `module_usb_driver()` , etc.
3. These macros typically take a struct as an argument.
    - E.g. `module_pci_driver()` expects a `struct pci_driver` .
    - Defines driver call-backs, parameters and other config. relevant to that device driver type.
4. An important aspect of this struct is the ID table of devices that this driver supports.
5. E.g. If a PCIe device is found where the VID:DID combination match what a driver supports, it will be loaded into the kernel if not already loaded or built-in.


## Major & Minor Numbers

1. Device filesystem nodes (character, block) have a set of major and minor numbers that associates them with a specific device driver.
2. The nodes can generally be interacted with like regular files (character) or filesystem images (block).
3. Created with `mknod()` , see [mknod(2) - man](https://man7.org/linux/man-pages/man2/mknod.2.html).
4. In kernel code, `dev_t` encodes the major and minor number combination.
5. A single combination of major and minor number can only have one driver associated with it.
6. Kernel has an internal table of all register major and minor numbers, and their corresponding drivers.
7. An access to a device node will result in a call into the appropriate device driver.
8. Many device numbers are reserved, see [Allocated Devices - The Linux Kernel Documentation](https://docs.kernel.org/admin-guide/devices.html).


## File Operations

1. `struct file_operations` provides a jump-table of operations related to file management.
    - Provides an entrypoint to the kernel code from user-space system calls (`open()` , `read()` , `write()` , `close()` , etc).
    - Used not just by device drivers, but also by other drivers like filesystem drivers.
    - Not all operations are relevant to a given driver type.
    - Unimplemented operations may result in an error (e.g. `mmap()`) or may call the default kernel-provided operation (e.g. `open()` , `close()`).
    - The operations in this struct may not share the same name as the system call. E.g. operation for `close()` is `release()` .
2. Each filesystem entry has an inode (index node) associated with it, that is represented by `struct inode` .
    - For device filesystem nodes, it is associated with the major and minor number internally.
3. Each time a file/directory is opened, a `struct file` is associated with the file descriptor that is returned to user-space.
    - Associated with the corresponding `struct inode` internally.
    - Created upon `open()` and destroyed upon `close()` .
4. A filesystem entry may have several instances of `struct file` (one per `open()`), but only one `struct inode` .


## Character Devices

The setup process for a character device driver is the following:

1. Reserve a major and minor number range.
2. Register the driver as a character device driver.
    - A key aspect of this is the `struct file_operations` which provides the jump-table of driver entrypoints.
3. Perform any device-specific setup.
4. Create filesystem nodes.
    - Can be done manually or automatically (e.g. via `udev` which receives notifications from kernel to trigger node creation/removal).
    - For `udev` , see [udev(7) - man](https://man7.org/linux/man-pages/man7/udev.7.html).

For simple character devices, the drivers may be registered as a miscellaneous character driver.
It can automatically handle most of the steps above, including:

1. Dynamic minor number assignment (major number is fixed for miscellaneous character devices).
2. Filesystem node creation and removal.
