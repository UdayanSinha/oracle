# BusyBox Setup


## Notes

- Tested on Ubuntu 24.04 LTS.
- For details, see [busybox.net](https://www.busybox.net).


## Setup

1. Download BusyBox and its dependencies.

2. Configure and compile BusyBox.

    ```console
    make menuconfig
    # BusyBox Settings --> Build Options --> Build BusyBox as a static binary --> yes
    # Setup other options if needed
    make
    ```

3. Run BusyBox. E.g. To run BusyBox shell: `/path/to/busybox sh`
