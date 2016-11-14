#include <linux/version.h>
#include <linux/module.h>
#include "rx.h"

MODULE_LICENSE("GPL");

char *iso_param_dev;
MODULE_PARM_DESC(iso_param_dev, "Interface to operate batching packets.");
module_param(iso_param_dev, charp, 0);


struct net_device *iso_netdev;
static int bat_init(void);
static void bat_exit(void);
int iso_exiting;


static int bat_init() {
	int i, ret = -1;
	iso_exiting = 0;
	if (iso_param_dev == NULL) {
		/*
		  printk(KERN_INFO "batiso: need iso_param_dev, the interface to protect.\n");
		  goto err;
		*/
		iso_param_dev = "ens9\0";
	}
	/* trim */
	for (i = 0; i < 32 && iso_param_dev[i] != '\0'; i++) {
		if(iso_param_dev[i] == '\n') {
			iso_param_dev[i] = '\0';
			break;
		}
	}
	/* get the device by name*/
	rcu_read_lock();
	iso_netdev = dev_get_by_name(&init_net, iso_param_dev);
	rcu_read_unlock();

	if (iso_netdev == NULL) {
		printk(KERN_INFO "batiso: device %s not found", iso_param_dev);
		goto out;
	}
	if (iso_rx_hook_init())
		goto out_1;

	printk(KERN_INFO "batiso: operating on %s (%p)\n",
	       iso_param_dev, iso_netdev);

	ret = 0;
	goto out;

out_1:
	dev_put(iso_netdev);
out:
	return ret;
}

static void bat_exit() {
	iso_exiting = 1;
	/* memory barrir */
	mb();

	/* unregister the rx_handler*/
	iso_rx_hook_exit();
	dev_put(iso_netdev);
	printk(KERN_INFO "batiso: goodbye.\n");
}


module_init(bat_init);
module_exit(bat_exit);
