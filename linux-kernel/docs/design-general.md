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


## Coding Guidelines

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
    - Everything is a task with an associated `struct task_struct`.
3. Kernel can execute in 2 contexts:
    - Atomic/interrupt context: Cannot block/sleep.
    - Process context: Executing on behalf of a user-space process. Can block/sleep.
4. Execution transferred from user-space to kernel-space via system calls (implemented with synchronous IRQs i.e. exceptions) or asynchronous IRQs (HW).
5. Nested-handling of IRQs is not considered as "sleep" for a pre-empted IRQ.
6. Kernel may be configured to be preemptible or non-preemptible.
    - All kernel code must be written with this in mind.
    - All kernel code must be fully-reentrant to allow safe execution in multi-core systems.
    - Code running in user-space is always preemptible.
7. Using `PREEMPT_RT` will alter scheduling behavior.
    - E.g.  IRQ handlers may become threaded and spin-locks may become preemptible.
8. `current` points to currently running task on the CPU.


## Synchronization Methods

There are several methods available for synchronization.
The choice depends on context (process/atomic) of use, number and nature of operations and expected resource contention.
Following table lists key mechanisms available in kernel, in approximate order of increasing overhead:

| Method | Notes |
| ---------- | ------------ |
| Atomic operations | Self-explanatory. Also includes bit operations. Never sleeps i.e. can be used in any context. |
| Spinlocks | Self-explanatory. Will block, but never sleep i.e. can be used in any context. |
| Rwlocks | Spinlocks with reader/writer lock variants. Prone to cause writer starvation if there are many readers. Same context constraints as spinlock. |
| Seqlocks | Spinlocks that are optimized to prevent writer starvation. Same context constraints as spinlock |
| Mutexes | Self-explanatory. Will sleep i.e. can be used in process context only. Owner of the mutex is the task in whos context the mutex was taken. |
| Completion functions | Ideal for high resource contention cases. Will sleeps i.e. process context only. |
| Read-Copy-Update (RCU) | Similar use cases as that of seqlocks. Can be used in any context (unless sleepable variant is used). |

Note:
1. Kernel also provides a reference counter utility (kref).
2. Spinlocks and rwlocks have variants that allow disabling IRQ handling (only on current CPU) while in the critical section.
3. Seqlocks can result in reader starvation if there are too many writers, frequent writes or writers take too much time within the lock.
    - This is because readers do not actually take a lock.
4. Mutex-use violations (use in atomic context, unlocking done by task who is not the owner, etc) can only be detected when mutex debugging is enabled in kernel.

There also exists functions to enable and disable kernel preemption: `preempt_enable()`, `preempt_disable()`
Note that `preempt_disable()` may have been called several times (count can be obtained via `preempt_count()` ).
In that case, `preempt_enable()` should also be called a corresponding number of times to re-enable preemption.
Note that `preempt_enable()` is a scheduling point i.e. it may lead to a context switch when preemption actually gets re-enabled.
There is a `preempt_enable_no_resched()` to avoid that.

Always select the simplest possible synchronization primitive.


## Virtual Memory Management

Presence of an MMU is one the minimum requirements to run Linux.

### Address Space

1. User-space tasks always execute in virtual address space.
2. The virtual address space for each task would be something like this.

    ```
    +================================+
    |                                |
    |        Kernel Addresses        | (typically linear mapping of physical RAM, kernel code + data, devices)
    |      (global, privileged)      |
    +================================+  address space boundary (PAGE_OFFSET in many architectures)
    |                                |
    |         User Addresses         |  (files, program code + data, some pages of physical RAM)
    |          (per-process)         |
    +================================+  0x00000000
    ```

    - Kernel addresses are a linear map: Virtual Address = Physical Address + Address Space Boundary Offset
        - The mapping of kernel addresses is shared/global, i.e. the same mapping applies for all tasks.
    - The address space boundary offset is very high for 64-bit systems, making the effective user address space be unlimited.

3. Kernel-space tasks (like kthread) have a similar address map model, but the user address range may be empty/unused.
4. Systems may divide kernel addressable memory into "high" and "low" (typical in 32-bit systems):
    - Low memory is permanently mapped in kernel address space.
    - High memory is not permanently mapped in kernel address space.
    - This is done since kernel only gets 1GB of address space, some of which may be used for stuff like device memory mapping.
        - Therefore remaining kernel address space may not be enough to map everthing else.
    - 64-bit systems do not contain a high memory, as the address space is large enough.
5. Linux divides memory into zones, which is used by memory management functions to decide how to allocate memory.

    | Zone | Meaning | 32-Bit Address Range | 64-Bit Address Range
    | ----------- | ----------- | ----------- | ----------- |
    | DMA | DMA for ISAs with 24-bit address limitations. | 0 - 16MB | 0 - 16MB |
    | DMA32 | DMA for ISAs with 32-bit address limitations. | 16MB - 4GB | 16MB - 4GB
    | Normal | Low memory. | 16MB - 896MB | 4GB - Remaining RAM |
    | High | High memory | 896MB - Remaining RAM | - |

Note that the address ranges differ by architecture, the numbers shown above apply to x86.

6. Linux can be considered to be always NUMA-aware.
    - Non-NUMA systems can just be considered to be a single NUMA node.
    - Kernel view of the NUMA topology is exposed here: `/sys/devices/system/cpu/cpu<n>/topology`

7. Virtual memory is divided into fixed-sized segments called pages (typical size is 4kB).
    - Access rights and other flags w.r.t memory is typically specified at page-level, not address-level.
    - `PAGE_SIZE` defines the size of a page in kernel code (should NEVER be assumed).
    - A page frame (physical page) is a fixed-size segment of physical memory.
    - A page may reside in a page frame, storage, device, etc.
    - Page tables (typically 3, 4 or 5 levels) used to map virtual to physical addresses.
    - Each task has its own page table.
    - E.g. A 5-level page table is shown below:

        ```
        Example for a 57-bit total addressable space:

        |  PGD  |  P4D  |  PUD  |  PMD  |  PTE  |  Offset  |
        |  9b   |  9b   |  9b   |  9b   |  9b   |   12b    |

        +-----------------------------+
        | Page Global Directory (PGD) |  <------------ Virtual Address
        +-----------------------------+
            |--->  +--------------------------------+
                   | Page 4th Level Directory (P4D) |
                   +--------------------------------+
                       |---> +----------------------------+
                             | Page Upper Directory (PUD) |
                             +----------------------------+
                                 |---> +-----------------------------+
                                       | Page Middle Directory (PMD) |
                                       +-----------------------------+
                                           |---> +------------------------+
                                                 | Page Table Entry (PTE) |
                                                 +------------------------+
                                                     |---> +------------+
                                                           | Page Frame |
                                                           +------------+
                                                               |---> Physical Address
        ```

8. Flags specified on page level determined aspects like access control i.e. these are not specified on individual addresses.
    - Common page flags are shown below.

        | Page Flags | Purpose |
        | ---------- | ------------ |
        | Present | Indicates if page (or page table) is in main memory. Used to control swapping. |
        | Accessed | Indicates if page is accessed by the OS. |
        | Dirty | Only applies to page table entries Indicates if the page has been updated. |
        | RW | Access right (read/write) for page (or page table). |
        | User/Superuser | Privilege level needed to access page (or page table). |
        | PCD | Controls caching behavior of pages. Linux always enables page caching |
        | PWD | Controls page update policy (write-back/write-through). Linux always sets write-back. |
        | Page Size | Only applies to page directory entries to indicate size of page containing the address. |

9. Each page frame has a `struct page` associated with it.
    - `pfn_to_page()` and `page_to_pfn()` can be used to map page frames to `struct page` and vice versa.
    - `enum pageflags` defines page flags used by Linux.
    - `virt_to_page` can be used to map a kernel virtual address to a `struct page` .
