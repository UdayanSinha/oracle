# LTTng Workflow


## Notes

- Tested on Ubuntu 24.04 LTS.
- Tested using LTTng 2.13.


## References

1. [LTTng Documentation](https://lttng.org/docs)


## Setup

1. Install necessary packages.

    ```console
    sudo apt install lttng-tools    # user-space app & library tracing, CLI
    sudo apt install lttng-modules-dkms    # only if kernel tracing is required, may need additional setup if using SecureBoot
    sudo apt install liblttng-ust-dev    # library for using LTTng in user-space apps/libs
    sudo apt install babeltrace2    # For trace output viewing
    ```

2. LTTng runs a session daemon (recommended to be started at system start), which will manage recording sessions.
    - The CLI will also communicate with the session daemon for execution of most commands.
    - There may be an independent session daemon running per user.

3. LTTng will by default create a `tracing` group.
    - E.g. for kernel tracing, either `root` session daemon must be used, or the user must be part of the `tracing` group.

4. Add the user into the `tracing` group if that is not already the case:

    ```console
    sudo usermod -a -G group-name user-name
    ```


## Kernel-Space Tracing

1. Create a recording session.

    ```console
    lttng create test-kern-session --output=/tmp/test-kern-trace
    ```

2. List available kernel tracepoints and system calls.

    ```console
    lttng list --kernel
    lttng list --kernel --syscall
    ```

3. Create event recording rules and start the recording.

    ```console
    lttng enable-event --kernel sched_process_fork,sched_process_exec
    lttng enable-event --kernel --syscall open,close
    lttng start
    ```

4. Trigger events (e.g. run `ls`, `ps`, `ping`).

5. Stop and destroy the recording session (does not delete recorded data).

    ```console
    lttng stop
    lttng destroy
    ```


## User-Space Tracing

1. Tracing in user-space applications/libraries can be done with either of the following:
    - Define custom tracepoint providers, and use those tracepoints in the application/library.
    - Use `lttng_ust_tracef()`/`lttng_ust_tracelog()` in the application/library for quick `printf()`-style tracing.

2. Refer LTTng documentation for custom tracepoint providers.
    - Note that while `lttng_ust_tracef()`/`lttng_ust_tracelog()` is better for quick prototyping, there are drawbacks to using it including potentially more performance overhead, and that one is limited in enabling specific trace events.
    - Use custom tracepoint providers if these are not acceptable, or the tracing solution needs to be long-term.

3. Example of using `lttng_ust_tracelog()` in a C program:

    ```c
    #include <stdio.h>
    #include <lttng/tracelog.h>

    int main(int argc, char *argv[])
    {
        unsigned int i = 0;
        unsigned int num = 1;
        unsigned int factorial = 1;

        printf("Enter number [1-10] to calculate factorial of: ");
        scanf("%u", &num);

        tracelog(LTTNG_UST_TRACEPOINT_LOGLEVEL_WARNING, "Number: %u", num);
        if ((num < 1) || (num > 10)) {
            printf("Number [%u] is out of bounds\n", num);
            return 0;
        }

        for (i = 1; i <= num; i++) {
            tracelog(LTTNG_UST_TRACEPOINT_LOGLEVEL_DEBUG,
                    "Factorial Intermediate[%u]: %u", i, factorial);
            factorial *= i;
        }

        printf("Factorial: %u\n", factorial);
        tracelog(LTTNG_UST_TRACEPOINT_LOGLEVEL_INFO, "Factorial Final: %u",
                factorial);

        return 0;
    }
    ```

4. Link dynamically to `liblttng-ust` when compiling the application.

5. Create a recording session.

    ```console
    lttng create test-user-session --output=/tmp/test-user-trace
    ```

6. List available user-space tracepoints.

    ```console
    lttng list --userspace
    ```

7. Create event recording rules and start the recording.

    ```console
    lttng enable-event --userspace lttng_ust_tracelog:LTTNG_UST_TRACEPOINT_LOGLEVEL_INFO    # tracelog for a specific log-level
    lttng enable-event --userspace lttng_ust_tracelog:*    # tracelog for all log levels
    lttng start
    ```

8. Trigger events by running the program.

9. Stop and destroy the recording session (does not delete recorded data).

    ```console
    lttng stop
    lttng destroy
    ```


## Viewing Recorded Data

Use `babeltrace2` to view recorded data.

```console
babeltrace2 /path/to/recording    # the path provided to --output in "lttng create"
```
