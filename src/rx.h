#ifndef __RX_H__
#define __RX_H__

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <asm/atomic.h>


#define MAX_BATPKT_SIZE 1500


int iso_rx_hook_init(void);
void iso_rx_hook_exit(void);

rx_handler_result_t iso_rx_handler (struct sk_buff **pskb);

#endif /* __RX_H__ */
