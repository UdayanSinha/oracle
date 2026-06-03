# Kernel Debugging Guide


## References

1. [The Linux Kernel Documentation](https://docs.kernel.org)
2. [Linux source code - Bootlin Elixir Cross Referencer](https://elixir.bootlin.com/linux/latest/source)
    - v6.18.X used as baseline.


## Defconfig Options

1. Enable these options for debugging:
    - `CONFIG_EARLY_PRINTK` : Enable use of early printk() to detect kernel
    failures before console is initialized.
    - `CONFIG_DEBUG_KERNEL` : Enable kernel debugging.
    - `CONFIG_DETECT_HUNG_TASK` : Enable identification of kernel-freeze culprits.
    - `CONFIG_DEBUG_INFO` : Allows decoding of kernel OOPS symbols.
    - `CONFIG_LOG_BUF_SHIFT=21` : Sets the kernel buffer log size to the biggest
    possible value.
    - `CONFIG_PRINTK_TIME` : Enables time-stamping when using printk().
2. The defconfig for the running kernel may be possible to examine.
    - This requires that the kernel was built with `CONFIG_IKCONFIG_PROC` enabled.
    - The location varies depending on the installation, but is usually found
    in one of the following places:
        - `/proc/config.gz`
        - `/boot/config`
        - `/boot/config-$(uname -r)`


## Kernel Command-Line Parameters

1. Can be reviewed in `/proc/cmdline` for the current environment.
2. `CONFIG_CMDLINE` will reflect defconfig-specified parameters.
3. Should be possible to modify at run-time via bootloader shell.


## Dmesg & Syslog

1. Kernel message buffer can be viewed with the `dmesg` utility. See [dmesg(1) - man](https://man7.org/linux/man-pages/man1/dmesg.1.html).
2. Syslog location varies depending on the installation.
3. It is possible to have rate-limiting and delays on `printk()` , see settings in `sysctl` .


## Basic System Information

1. CPU information: `/proc/cpuinfo`
    - Cache information: `/sys/devices/system/cpu/cpu<n>/cache`
2. Memory information:
    - `/proc/meminfo`
    - `/proc/buddyinfo`
    - `/proc/slabinfo`
    - `/proc/zoneinfo`
    - `/proc/vmallocinfo`
    - `free`
    - RSS/PSS information for processes
3. Interrupt status:
    - `/proc/interrupts`
    - `/proc/softirqs`
4. Kernel taint status: `/proc/sys/kernel/tainted`
5. Kernel symbol table: `/proc/kallsyms`
6. Kernel parameters: `sysctl -a`
    - See [sysctl(8) - man](https://man7.org/linux/man-pages/man8/sysctl.8.html).
7. Supported filesystems: `/proc/filesystems`
8. Swap areas: `/proc/swaps`
9. Timer information: `/proc/timer_list`


## Boot Firmware Information

1. Check settings for relevant boot firmware (BIOS, UEFI, U-Boot, etc).
2. Kernel also provides some information here: `/sys/firmware`


## Tracing In Code

1. Other than `printk()`, OOPS messages are also a useful source of debug info.
    - See `WARN()` , `BUG()` and its variants.
2. Note that `WARN()` and its variants will result in an OOPS message w/o further action.
3. `BUG()` and its variants will result in an OOPS message and may kill the user-space process (process context).
4. `/proc/sys/kernel/panic_on_oops` or `CONFIG_PANIC_ON_OOPS` will affect behavior of `BUG()` .
5. It is also possible to call `panic()` , which will result in an OOPS message and cause a crash.


## strace

Use strace for system call usage profiling in user-space programs.
See [strace(1) - man](https://man7.org/linux/man-pages/man1/strace.1.html).


## perf

Use perf for performance and latency profiling.
It can analyze both user-space and kernel-space code.


## powertop

Use powertop for power consumption profiling.


## stress-ng

Use stress-ng for stress tests.


## Process Address Space

Mapping information of a process address space can be viewed here:
- List of regions: `/proc/<PID>/maps`
- Memory usage statistics for each region: `/proc/<PID>/smaps`


## Dynamic Debug

Enable dynamic debug, see [Dynamic debug - The Linux Kernel documentation](https://docs.kernel.org/admin-guide/dynamic-debug-howto.html).

E.g. To see pr_debug() logs for a kernel module: `echo "module <module_name> +p" > /sys/kernel/debug/dynamic_debug/control`


## Event-Tracing (kprobes)

Enable event-tracing, see [Event Tracing - The Linux Kernel documentation](https://docs.kernel.org/trace/events.html).

E.g. Trace context-switch scheduling events as shown below.

```console
echo 0 > /sys/kernel/debug/tracing/tracing_on                          # disable tracing
echo > /sys/kernel/debug/tracing/trace                                 # flush previous traces
echo 1 > /sys/kernel/debug/tracing/events/sched/sched_switch/enable    # enable context-switch event traces
echo 1 > /sys/kernel/debug/tracing/tracing_on                          # enable tracing
```

Trace logs can be found at: `/sys/kernel/debug/tracing/per_cpu/cpu<n>/trace`


## Memory-Profiling

1. To use kmemleak, see [Kernel Memory Leak Detector - The Linux Kernel documentation](https://docs.kernel.org/dev-tools/kmemleak.html).
2. To use KASAN, see [Kernel Address Sanitizer (KASAN) - The Linux Kernel documentation](https://docs.kernel.org/dev-tools/kasan.html).


## SysRq

Use sysrq, as explained in [Linux Magic System Request Key Hack - The Linux Kernel documentation](https://docs.kernel.org/admin-guide/sysrq.html).

E.g. Trigger kernel panic: `echo c > /proc/sysrq-trigger`


## Lockdep

Enable lock-validation via lockdep. See [Runtime locking correctness validator - The Linux Kernel documentation](https://docs.kernel.org/locking/lockdep-design.html).

`Hacking Options` in the defconfig lists lockdep settings.


## kdump

If available, use kdump to troubleshoot kernel panics by analyzing the system memory dump.

See [Documentation for Kdump - The kexec-based Crash Dumping Solution](https://docs.kernel.org/admin-guide/kdump/kdump.html).
