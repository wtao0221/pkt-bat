#include "rx.h"

extern struct net_device *iso_netdev;
extern struct list_head ptype_all;

int iso_rx_hook_init() {
	int ret = 0;
	if (iso_netdev == NULL) {
		return 1;
	}

	rtnl_lock();
	ret = netdev_rx_handler_register(iso_netdev, iso_rx_handler, NULL);
	rtnl_unlock();

	synchronize_net();
	return ret;
}

void iso_rx_hook_exit() {
	rtnl_lock();
	netdev_rx_handler_unregister(iso_netdev);
	rtnl_unlock();
	synchronize_net();
}

rx_handler_result_t iso_rx_handler (struct sk_buff **pskb) {
	struct sk_buff *skb = *pskb;
	struct sk_buff *new_skb = NULL;

	struct ethhdr *ethh_from, *ethh_to;
	struct iphdr *iph_from, *iph_to;
	
	unsigned int proc_len = skb->len;
	unsigned char *pt_cur = NULL;
	unsigned int sum_all = 0;


	ethh_from = eth_hdr(skb);

	/* we are in the Ether layer
	 * not the IP layer 
	 * we need to cast*/

	// iph_from = (struct iphdr *)skb_network_header(skb);
	pt_cur = skb_network_header(skb);
	iph_from = (struct iphdr *)pt_cur;
	
	if (unlikely(skb->pkt_type == PACKET_LOOPBACK)) {
		// printk(KERN_INFO "this is the loopback packet\n");
		return RX_HANDLER_PASS;
	}
	
	if (unlikely(ethh_from->h_proto != __constant_htons(ETH_P_IP))) {
		return RX_HANDLER_PASS;
	}

	/* TAO: we only consider ICMP packet here */
	if (likely(iph_from->protocol != IPPROTO_ICMP)) {
		// printk(KERN_INFO "we are now not processing ICMP pkt!\n");
		return RX_HANDLER_PASS;
	}

	while (proc_len > 0) {
		
		// printk(KERN_INFO "the proce_len is %u\n", proc_len);
		iph_from = (struct iphdr *)pt_cur;
		new_skb = netdev_alloc_skb(skb->dev, MAX_BATPKT_SIZE);
		if (likely(new_skb != NULL)) {
			sum_all++;
			skb_set_queue_mapping(new_skb, 0);
			/*skb_reserve(new_skb, 2+sizeof(ethhdr)
					+iph_from->tot_len); */
			new_skb->len = ntohs(iph_from->tot_len)+sizeof(struct ethhdr);
			new_skb->protocol = __constant_htons(ETH_P_IP);
			new_skb->pkt_type = PACKET_LOOPBACK;
		
			/* fill in the Eth Header */
			skb_reset_mac_header(new_skb);
			skb_set_tail_pointer(new_skb, new_skb->len);
			ethh_to = eth_hdr(new_skb);

			memcpy(ethh_to->h_dest, ethh_from->h_dest, ETH_ALEN);
			memcpy(ethh_to->h_source, ethh_from->h_source, ETH_ALEN);
			ethh_to->h_proto = ethh_from->h_proto;
		
			/* fill in the IP  Header */
			skb_pull(new_skb, ETH_HLEN);
			// skb_set_tail_pointer(new_skb, new_skb->len);
			skb_reset_network_header(new_skb);

			iph_to = (struct iphdr *)skb_network_header(new_skb);
			/* deliver the packet to the upper layer */
			memcpy(iph_to, iph_from, ntohs(iph_from->tot_len));
			// printk(KERN_INFO "the iph_to_tot_len is %u\n", ntohs(iph_to->tot_len));
			netif_rx(new_skb);

			/* update related structures */
			pt_cur = pt_cur+ntohs(iph_from->tot_len);
			proc_len = proc_len-ntohs(iph_from->tot_len);
		}
	}
	// printk(KERN_INFO "sum_all is %u\n", sum_all);
	dev_kfree_skb(skb);
	return RX_HANDLER_CONSUMED;
}
// printk(KERN_INFO "the protocol number is %u\n", iph_to->protocol);
// printk(KERN_INFO "the skb type is %u\n", skb->pkt_type);
// printk(KERN_INFO "len: %u\tdatalen: %u\n",skb->len, skb->data_len);
// printk(KERN_INFO "%p %p %p %p\n", skb->head, skb->data, skb->tail, skb->end);
// printk(KERN_INFO "mac_header: %u,\tnetwork_header: %u\n", skb->mac_header, skb->network_header);
// printk(KERN_INFO "the ip protocol is %u\n", iph_from->protocol);
// printk(KERN_INFO "the total size is %u\n", ntohs(iph_from->tot_len));
// printk(KERN_INFO "len: %u\tdatalen: %u\n",skb->len, skb->data_len);
// printk(KERN_INFO "%p %p %p %p\n", skb->head, skb->data, skb->tail, skb->end);
// printk(KERN_INFO "The true size in skb is %u\n", skb->truesize);
// printk(KERN_INFO "IHL is %u\n", iph_from->ihl);
// printk(KERN_INFO "The tot_len in iph is %d\n", ntohs(iph_from->tot_len));
// printk(KERN_INFO "mac_header: %u,\tnetwork_header: %u\n", new_skb->mac_header, new_skb->network_header);
// printk(KERN_INFO "%u, %u\n", new_skb->len, iph_to->tot_len);
// printk(KERN_INFO "pkt_tpye: %u\n", new_skb->pkt_type);
// printk(KERN_INFO "%p %p %p %p\n", new_skb->head, new_skb->data, new_skb->tail, new_skb->end);
