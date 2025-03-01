# Network Driver Test

1. The driver can be tested using `ip` and `ping`.

2. Insert the kernel module. For the steps below, the following is assumed:
    - Interface name: `testnet0`
    - IP address `192.168.122.77` is available to use, and is available on the same sub-network as the system.
        - E.g. if the system network device has an IP address of `192.168.122.76`,
        then use an address like `192.168.122.77` (i.e. `192.168.122.XXX`).

3. Bring the interface up.

    ```console
    ip link set testnet0 up
    ```

4. Assign an IP address to the interface.

    ```console
    ip addr add 192.168.122.77 dev testnet0
    ```

5. Send and receive packets.

    ```console
    ping 192.168.122.77
    ```

6. Check interface statistics.

    ```console
    ip -s link show testnet0
    ```
