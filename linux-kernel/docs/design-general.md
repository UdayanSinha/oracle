# Kernel Development - General


## References

1. [The Linux Kernel Documentation](https://docs.kernel.org)
2. [Linux source code - Bootlin Elixir Cross Referencer](https://elixir.bootlin.com/linux/latest/source)
    - v6.18.X used as baseline.


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

    - See [indent(1) - man](https://linux.die.net/man/1/indent).
3. Use `sparse` for code static analysis (other tools may also be applicable).
    - See [sparse(1) - man](https://linux.die.net/man/1/sparse).
    - Invoke during compilation of the kernel.

        ```console
        make C=1 ...    # only specific module
        make C=2 ...    # all files
        ```

4. Use `likely()` and `unlikely()` where possible, to aid in branch prediction.
5. User-space parameters (e.g. system call arguments) should not be trusted by default.
    - User-space pointers may be invalid, or the pages may have been swapped.
    - Use `copy_to_user()` and `copy_from_user()` instead.
    - Kernel never swaps out its own pages and always allocates its memory with urgency (i.e. no demand/lazy paging).
    - Page faults will result in panic, except in few places like the `copy_*()` functions show above, which will result in demand-paging of those pages.
6. Do not assume page size, address lengths, etc.
    - Instead use general variable types and definitions that kernel provides.
    - E.g. Use `offset_t` for file offsets.
7. Always use generic kernel methods/utilities if they are available.
8. See [Working with Kernel development](https://docs.kernel.org/process/index.html).


## Boot Flow

```
+----------------------+
|   Power On / Reset   |
+----------------------+
  |---> +----------------------+
        |       Firmware       |
        |                      |    (can be several firmware steps/stages)
        | - Init hardware      |
        | - Find boot device   |
        | - Load bootloader    |
        +----------------------+
          |---> +----------------------+
                |      Bootloader      |
                |                      |    (can be several bootloaders steps/stages)
                | - Load kernel        |
                | - Load initramfs     |
                | - Pass boot args     |
                +----------------------+
                  |---> +----------------------+
                        |        Kernel        |
                        |                      |
                        | - CPU/MMU setup      |
                        | - Memory setup       |
                        | - Scheduler setup    |
                        | - Interrupt setup    |
                        | - Drivers setup      |
                        | - Virtual FS setup   |
                        +----------------------+
                          |---> +----------------------+
                                |   Mount initramfs    |
                                |      (optional)      |
                                |                      |
                                | - Prepare to mount   |
                                |   rootfs             |
                                +----------------------+
                                  |---> +----------------------+
                                        |    Mount rootfs      |
                                        +----------------------+
                                          |---> +----------------------+
                                                |      Start PID 1     |
                                                |                      |
                                                | - Setup user-space   |
                                                +----------------------+
```


## System Calls

1. Provides mechanism for user-space to invoke the kernel to get work done.
    - The work may be HW access or services provided by kernel infrastructure.
2. `syscall()` is interface for user-space to make system-calls.
    - Normally wrapped by library functions. E.g. `open()` .
    - A user-space program may not necessarily know when a system call could result from a library function call.
    - See [syscall(2) - man](https://linux.die.net/man/2/syscall).
3. System calls are typically implemented via exceptions (synchronous IRQs), but this is architecture-specific.
    - System calls available for a platform may also be architecture-specific.
4. In kernel, a new system call is typically defined via the `SYSCALL_DEFINE<n>()` macro.
    - `n` is the number of arguments to the system call.
    - There may be other variations of this macro, e.g. `COMPAT_SYSCALL_DEFINE<n>()` .


## Tasks

1. ID jargon is specified below.

    | User-space | Kernel-space |
    | ---------- | ------------ |
    | Process ID (PID, `getpid()` | Thread Group ID (TGID) |
    | Thread ID (TID), `gettid()` | Process ID (PID) |

2. Kernel does not make a distinction between processes and it's threads, or even kthreads.
    - Everything is a task with an associated `struct task_struct` with different settings.
    - The underlying system call to spawn processes and threads is the same as well (`clone()`). See [clone(2) - man](https://linux.die.net/man/2/clone).
3. Kernel can execute in 2 contexts:
    - Atomic/interrupt context: Cannot sleep.
    - Process context: Executing on behalf of a user-space process. Can sleep.
4. Execution transferred from user-space to kernel-space via system calls (exceptions) or asynchronous IRQs (HW).
5. Nested-handling of IRQs is not considered as "sleep" for a pre-empted IRQ.
6. Kernel may be configured to have its own code be preemptible or non-preemptible.
    - All kernel code must be written with this in mind.
    - All kernel code must be fully-reentrant to allow safe execution in multi-core systems.
    - Code running in user-space is always preemptible.
7. Using `PREEMPT_RT` will alter scheduling behavior.
    - E.g.  IRQ handlers may become threaded and spin-locks may become preemptible.
8. `current` points to currently running task on the CPU.
9. Maximum number of allowed tasks: `/proc/sys/kernel/threads-max`
10. Maximum allowed PID value: `/proc/sys/kernel/pid_max`
11. Common task states in `task_struct` include the following.

    | State | Purpose |
    | ---------- | ------------ |
    | `TASK_RUNNING` | Running, or waiting to be scheduled, on a CPU. |
    | `TASK_INTERRUPTIBLE` | Sleeping conditionally, i.e. can be woken up on signals (i.e. become `TASK_RUNNING`). |
    | `TASK_UNINTERRUPTIBLE` | Sleeping unconditionally, i.e. cannot be woken up on signals. |
    | `TASK_STOPPED` | Stopped due to `SIGSTOP` , `SIGTSTP` , `SIGTTIN` , `SIGTTOU` . |
    | `TASK_TRACED` | Under tracing (`ptrace()`). |
    | `TASK_DEAD` | Exited. |

12. kthreads should not be created in atomic context as the creation code can sleep.

### User-Space Process Termination Handling

1. Child processes must have their exit code read by the parent process via `wait()` .
    - Until then, the `task_struct` for the child process will not be cleaned up (zombie).
2. Parent process is notified of child process exits based on the requested signal in `clone()` (`fork()` sets `SIGCHLD`).
3. If a parent process terminates itself, the child processes will generally be reparented to the init process, unless a grandparent process has set the subreaper attribute.
    - See [prctl(2) - man](https://linux.die.net/man/2/prctl).
4. A process can exit either by calling `exit()` , return-from-main(), due to signals or an unrecoverable CPU exception.
5. For user-space threads, no signal is generated.
    - It may still be possible to monitor thread exits, e.g. `pthread_join()` .

### Symmetric Multi-Processing (SMP) & Task Isolation

1. All CPUs are considered to have equal access to memory and I/O.
2. Kernel generally tries to keep tasks on the same CPU to avoid migration overhead.
3. A task can be assigned an affinity to inform kernel **what CPUs it is allowed to be scheduled on**.
    - Affinity is typically specified as a bit mask of CPUs.
    - Can be done via `taskset` over command-line. See [taskset(1) - man](https://linux.die.net/man/1/taskset).
    - Can be done via `sched_setaffinity()` in user-space code. See [sched_setaffinity(2) - man](https://linux.die.net/man/2/sched_setaffinity).
    - CPU currently being used for execution can also be checked in code.
        - User-space code: See [sched_getcpu(3) - man](https://linux.die.net/man/3/sched_getcpu).
        - Kernel-space code: See `smp_processor_id()` .
4. Affinity can also be applied to IRQs, to direct execution of their handler on specific CPUs.
    - Can be done via `/proc/irq/<irq-num>/smp_affinity` .
    - Requires support in HW.
    - Note that a system may also be running a user-space daemon to balance IRQ handling load.
5. Kernel code (e.g drivers) can also recommend affinity behavior via `irq_set_affinity_and_hint()` and `irq_update_affinity_hint()` .
    - Not used very often, a user-space solution is preferred for this.
6. Cpuset cgroup controller can be used to inform kernel **what tasks are allowed to be scheduled on the specified CPUs**.
    - Provides exclusivity.
    - Can be used to implement *task pinning* when combined with affinity settings.
    - CPUs for task pinning = `affinity-mask & cpuset-mask`
        - Note that if no CPUs are found common, the task will be unpinned/floating.
7. Kernel also provides tickless scheduling to further isolate workloads on specific CPUs.
    - See [NO_HZ - The Linux Kernel Documentation](https://docs.kernel.org/timers/no_hz.html).
8. Following guide shows how to combine all of the above-mentioned concepts:
    - [CPU Isolation by SUSE Labs #1](https://www.suse.com/c/cpu-isolation-introduction-part-1/).
    - [CPU Isolation by SUSE Labs #2](https://www.suse.com/c/cpu-isolation-full-dynticks-part2/).
    - [CPU Isolation by SUSE Labs #3](https://www.suse.com/c/cpu-isolation-nohz_full-part-3/).
    - [CPU Isolation by SUSE Labs #4](https://www.suse.com/c/cpu-isolation-housekeeping-and-tradeoffs-part-4/).
    - [CPU Isolation by SUSE Labs #5](https://www.suse.com/c/cpu-isolation-practical-example-part-5/).
9. Kernel also provides the means to define per-CPU variables (e.g. `DEFINE_PER_CPU()`).

### Scheduling Policies For User-Space Tasks

1. Tasks are categorized into policies, which in-turn determine priority and scheduling behavior.
    - `task_struct` for that task will keep track of this.
2. Common policies are mentioned below.
    - `SCHED_OTHER` : Default policy.
        - Tasks are assigned a time-slice and the scheduler will rotate among them.
        - Dynamic priority (nice) can be used to affect time slice length. See [setpriority(2) - man](https://linux.die.net/man/2/setpriority).
    -  `SCHED_FIFO` : Soft RT policy.
        - Uses static priority which is higher than dynamic priorities of `SCHED_OTHER` .
        - A running task will continue running (as long as its runnable) if no runnable task exists with higher-priority.
        - A running task will continue running (as long as its runnable) even if there are runnable tasks with same priority.
    - `SCHED_RR` : Soft RT policy.
        - Same as `SCHED_FIFO` except that the scheduler will rotate among `SCHED_RR` tasks of same priority.
        - Will not preempt a running `SCHED_FIFO` task of same priority.
3. To configure these policies, see [sched_setscheduler(2) - man](https://linux.die.net/man/2/sched_setscheduler).

### Running User-Space Tasks From Kernel-Space

1. It is possible to run user-space processes from the kernel-space, but highly discouraged.
2. A valid use for this is `modprobe`.
3. Kernel provides `call_usermodehelper()` to do this, if needed.


## Synchronization Methods

There are several methods available for synchronization.
The choice depends on context (process/atomic) of use, number and nature of operations and expected resource contention.
Following table lists key mechanisms available in kernel, in approximate order of increasing overhead.

| Method | Notes |
| ---------- | ------------ |
| Atomic operations | Self-explanatory. Also includes bit operations. Never sleeps i.e. can be used in any context. |
| Spinlocks | Self-explanatory. Will block, but never sleep i.e. can be used in any context. |
| Rwlocks | Spinlocks with reader/writer lock variants. Prone to cause writer starvation if there are many readers. Same context constraints as spinlock. |
| Seqlocks | Spinlocks that are optimized to prevent writer starvation. Same context constraints as spinlock. |
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

Note that taking a spinlock may disable preemption.
Always select the simplest possible synchronization primitive.


## Memory Addressing

Presence of an MMU is one the minimum requirements to run Linux.

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

    | Zone | Meaning | 32-Bit Address Range | 64-Bit Address Range |
    | ----------- | ----------- | ----------- | ----------- |
    | DMA | DMA for ISAs with 24-bit address limitations. | 0 - 16MB | 0 - 16MB |
    | DMA32 | DMA for ISAs with 32-bit address limitations. | 16MB - 4GB | 16MB - 4GB |
    | Normal | Low memory. | 16MB - 896MB | 4GB - Remaining RAM |
    | High | High memory. | 896MB - Remaining RAM | - |

    - `/proc/zoneinfo` will show status of page usage per zone.
    - Note that the address ranges differ by architecture, the numbers shown above apply to x86.
6. Linux can be considered to be always NUMA-aware.
    - Non-NUMA systems can just be considered to be a single NUMA node.
    - Kernel view of the NUMA topology is exposed here: `/sys/devices/system/cpu/cpu<n>/topology`
    - See [CPU Topology - The Linux Kernel Documentation](https://docs.kernel.org/admin-guide/cputopology.html).
7. Virtual memory is divided into fixed-sized segments called pages (typical size is 4kB).
    - Flags w.r.t memory is typically specified at page-level, not address-level.
        - Flags determine aspects like dirty state (page updated but not written to storage), update policies (write-back/write-through), etc.
        - `enum pageflags` defines page flags used by Linux.
    - `PAGE_SIZE` defines the size of a page in kernel code (should NEVER be assumed).
    - A page frame (physical memory page) is a fixed-size segment of physical memory.
    - A page may reside in a page frame, storage, device, etc.
    - Page tables (typically 3, 4 or 5 levels) used to map virtual to physical addresses.
    - Each task has its own page table.
    - E.g. A 5-level page table is shown below.

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

    - 3-level page tables lack P4D and PUD.
    - 4-level page tables lack P4D.
8. Each page frame has a `struct page` associated with it.
    - `pfn_to_page()` and `page_to_pfn()` can be used to map page frames to `struct page` and vice versa.
    - `virt_to_page` can be used to map a kernel virtual address to a `struct page` .


## Memory Management

Kernel stack size is limited (order of kBs), unlike the stack size in user-space (order of MBs).
Several mechanisms exist for allocating memory dynamically, as described below.

Note that for user-space allocations, kernel may assign pages to the task address space w/o page frames being actually assigned.
Page frames will be assigned when the memory is actually attempted to be used (demand-paging).
For kernel-space allocations, page frames are always assigned when the memory was requested.

Depending on purpose of allocation and the context (atomic/process) the appropriate flags must be passed when requesting memory.
These also guide the kernel in assigning memory from an appropriate source. Common flags are shown below.

| State | Purpose |
| ---------- | ------------ |
| `GFP_KERNEL` | Most common approach. May sleep if memory is not available. |
| `GFP_ATOMIC` | Sleeping is not allowed. If memory is not available, allocation will be aborted. |
| `GFP_USER` | For allocating memory to user-space that needs to be accessible from kernel-space. May sleep if memory is not available. |
| `GFP_TRANSHUGE` | Transparent Huge Page (THP) allocations. May sleep if memory is not available. |

### Page Allocator

1. Physically contiguous memory allocated in multiple of pages.
2. Interfaces (multiple pages specified in order of 2):
    - Allocate (pointer to 1st page returned):  `__get_free_page()`, `__get_free_pages()`
    - Free: `free_page()`, `free_pages()`
3. May result in allocation being done from high memory. Use following interfaces to map them into kernel virtual address space:
    - `kmap()` (may sleep), `kmap_atomic()` (will not sleep)
    - `kunmap()` (may sleep), `kunmap_atomic()` (will not sleep)
    - The above functions are safe to use on low memory allocations too (no effect).
4. Buddy system used to prevent memory fragmentation due to frequent page allocation and freeing:
    - Fragmentation cannot be solved with MMU:
        - Frequent page table updates would be needed (overhead).
        - DMA bypasses memory paging.
    - Multiple blocks of physically contiguous pages maintained per zone.
    - Pages per-block are in increasing order of 2 (may be multiple blocks per order).
        - E.g. Blocks may have quantity of pages like so: 1, 2, 4, 8, 16, 32, ...
    - When pages are requested, kernel tries to fulfill the request (also in order of 2) from a block of that size.
    - If the block of that size is not available, it will move to the block in the next order.
        - This continues until a block is found which can satisfy the request.
        - The block may then be split into smaller blocks if needed.
    - When freeing memory, attempt is made to merge eligible (buddies) blocks into larger blocks.
    - `/proc/buddyinfo` will show number of free blocks per order.
5. User-space allocations (e.g. `malloc()`)  also go through page allocation.
    - Will result in pages being allocated to the process address space.
    - User-space libraries like libc will have their own allocator.
    - This will then break pages upto into smaller segments to allocate the requested memory sizes.
    - If more memory is needed, it will use `brk()` or `mmap()` to request from the kernel.

### Slab Allocator

1. For efficient handling of memory requests which are not a multiple of page size, or for frequent/repeated allocations.
2. Blocks of memory (caches) assigned via the page allocator.
    - Divided internally into physically contiguous memory segments (slabs).
    - Slabs may not be physically contiguous between each other.
    - Slabs are sized in number of pages.
3. Requested memory (object) assigned from one of the slabs.
4. Freed memory goes back to the slab (i.e. does not result in a page free necessarily).
5. Only used for kernel-space allocations.
6. `/proc/slabinfo` shows current slab status.
    - `vmstat -m` is also another option.
8. Additional caches can be added if needed, see `struct kmem_cache` and its interfaces.
9. `kmalloc()` and its variants use caches underneath to provide a physically contiguous memory segment (object) of requested size.

### vmalloc()

1. Provided memory may not be physically contiguous.
2. Goes via page allocator (i.e. not slab allocator) for the assigned memory.
3. Unsuitable for DMA, as page tables are not involved.
4. Useful for very large allocations (in theory, up to full size of physical memory), where memory may be access seldom.
5. `/proc/vmallocinfo` shows current `vmalloc()` allocations.
6. May sleep during allocation.

### Huge Pages

1. Larger than standard pages.
2. Used for memory-intensive applications.
    - To reduce page table size that would be needed for the memory (beneficial when there are many processes accessing that memory).
    - To reduce MMU pressure due to frequent address translation.
3. Traditional way of using huge pages requires that the pages be made available in the system first (e.g. during system initialization).
    - `echo <num-huge-pages> > /proc/sys/vm/nr_hugepages`
    - After that, there are 3 ways to use the huge pages.
        - Mount a `hugetlbfs` filesystem: `mount -t hugetlbfs none /path/to/mount/at -o size=<size-of-mount>`
            - Application can `mmap()` a file on the filesystem and use the huge pages.
            - Regular read/write access will not work.
        - Use the huge pages as System V shared memory. See [shmget(2) - man](https://linux.die.net/man/2/shmget).
            - May require changes to shared memory settings.
                - `/proc/sys/kernel/shmmax`
                - `/proc/sys/kernel/shmall`
                - `/proc/sys/kernel/shmmni`
        - Use libhugetlbfs. See [libhugetlbfs(7) - man](https://linux.die.net/man/7/libhugetlbfs).
4. Transparent Huge Pages makes it possible for applications to use huge pages on demand, w/o the administrative setup that is required otherwise.
    - Can be configured in 2 ways.
        - `CONFIG_TRANSPARENT_HUGEPAGE_ALWAYS` : Kernel will automatically assign huge pages for an application.
        - `CONFIG_TRANSPARENT_HUGEPAGE_MADVISE` : Kernel will assign huge pages if application requests it via `madvise()` .
            - See [madvise(2) - man](https://linux.die.net/man/2/madvise).
    - Note that transparent huge page support may also enable memory compaction (defragmentation support in kernel).
    - See [Transparent Huge Pages - The Linux Kernel Documentation](https://docs.kernel.org/admin-guide/mm/transhuge.html).
