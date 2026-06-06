# Kernel Development - Device Drivers


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


## Modules

1. Device drivers may be built as loadable modules or may be built-in.
2. `module_driver()` family of macros are preferred for device drivers instead of standard `module_init()` and `module_exit` interfaces.
    - Avoids repetition of boilerplate initialization and clean-up code.
    - Each driver type has its own variant. E.g. `module_pci_driver()` , `module_usb_driver()` , etc.
3. These macros typically take a struct as an argument.
    - E.g. `module_pci_driver()` expects a `struct pci_driver` .
    - Defines driver call-backs, parameters and other config. relevant to that device driver type.
    - An important aspect of this struct is the ID table of devices that this driver supports.
    - E.g. If a PCIe device is found where the VID:DID combination match what a driver supports, it will be loaded into the kernel if not already loaded or built-in.


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
        | `release()` | `release()` |
        | `mmap()` | `mmap()` |
        | `unlocked_ioctl()` | `ioctl()` |

2. Each filesystem entry has an inode (index node) associated with it, that is represented by `struct inode` .
    - For device filesystem nodes, it is associated with the major and minor number internally.
3. Each time a file/directory is opened, a `struct file` is associated with the file descriptor that is returned to user-space.
    - Associated with the corresponding `struct inode` internally.
    - Created upon `open()` and destroyed upon `close()` .
4. A filesystem entry may have several instances of `struct file` (one per `open()`), but only one `struct inode` .

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
    - Use kernel-provided helpers for command decoding (`_IOC_DIR()` , `IOC_TYPE()` , `IOC_NR` , `IOC_SIZE()` , etc).
3. While `ioctl()` allows for a variable number of arguments, the number of them needs to match what `unlocked_ioctl()` supports.
    - The variable arguments technique is used to prevent type-checking of the 3rd argument.
4. Excessive use of `ioctl()` is discouraged, as the commands can tend to essentially be new system calls.
3. See [ioctl(2) - man](https://man7.org/linux/man-pages/man2/ioctl.2.html).


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
    - CPU may not be immediately available when the time period elapses i.e. there may additional delay before function is executed.
2. Typical use of kernel timers is of 2 types:
    - *Timeout* use:
        - Timer-callback will execute only if an event does not occur within a specific time window.
        - In most cases, the timer-callback is never run.
        - Generally relaxed time resolution requirements.
    - *Timer* use:
        - Timer-callabcks are actually intended to run.
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
