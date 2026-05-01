# Kernel Development - General


## References

1. [The Linux Kernel Documentation](https://docs.kernel.org)
2. [Linux source code - Bootlin Elixir Cross Referencer](https://elixir.bootlin.com/linux/latest/source)


## Config. + Build

1. Download kernel source tree (e.g. from [The Linux Kernel Archives](https://kernel.org)).

2. Setup the build environment.

3. Select a kernel defconfig. (see `arch/<ISA>/configs/`). E.g. for x86_64

    ```console
    make help    # if usage info is needed
    make x86_64_defconfig
    make <menuconfig, nconfig>    # to make changes
    # .config file created at the end which contains finished defconfig.
    ```

4. Compile the kernel.

    ```console
    make
    ```

5. See [Kernel Build System - The Linux Kernel documentation](https://docs.kernel.org/kbuild/index.html).


## Modules

1. A module, once inserted, is part of the kernel and will run at the same privilege level.

2. `module_init()` and `module_exit()` defines the init (called upon module load) and exit (called upon module unload) functions.
    - A module w/o an exit function cannot be unloaded.
    - Built-in modules do not have functions referenced by `module_exit()` compiled in.

3. Functions marked with`__init` and data marked with `__initdata` are discarded after kernel initialization.

4. To compile modules out-of-tree (OOT), it requires a previously compiled kernel source tree.

```console
make -C /path/to/kernel-tree M=/path/to/oot-module
```

5. Loading an OOT module taints the kernel.

6. Modules can access kernel interfaces exported via `EXPORT_*()`.
    - E.g. `EXPORT_SYMBOL()` , `EXPORT_SYMBOL_GPL()` (for modules with closed licenses).
    - Can be done by modules for other modules too.
    - Symbol use by the kernel is resolved at boot. Therefore, it will not be able to access symbols exported by kernel modules which are loaded afterwards.
    - Symbol use by a module is resolved when it is loaded. This may introduce load dependencies across modules.
    - All exported symbols are present in `/proc/kallsyms` .

7. Use module utilies like `modprobe` , `insmod` , `lsmod` and `rmmod` to manage module use.


## Coding Conventions

1. See [Linux Kernel Coding Style](https://docs.kernel.org/process/coding-style.html).

2. Use `indent` to enforce coding style.
    - It has a built-in style for Linux Kernel use.

    ```console
    indent /path/to/file -linux
    ```

    - See [indent(1) - man](https://man7.org/linux/man-pages/man1/indent.1.html).

3. Use `sparse` for code static analysis (other tools may also be applicable).
    - See [sparse(1) - man](https://man7.org/linux/man-pages/man1/sparse.1.html).
    - Invoke during compilation of the kernel.

    ```console
    make C=1 ...    # only specific module
    make C=2 ...    # all files
    ```

4. Use `likely()` and `unlikely()` where possible, to aid in branch prediction.

5. User-space parameters (e.g. system call arguments) should not be trusted by default.
    - User-space pointers may be invalid, or the pages may have been swapped.
    - Use `copy_to_user()` and `copy_from_user()` instead.
    - Kernel never swaps out its own pages and always allocates its memory with urgency.
    - Page faults will result in panic, except in few places like the `copy_*()` functions show above, which will result in demand-paging of those pages.

6. Do not assume page size, address lengths, etc.
    - Instead use general variable types and definitions that kernel provides.
    - E.g. Use `offset_t` for file offsets.

7. Always use generic kernel methods/utilities if they are available.

8. See [Working with Kernel development](https://docs.kernel.org/process/index.html).


## Tasks

1. ID jargon:

    | User-space | Kernel-space |
    | ---------- | ------------ |
    | Process ID (PID) | Thread Group ID (TGID) |
    | Thread ID (TID) | Process ID (PID) |

2. Kernel does not make a distinction between processes and it's threads.
    - Everything is a task with a `task_struct`.

3. Kernel can execute in 2 contexts:
    - Atomic/interrupt context: Cannot block/sleep.
    - Process context: Executing on behalf of a user-space process. Can block/sleep.

4. Execution transferred from user-space to kernel-space via system calls (implemented with synchronous IRQs i.e. exceptions) or asynchronous IRQs (HW).

4. Nested-handling of IRQs is not considered as "sleep" for a pre-empted IRQ.

5. Kernel may be configured to be preemptible or non-preemptible.
    - All kernel code must be written with this in mind.
    - All kernel code must be fully-reentrant to allow safe execution in multi-core systems.
    - Code running in user-space is always preemptible.

6. Using `PREEMPT_RT` will alter scheduling behavior.
    - E.g.  IRQ handlers may become threaded and spin-locks may become preemptible.

7. `current` points to currently running task on the CPU.
