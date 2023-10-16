/*
 * network_drv.c: Example of a network driver
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* data types */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/netdevice.h>	/* network-device APIs and utils */
#include <linux/skbuff.h>	/* skbuff APIs and utils */
#include <linux/etherdevice.h>	/* Ethernet-packet APIs and utils */

static struct net_device *test_netdev = NULL;

int test_ndo_open(struct net_device *dev)
{
	pr_debug("Called ndo_open() for %s\n", dev->name);

	/* initialize the network HW */

	/* start TX queue */
	netif_start_queue(dev);

	return 0;
}

int test_ndo_stop(struct net_device *dev)
{
	pr_debug("Called ndo_stop() for %s\n", dev->name);

	/* de-initialize the network HW */

	/* stop TX queue */
	netif_stop_queue(dev);

	return 0;
}

/*
 * test_netif_rx() will normally be called from an interrupt handler
 * that is triggered when the network HW received a packet
 */
void test_netif_rx(struct net_device *dev, unsigned int len, unsigned char *data)
{
	struct sk_buff *skb = netdev_alloc_skb(dev, len + 2);
	int ret = 0;

	pr_debug("Received a packet for %s\n", dev->name);

	if(!skb) {
		pr_info("Unable to allocate sk_buff and receive packet for %s\n",
				dev->name);

		dev->stats.rx_dropped++;
		return;
	}

	skb_reserve(skb, 2);	/* 16-byte alignment for Ethernet */
	skb_put(skb, len);	/* make space for the data buffer */
	memcpy(skb->data, data, len);

	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	/* forward the sk_buff to upper layers of the network stack */
	ret = netif_rx(skb);
	if (ret != NET_RX_SUCCESS) {
		pr_info("Did not receive NET_RX_SUCCESS for %s, %d\n",
				dev->name, ret);

		dev->stats.rx_errors++;
		dev->stats.rx_dropped++;
		return;
	}

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += len;
	return;
}

netdev_tx_t test_ndo_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	unsigned int len = skb->len;
	unsigned char *data = skb->data;

	pr_debug("Called ndo_start_xmit() for %s\n", dev->name);

	/* update start of transmission time */
	netif_trans_update(dev);

	/*
	 * Send the data over to the network HW.
	 * This is normally some sort of HW-specific code which involves putting
	 * the data to the HW's memory space.
	 * In this case, we call test_netif_rx() since
	 * we are simply looping back.
	 */
	test_netif_rx(dev, len, data);

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += len;

	/*
	 * Free the sk_buff.
	 * Usually done in an IRQ handler which is called
	 * when HW finishes transmission.
	 */
	dev_kfree_skb(skb);
	return 0;
}

static void test_ndo_tx_timeout(struct net_device *dev, unsigned int txqueue)
{
	pr_debug("Called ndo_tx_timeout() for %s\n", dev->name);

	(void) txqueue;
	dev->stats.tx_errors++;

	/* resume TX queue */
	netif_wake_queue(dev);

	return;
}

static struct net_device_stats* test_ndo_get_stats(struct net_device *dev)
{
	pr_debug("Called ndo_get_stats() for %s\n", dev->name);
	return &dev->stats;
}

static struct net_device_ops ndo = {
	.ndo_open = test_ndo_open,
	.ndo_stop = test_ndo_stop,
	.ndo_start_xmit = test_ndo_start_xmit,
	.ndo_tx_timeout = test_ndo_tx_timeout,
	.ndo_get_stats = test_ndo_get_stats,
};

void test_netdev_setup(struct net_device *dev)
{
	ether_setup(dev);	/* initialize with standard Ethernet config. */
	dev->netdev_ops = &ndo;
	dev->watchdog_timeo = (1 * HZ);
}

static int __init network_drv_init(void)
{
	int ret = 0;
	test_netdev = alloc_netdev(0, "testnet%d",
			NET_NAME_ENUM, test_netdev_setup);

	if (!test_netdev) {
		pr_err("alloc_netdev() failed\n");
		return -ENOMEM;
	}

	ret = register_netdev(test_netdev);
	if (ret) {
		pr_err("register_netdev() failed, %d\n", ret);
		free_netdev(test_netdev);
	}

	return ret;
}

static void __exit network_drv_exit(void)
{
	unregister_netdev(test_netdev);
	free_netdev(test_netdev);
	return;
}

module_init(network_drv_init);
module_exit(network_drv_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
