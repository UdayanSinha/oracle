Testing the network driver:

1) The driver can be tested using ip and ping from the shell.

2) Insert the kernel module. For the steps below, it is assumed:
	- The interface is named testnet0.
	- The IP address 192.168.122.77 is available to use.

3) Bring the interface up:
	- ip link set testnet0 up

4) Assign an IP address to the interface:
	- ip addr add 192.168.122.77 dev testnet0
	- The IP address must be available (i.e. not used by anything else on the system).
	- The IP address must be on the same sub-network as the system.
	- E.g. if the system's Ethernet/WiFi device has an IP address of 192.168.122.76, then use an address like 192.168.122.77 (i.e. 192.168.122.XXX).

5) Send and receive packets:
	- ping 192.168.122.77

6) Check interface statistics:
	- ip -s link show testnet0
