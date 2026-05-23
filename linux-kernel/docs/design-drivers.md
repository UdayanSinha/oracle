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
