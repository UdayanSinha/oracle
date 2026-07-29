# Kernel Development - Device Drivers


## Table Of Contents
- [Notes](#notes)
- [Traditional Device Taxonomy](#traditional-device-taxonomy)
- [Unified/Integrated Device Model](#unifiedintegrated-device-model)
- [Modules](#modules)
- [Mechanism vs Policy](#mechanism-vs-policy)
- [Major & Minor Numbers](#major--minor-numbers)
- [File Operations](#file-operations)
- [Character Devices](#character-devices)
- [Interrupts](#interrupts)
- [Timing Measurement](#timing-measurement)
- [PCI/PCIe Device Drivers](#pcipcie-device-drivers)
- [DMA](#dma)
- [Platform Device Drivers](#platform-device-drivers)
- [Network Stack](#network-stack)
- [Block Devices](#block-devices)


## Notes

1. [The Linux Kernel Documentation](https://docs.kernel.org)
2. [Linux source code - Bootlin Elixir Cross Referencer](https://elixir.bootlin.com/linux/latest/source)
    - v6.18.X used as baseline.


## Traditional Device Taxonomy

Devices are typically divided into 3 traditional types, depending on how user-space interactions occur:

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

Character and block devices have filesystem nodes (typically under `devtmpfs`) whereas network devices do not.
They are identified by an interface name instead.

The diagram below shows the interaction of applications with these devices via the kernel:

```
    +--------------------------------------------------------------------+
    |                            Application                             |
    +--------------------------------------------------------------------+
          |                    |                                  |
          v                    v                                  v
    +-------------+     +--------------+                   +-------------+
    |  Character  |     |  Block Node  |                   |    Socket   |
    |    Node     |     |              |                   |  Interface  |
    +-------------+     +--------------+                   +-------------+
          |                    |                                  |          User-space
----------|--------------------|----------------------------------|---------------------
          |                    |                                  |         Kernel-space
          +--------------------+                                  |
                     |                                            v
                     v                             +----------------------------+
          +----------------------------------+     |      Transport Layer       |
          |     Virtual Filesystem (VFS)     |     | (TCP, UDP, ICMP, ARP, etc) |
          +----------------------------------+     +----------------------------+
                  |                    |                          |
                  v                    v                          v
         +----------------+   +----------------+   +----------------------------+
         |    Character   |   |   Filesystem   |   |       Network Layers       |
         |     Driver     |   |     Driver     |   | (routing tables, NAT, etc) |
         +----------------+   +----------------+   +----------------------------+
                  |                    |                          |
                  |                    v                          v
                  |           +---------------+    +-----------------------------+
                  |           |  Block Layer  |    |        Data-Link Layer      |
                  |           +---------------+    | (qdisc, device driver, etc) |
                  |                    |           +-----------------------------+
                  |                    v                          |
                  |           +----------------+                  |
                  |           |  Block Driver  |                  |
                  |           +----------------+                  |
                  |                    |                          |
                  v                    v                          v
         +-----------------------------------------------------------------------+
         |                                Hardware                               |
         +-----------------------------------------------------------------------+
```

In addition to these device types, the driver may additionally incorporate other kernel device controller interfaces.
E.g. a network driver for a NIC may make use of the PCIe subsystem of the kernel if the NIC is a PCIe endpoint device.

Applications interact with device drivers via system calls.
In addition, the kernel may also interact with device drivers for the purpose of IRQ handling, power management, etc.


## Unified/Integrated Device Model

1. Traditional device taxonomy classifies devices based on user-space interaction.
2. Unified Device Model defines how kernel represents devices and their drivers internally.
    - For every device, there is a fundamental struct: `struct device`
        - It includes 2 additional fundamental structs: `struct device_driver` , `struct bus_type`
        - Together, these describe the device, bus it is connected to and the corresponding driver.
    - Information and parameter controls about the devices is exposed via sysfs.
    - Drivers can be registered (`driver_register()`) and de-registered (`driver_unregister()`).
3. Driver code will rarely use these structs or functions directly.
    - Instead the corresponding structs and functions for the specific device type are used, which use the fundamental code internally.
    - E.g. for a PCIe device:
        - `struct pci_dev` includes `struct device` .
        - `struct pci_driver` includes `struct device_driver` .
        - Registration done via `pci_register_driver()` which calls `driver_register()` internally.
        - De-registration done via `pci_unregister_driver()` which calls `driver_unregister()` internally.
4. Note that for power management:
    - Driver code would typically need to implement relevant call-backs (e.g. `suspend()` , `resume()` ) for its operations.
    - Kernel keeps track of dependencies e.g. all PCIe end-point devices must be powered off before attempting to power off the PCIe root port.


## Modules

1. Device drivers may be built as loadable modules or may be built-in.
2. `module_driver()` family of macros are preferred for device drivers instead of standard `module_init()` and `module_exit` interfaces.
    - Avoids repetition of boilerplate initialization and clean-up code.
        - For drivers, these may just call `driver_register()` and `driver_unregister()` respectively.
        - Unless additional operations are needed, `module_driver()` macros will suffice.
    - Each driver type has its own variant, which calls the correct `driver_register()` and `driver_unregister()` variant internally.
        - E.g. `module_pci_driver()` , `module_usb_driver()` , etc.
3. These macros typically take a struct as an argument.
    - E.g. `module_pci_driver()` expects a `struct pci_driver` .
    - Defines driver call-backs (e.g. `probe()` , `remove()`), parameters and other config. relevant to that device driver type.
    - An important aspect of this struct is the ID table of devices that this driver supports.
    - E.g. If a PCIe device is found where the VID:DID combination match what a driver supports, it will be bound to this driver.
        - This assumes that the device is not already bound.
4. Kernel also provides the means to allow easy clean-up of allocated memory in driver code via `devm_*()` functions.
    - E.g. `devm_kzalloc()` .
    - Avoids the need to clean-up during driver unload or detach.
    - Avoids the need to clean-up during failed driver setup (e.g. failure in `probe()`).


## Mechanism vs Policy

*Mechanism* provides the means to access HW.
*Policy* provides directives on how the mechanisms should be utilized.

Generally, a device driver should aim to provide mechanisms, not dictate policy (unless there is a HW limitation or similar).
The reason for this is that the use-cases may evolve/change over time and the original policies may become irrelevant.
In practice, the device driver may be provided with a user-space program or similar that uses the driver mechanisms to help in dictating policy.


## Major & Minor Numbers

1. Device filesystem nodes (character, block) have a set of major and minor numbers that associates them with a specific device driver.
2. The nodes can generally be interacted with like regular files (character) or filesystem images (block).
3. Created with `mknod()` , see [mknod(2) - man](https://man7.org/linux/man-pages/man2/mknod.2.html).
4. In kernel code, `dev_t` encodes the major and minor number combination.
5. A single combination of major and minor number can only have one driver associated with it.
6. Kernel has an internal table of all registered major and minor numbers, and their corresponding drivers.
7. An access to a device node will result in a call into the appropriate device driver.
8. Many device numbers are reserved, see [Allocated Devices - The Linux Kernel Documentation](https://docs.kernel.org/admin-guide/devices.html).


## File Operations

1. `struct file_operations` provides a jump-table of operations related to file management.
    - Provides an entrypoint to the kernel code from user-space system calls (`open()` , `read()` , `write()` , `close()` , etc).
    - Used not just by device drivers, but also by other drivers like filesystem drivers.
    - Not all operations are relevant to a given driver type.
    - Unimplemented operations may result in an error (e.g. `mmap()`) or may call the default kernel-provided operation (e.g. `open()` , `close()`).
    - The operations in this struct may not share the same name as the system call. Mapping for common operations are shown below:

        | `struct file_operations` call-back | System call |
        | ---------- | ------------ |
        | `open()` | `open()` |
        | `read()` | `read()` |
        | `write()` | `write()` |
        | `release()` | `close()` |
        | `mmap()` | `mmap()` |
        | `unlocked_ioctl()` | `ioctl()` |

2. Each filesystem entry has an inode (index node) associated with it, that is represented by `struct inode` .
    - For device filesystem nodes, it is associated with the major and minor number internally.
3. Each time a file/directory is opened, a `struct file` is associated with the file descriptor that is returned to user-space.
    - Associated with the corresponding `struct inode` internally.
    - Created upon `open()` and destroyed upon `close()` .
4. A filesystem entry may have several instances of `struct file` (one per `open()`), but only one `struct inode` .
5. For `mmap()` , `remap_pfn_range()` or `vm_iomap_memory()` is typically used to map the memory region in user-space.

### ioctl()

1. Allows sending and receiving **custom** information for devices by user-space programs.
2. Implemented as device-specific commands.
    - Each command (32-bit) has the following parts:
        - Type (8-bit): Magic number to be used throughout the driver.
        - Number (8-bit): Command number (typically sequential).
        - Size (14-bit): Data transfer size.
        - Direction (2-bit): Direction of data transfer from user-space PoV (read, write, none).
    - Max. size of allowed data transfer is thus set to 16kB (14-bit) by default, but it is possible to be platform-specific.
    - It is recommended to keep the commands as unique as possible.
        - For known commands, see [ioctl numbers - The Linux Kernel Documentation](https://docs.kernel.org/userspace-api/ioctl/ioctl-number.html).
    - Use kernel-provided helpers for command encoding (`_IO()` , `_IOR()` , `_IOW()` , `_IOWR()` , etc).
    - Use kernel-provided helpers for command decoding (`_IOC_DIR()` , `_IOC_TYPE()` , `_IOC_NR` , `_IOC_SIZE()` , etc).
3. While `ioctl()` allows for a variable number of arguments, the number of them needs to match what `unlocked_ioctl()` supports.
    - The variable arguments technique is used to prevent type-checking of the 3rd argument.
4. Excessive use of `ioctl()` is discouraged, as the commands can tend to essentially be new system calls.
3. See [ioctl(2) - man](https://man7.org/linux/man-pages/man2/ioctl.2.html).


## Character Devices

In the kernel, a character device is represented by `struct cdev` .
The setup process for a character device driver is the following:

1. Reserve a major and minor number range (see `dev_t`).
2. Register the driver as a character device driveri (`register_chrdev()` , likewise there is `unregister_chrdev()`).
    - A key aspect of this is the `struct file_operations` which provides the jump-table of driver entrypoints.
3. Perform any device-specific setup.
4. Create filesystem nodes.
    - Can be done manually or automatically (e.g. via `udev` which receives notifications from kernel to trigger node creation/removal).
    - For `udev` , see [udev(7) - man](https://man7.org/linux/man-pages/man7/udev.7.html).

For simple character devices, the drivers may be registered as a miscellaneous character driver.
It can automatically handle most of the steps above, including:

1. Dynamic minor number assignment (major number is fixed for miscellaneous character devices).
2. Filesystem node creation and removal.


## Interrupts

1. In Linux, IRQs can be divided into 2 types:
    - Synchronous IRQs (exceptions):
        - Typically generated by the CPU.
        - E.g. System-calls may initiate switch to kernel-mode via a dedicated instruction that causes an exception.
    - Asynchronous IRQs:
        - Typically generated by other HW devices and fed into the CPU via its IRQ controller.
        - May be maskable (can be disabled) or non-maskable (NMI, cannot be disabled).
2. IRQ handling may be nested, but only one IRQ of a certain type is serviced at any given point.
    - The "type" typically corresponds to a numbered IRQ line on the processor.
3. IRQ handling is not associated with any process context. This means:
    - Sleeping is not allowed.
    - `current` must not be accessed as it is not valid (may refer to previously running task).
    - Scheduler must not be invoked.
    - Module loading should not be requested (e.g. `request_module()`).
    - Data should not be accessed from a process's address-space (includes utilities like `copy_to_user()` and `copy_from_user()`).
4. IRQ handling may be "shared" i.e. multiple handlers can be registered for a given IRQ type and they will be called one by one.
    - The IRQ can therefore not be "consumed" by any single handler.
    - All handlers should agree to the sharing during handler registration.
5. IRQ handling for a given IRQ type can be set to occur on specific CPUs in an SMP system via affinity settings (see general design guide for details).
6. Installed IRQs and their statistics can be checked here: `/proc/interrupts`
7. Following interfaces are important w.r.t IRQs:
    - `request_irq()` (and variants):
        - Register an IRQ handler for a specified IRQ.
        - Must be called **BEFORE** the device is ready to generate that IRQ.
    - `free_irq()` (and variants):
        - De-register an IRQ handler for a specified IRQ.
        - Must be called **AFTER** a device is no longer generating that IRQ.
    - `enable_irq()` , `disable_irq()` (and variants):
        - Enable or disable IRQs of a given type (on all CPUs).
        - This is different from the enable/disable done by synchronization primitives like spin-lock, which only disables IRQ handling on a given CPU.
        - These functions have depth e.g. if `disable_irq()` was called 3 times to disable an IRQ, then `enable_irq()` must also be called 3 times to enable it back.
8. For MSI/MSI-x in PCI-e devices, see [MSI/MSI-x - The Linux Kernel Documentation](https://docs.kernel.org/PCI/msi-howto.html).

### SoftIRQ

1. SW-triggered IRQ mechanism, which may itself trigger as a result of a HW IRQ.
2. Used to handle specific kernel functions like network activity (TX/RX), scheduling ticks, etc.
    - Each softIRQ source has an assigned priority in the kernel.
3. SoftIRQ statistics can be checked here: `/proc/softirqs`
4. SoftIRQ handlers may be executed in the following ways:
    - Via kernel checks for pending softIRQs (e.g. after execution of a IRQ).
    - Via scheduling-in of `ksoftirqd/*` tasks.
5. If pending, softIRQs are executed in priority order.

### Deferred Handling

1. IRQ handling may be split into top and bottom halves:
    - Top half is executed ASAP, as part of the IRQ handler call-back.
    - Bottom half (if used) may be used to execute deferred (usually less urgent) operations at a later point.
2. Bottom half can be implemented in 3 ways:
    - Tasklets:
        - Runs in atomic context based on triggering via softIRQs.
        - Runs only on the CPU where the request was made.
        - Multiple tasklets execute in parallel, but the same tasklet can only be run one at a time.
        - Invoked as a result of kernel checks at specific points (e.g. scheduling tick IRQs, user-kernel mode transition, etc), if needed.
        - It is a general deferred execution mechanism i.e. they can be used for non-IRQ related work too.
        - Considered deprecated, but significant legacy driver code still use it today.
    - Workqueues:
        - Runs in process context via kthreads (pool of `kworker/*`).
        - Scheduled like any other task.
        - Depending on the workqueue settings, submitted work may run on a specific CPU (bound) or on any CPU (unbound).
        - It is a general deferred execution mechanism i.e. it can be used for non-IRQ related work too.
    - Threaded IRQs:
        - Runs in process context via kthreads (pool of `irq/*`).
        - While regular kthread interfaces can be used for this, kernel provides a dedicated set of interfaces to setup threaded IRQ handling.
        - Scheduled like any other task.


## Timing Measurement

1. Linux systems support traditional Unix-style of measuring time since the "epoch" (January 1st, 1970).
2. Clock sources are HW-specific.
    - Popular clock sources in x86 include Time Stamp Counter (TSC) and High Precision Event Timer (HPET).
        - TSC can be read via `rdtsc` instruction (also has a corresponding helper in kernel code).
    - Clock source for kernel's primary timekeeping can be specified via `clocksource` on the kernel command-line.
        - See [Kernel Parameters - The Linux Kernel Documentation](https://docs.kernel.org/admin-guide/kernel-parameters.html).
        - Clock source information on a running system can be found at `/sys/devices/system/clocksource/clocksource<n>` .
3. Timer frequency for scheduling ticks is defined by `CONFIG_HZ` .
    - Higher frequencies may potentially improve kernel responsiveness, but can also increase overhead due to more frequent timer IRQs (more time spent in kernel mode).
4. In kernel code, measurement is provided by:
    - Jiffies.
    - Kernel timers.
    - Delay functions equivalent to `sleep()` or similar in user-space.
        - E.g.`msleep()` (will sleep), `mdelay()` (will busy-wait).
5. Several system-calls exist for timing measurements in user-space. Refer relevant man pages.
    - E.g. `alarm()` , `clock_gettime()` , `clock_settime()` , `getitimer()` , `setitimer()` , etc.
6. Use of tickless scheduling is beneficial for specific workloads. See general design guide for details.
    - Note that not all CPUs can be set to use tickless mode. Typically, the bootstrap CPU cannot be set to tickless.

### Jiffies

1. Global counter shared by all CPUs.
2. Conceptually a counter of scheduling ticks (typically IRQ #0 per-CPU).
    - In reality, it may not match the tick frequency at all, especially for tickless scheduling.
3. 64-bit wide counter (`jiffies`) on 64-bit systems.
4. 32-bit (`jiffies`, lower 32-bit) and 64-bit (`jiffies_64`) wide counters on 32-bit systems.

### Kernel Timers

1. Delays execution of a certain function until a specified time period has elapsed.
    - Function will execute in atomic context (softirq).
    - Function will execute on the CPU on which the timer was started.
        - Note that it is possible that there is a task migration to another CPU inside the kernel function which starts the timer.
        - In that case, the CPU on which the function executes will be the CPU on which the task was migrated to.
    - CPU may not be immediately available when the time period elapses i.e. there may additional delay before the function is executed.
2. Typical use of kernel timers is of 2 types:
    - *Timeout* use-case:
        - Timer-callback will execute only if an event does not occur within a specific time window.
        - In most cases, the timer-callback is never run.
        - Generally relaxed time resolution requirements.
    - *Timer* use-case:
        - Timer-callbacks are actually intended to run.
        - Generally stricter time resolution requirements.
3. Kernel provides following timer types:
    - Low-resolution timers.
        - Uses `jiffies` underneath, hence will only provide equivalent time resolution.
        - Supports one-shot use only, but can be re-installed recursively if periodic triggering is needed.
        - Efficient timer creation and deletion.
        - Better suited for *timeout* use-cases.
    - High-resolution timers.
        - Nanosecond time resolution.
        - Supports one-shot and periodic use.
        - Higher overhead in timer creation and deletion compared to low-resolution timers.
        - Better suited for *timer* use-cases.
4. Information about currently used timers in the system is available in `/proc/timer_list` .


## PCI/PCIe Device Drivers

1. Drivers provide a list of relevant device IDs via `module_pci_driver()` .
2. Matched IDs during enumeration will result in `probe()` call-back being called in the driver, to perform necessary setup. Common actions include:
    - Calling `pci_enable_device()` which includes (if not already done) enabling access to memory and I/O regions.
    - Calling `pci_set_master()` to enable Bus Master capability.
    - Setting up IRQ handling in the driver.
    - Claim ownership of the BARs via `pci_request_regions()` .
        - If kernel code will access this memory, it will also need to be mapped into kernel virtual address space via `pci_iomap()` .
    - Setting up applicable user-space interfaces (e.g. registering as a network/character/block device).
    - Other device-specific setup.
3. `probe()` will also be called if a driver is manually attached to a device (done via sysfs).
4. Likewise, `remove()` will be called to perform necessary clean-up for these situations:
    - Device (hot-)unplug.
    - Driver unload.
    - Manual detach (done via sysfs) of a driver from a device.
5. Configuration space access is provided via `pci_read_config_*()` and `pci_write_config_*()` functions.
    - E.g. `pci_read_config_byte()` , `pci_write_config_dword()` , etc.
6. BAR ranges and flags can be retrieved via `pci_resource_*()` functions.
    - E.g. `pci_resource_start()` , `pci_resource_end()` , `pci_resource_flags()` , etc.
    - They should not be read from the configuration space directly.
7. If `mmap()` of BAR regions is needed, it can be done by passing BAR start and end addresses to `vm_iomap_memory()` or `remap_pfn_range()` .
8. See:
    - [System Address Map Initialization in x86_64 PCI/PCIe-based Systems #1](https://www.infosecinstitute.com/resources/hacking/system-address-map-initialization-in-x86x64-architecture-part-1-pci-based-systems).
    - [System Address Map Initialization in x86_64 PCI/PCIe-based Systems #2](https://www.infosecinstitute.com/resources/reverse-engineering/system-address-map-initialization-x86x64-architecture-part-2-pci-express-based-systems)
    - [PCI/PCIe Device Drivers - The Linux Kernel Documentation](https://docs.kernel.org/PCI/pci.html).


## DMA

1. DMA setup is typically very platform-specific, and kernel tries to provide an abstract interface for data transfer handling.
2. DMA-based data transfer requires a physcially contiguous memory (i.e. no paging involved) that is reachable by the device.
3. IRQs are typically involved to signal start and end of data transfer (or even data availability) between the CPU and the device.
4. Besides any platform-specific steps, a buffer needs to be setup for the data transfer.
    - This buffer needs to be mapped both to kernel virtual address space and the bus of the device.
5. Kernel supports 2 types of DMA buffer mappings:
    - Synchronous/coherent:
        - Higher overhead, intended for buffers that persist throughout the lifecycle of the driver.
        - Involves use of `dma_alloc_coherent()` , `dma_free_coherent()` and associated interfaces.
            - For `dma_alloc_coherent()` , the return value is the kernel virtual address.
        - The buffer can be accessed in parallel by both the CPU and the device w/o cache coherency issues.
            - Memory barriers may still be needed to account for CPU instruction reordering.
        - The minimum buffer allocation size is generally a page.
    - Asynchronous/streaming:
        - Lower overhead, intended for one-time operations.
        - Involves use of `dma_map_single()` , `dma_unmap_single()` and associated interfaces.
            - `dma_map_single()` takes a pre-allocated memory that must be addressable by the device.
        - Direction of data transfer must match the value provided during mapping.
            - `DMA_NONE` is intended for debugging, any attempt to use the memory may cause an error.
        - Once mapped, the buffer must not be touched by the CPU until it has been unmapped:
            - For a write to the device, the data must be placed in the buffer before mapping.
            - For a read from the device, the data must be read from the buffer after unmapping.
    - Note that `dma_addr_t` represents the physical address on the bus of the device, and is intended to be used opaquely in kernel code.
6. Similar to `struct kmem_cache` , kernel provides the ability to setup DMA pools as well.
    - For frequent **coherent** DMA transfers where the minimum-one-page restriction of `dma_alloc_coherent()` is not reasonable.
    - See `struct dma_pool` and its interfaces.
7. Kernel also provides a way to perform parallel DMA transfers from several buffers (scatter/gather), each of which is intended for different (but contiguous) address regions.
    - Generally requires support in HW.
    - For **streaming** DMA transfers only.
    - See `struct scatterlist` and associated interfaces (e.g. `dma_map_sg()` , `dma_unmap_sg()` , `sg_set_page()`).
8. Note that specific driver subsystems may have their own DMA interfaces that call these general interfaces underneath.
9. See [DMA Mapping API- The Linux Kernel Documentation](https://docs.kernel.org/core-api/dma-api-howto.html).


## Platform Device Drivers

1. Devices on major interfaces like PCIe/USB have defined methods of device discovery and removal.
    - These result in corresponding `probe()` and `remove()` call-backs in the matching driver being called.
    - Device matching done via defined IDs for the device.
    - The matched driver would have provided these IDs previously, usually as part of `module_<device-type>_driver()` .
2. Many devices (typically built into a SoC) are directly addressible by the CPU.
    - E.g. LED devices, host bus controllers, etc.
    - Collectively called *platform* devices.
3. Platform driver interface provides a way to attach/detach drivers to such devices in a way that mimics the approach for devices on a major interface like PCIe.
4. Note that the ID matching mechanism can vary. It may be done based on:
    - Hardcoded data in the kernel source code. Not preferred as it makes the kernel build be specific to that device.
    - Device trees.
    - ACPI.
5. See [Platform Device Drivers - The Linux Kernel Documentation](https://docs.kernel.org/driver-api/driver-model/platform.html).

### Device Tree

1. Represents integration of HW blocks i.e. connection between them, not their internal details.
    - Essentially a tree of nodes.
    - Each node represents a device or HW block.
    - Properties of the node represent the characteristics of the device or HW block.
2. Written in `.dts` (device tree source) files, which are compiled into `.dtb` (device tree binary/blob) files.
3. The information can be split into `.dtsi` files which are then included into `.dts` files (device tree inheritance) for modularity.
    - Note that the contents of a `.dts` file is *applied onto* the contents of the included `.dtsi` files.
    - E.g. Device tree of a SoC may be in a `.dtsi` file which is included in a`.dts` file describing the board.
4. Can be updated at runtime via device tree *overlays* (essentially device tree snippets).
5. OS-agnostic in principle. However the source files can typically be found in the kernel source tree (and built as part of kernel build): `arch/<ISA>/boot/dts`
6. Can be passed by the bootloader/firmware to the kernel at start-up, or built into the kernel image itself (`CONFIG_BUILTIN_DTB`).
7. Matched to drivers based on the `compatible` field in the device trees.
8. For the base specification, see [Specifications - Device Tree](https://www.devicetree.org/specifications).
    - Only describes the base syntax and some common properties.
9. Kernel provides further guidelines on how to describe different device types (device tree bindings).
    - See [Firmware & Device Trees - The Linux Kernel Documentation](https://docs.kernel.org/devicetree/index.html).
10. For further guidance, see [Device Trees 101: Linux Foundation](https://www.youtube.com/watch?v=TeygIvk4UHg&pp).

### Advanced Configuration & Power Interface (ACPI)

1. Provides the means to describe hardware and support OS-driven power management operations.
2. Access provided to the kernel by the firmware.
3. OS-agnostic in principle.
4. For power management, ACPI defines several power states on different levels (i.e. system/global-level, device-level, CPU-level, etc).


## Network Stack

The network stack in Linux follows a layered approach which maps to the OSI model.

For sending data (application -> network), the general flow is as follows:

1. The application writes some data over the socket interface (`send()` or similar).
2. The transport layer will creates a set of one or more packets with added control information, and forwards it to the network layer.
    - Depending on the transport type, it may be connection-oriented (e.g. TCP) or connection-less (e.g. UDP).
    - This layer is also responsible for transport-level error-handling (e.g. due to lost packets).
3. The network layer adds additional control information and routes it towards the correct destination.
    - The destination is expected to be reachable over one of the network interfaces.
4. The network driver associated with the applicable network interface will receive the packet and transfer it to the network HW for transmission (e.g. via DMA).

For receiving data (network -> application), the general flow is as follows:

1. The network HW receives a packet, which is notified to the driver (e.g. via IRQs) and retrieved from it (e.g. via DMA).
2. After verification, the packet is forwarded to the network and transport layers respectively, which may perform additional verification.
3. The application is able to read the data over the socket interface (`recv()` or similar).

Note the following:

- For local machine communication (e.g. loopback device, local TCP), the packet will typically not proceed into the data-link layer at all.
- For UDS (Unix Domain Socket) communication, it will bypass the network layers entirely.
- An application is generally independent of the other layers and does not know which network interface will be used for its purpose.
    - Specialized applications (e.g. network monitoring and debugging) are an exception.

### Network Drivers

1. Network devices are represented by `struct net_device` .
    - Can be allocated and freed with `alloc_netdev()` and `free_netdev()` .
    - To initialize most struct fields, there are convenience functions available.
    - E.g. `struct net_device` can be allocated for Ethernet-based devices with `alloc_etherdev()` , which will initialize the struct fields accordingly.
2. Operations related to the device can be defined as call-backs via `struct net_device_ops` .
    - Common call-backs include:
        - `ndo_open()`
        - `ndo_stop()`
        - `ndo_start_xmit()`
        - `ndo_do_ioctl()`
        - `ndo_get_stats64()`
        - `ndo_tx_timeout`
    - As mentioned before, unlike character/block devices, there are no filesystem nodes for a network device.
        - Call-backs like `ndo_open()` , `ndo_stop()` and `ndo_do_ioctl()` are called from user-space based on `ioctl()` calls on the socket file descriptor.
3. The struct is registered and un-registered with `register_netdev()` and `unregister_netdev()` .
4. The device transmit queue can be managed with the following:
    - Start the queue: `netif_start_queue()`
    - Stop the queue: `netif_stop_queue()`
    - Resume the queue from temporary shutdown: `netif_wake_queue()`
5. Packets are represented by `struct sk_buff` .
    - As done with `struct net_device` , there are convenience functions available for most struct fields.
    - Broadly speaking, this struct is essentially metadata around the actual data.
    - For more details about the layout of the struct, see [sk_buff - The Linux Kernel Documentation](https://docs.kernel.org/networking/skbuff.html).
6. `struct net_device` also maintains packet statistics, which need to be updated (typically done in `struct net_device_ops` call-backs).
7. The state information in the `struct net_device` may need to be updated due to changes in carrier state (e.g. plugging a cable in or out). See:
    - `netif_carrier_on()`
    - `netif_carrier_off()`
    - `netif_carrier_ok()`
8. Each socket file descriptor has a corresponding `struct socket` associated with it.
    - Among other things, it includes a pointer to a `struct file` and `struct proto_ops` .
    - Note that there is a `ioctl()` call-back in `struct proto_ops` too, which handles socket-level operations.
    - Device-level operations are handled by `ndo_do_ioctl()` in `struct net_device` .
9. `ioctl()` commands (both socket-level and device-level operations) are defined in `include/uapi/linux/sockios.h` .
    - Custom device-level `ioctl()` commands can be defined, see `SIOCDEVPRIVATE` .
    - Custom protocol-level `ioctl()` commands can be defined, see `SIOCPROTOPRIVATE` .
10. `ip` can be used to manage configuration of network interfaces from user-space, see [ip(8) - man](https://man7.org/linux/man-pages/man8/ip.8.html).


## Block Devices

1. As mentioned earlier, random access of data is typically allowed.
2. Access is typically cached (page cache), unless explicitly requested otherwise (e.g. `O_SYNC` via `open()`).
    - Likewise, write-backs can be forced via `sync()` , `fsync()` or similar.
3. Regardless of the amount of data requested by the system call, the low-level access is in-terms of multiple of a fixed *block* size.
4. Block device driver registration is done via `register_blkdev()` (likewise there is `unregister_blkdev()`).
    - Major number for the device node is specified via `register_blkdev()` .
    - Note that this is no longer needed to be called in most cases for newer kernels.
5. `struct gendisk` describes a partitionable device. Key elements include:
    - `major` : Major number associated with the device.
    - `disk_name` : Device node name w/o the partition number (e.g. `sda`).
    - `fops` : Points to `struct block_device_operations` (equivalent to `struct file_operations`).
    - `queue` : Points to `struct request_queue` . Tracks pending operations for the device (shared by all partitions on the device).
    - `flags` : Device control flags (see `GENHD_FL_*`).
6. Each device node will have a corresponding `struct gendisk` .
    - Nodes are named like this: `<major-name><unit><partition>`
    - E.g. `/dev/sda2` , where `sd` is the major name, `a` is the (first) unit, and `2` is the partition.
    - Note that a block device may have multiple partitions.
7. There is no `read()` or `write()` operations to be implemented as part of `struct block_device_operations`:
    - That is handled by the filesystem and block layers.
        - If the page cache is applicable in this case, it will be checked first.
    - E.g. The filesytem layer will map `read(fd, ...)` to *read x bytes from sector y* (block I/O or BIO) towards the block layer.
        - Each BIO is described by `struct bio` .
    - The block layer will manage any necesssary merging and reordering of access requests (for efficency), and make *requests* towards the block device driver.
        - See `queue_rq()` in `struct blk_mq_ops` .
    - The requests will be put on the driver request queue and be processed (e.g. via storage controller commands).
        - Each request is described by `struct request` .
    - If the HW is capable of parallel requests, there may be parallel request queues in the driver too.
8. Besides request queues, there are also means to submit BIOs directly to the driver.
    - See `submit_bio()` in `struct block_device_operations` .
    - Note that `queue_rq()` still needs to implemented, even if it is unused in this case.
