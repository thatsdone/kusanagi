/*
 * holden.c is a simple helper module for network trouble shooting:
 * 
 * Copyright (C) 2007  Masanori ITOH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *	
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * AUTHOR:
 *   Masanori ITOH  <masanori.itoh@gmail.com>
 *
 * $Id$
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* for dump_stack() */
#include <linux/types.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/tcp.h>

MODULE_AUTHOR("Masanori ITOH");
MODULE_DESCRIPTION("A simple module for network trouble shooting.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");


#define HOLDEN_TCP_URG 0x80000000
#define HOLDEN_TCP_ACK 0x40000000
#define HOLDEN_TCP_PSH 0x20000000
#define HOLDEN_TCP_RST 0x10000000
#define HOLDEN_TCP_SYN 0x08000000
#define HOLDEN_TCP_FIN 0x04000000

static int flags = HOLDEN_TCP_SYN;
module_param(flags, int, 0644);
MODULE_PARM_DESC(flags, "TCP flags (SYN,RST, etc.)");

#define HOLDEN_IP_PRE_ROUTING  0x80000000
#define HOLDEN_IP_LOCAL_IN     0x40000000
#define HOLDEN_IP_FORWARD      0x20000000
#define HOLDEN_IP_LOCAL_OUT    0x10000000
#define HOLDEN_IP_POST_ROUTING 0x08000000

static int hooks = HOLDEN_IP_POST_ROUTING;
module_param(hooks, int, 0644);
MODULE_PARM_DESC(hooks, "hooks, netfilter hook points");


static int verbose = 0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "verbose mode");

static int action = 0;
module_param(action, int, 0644);
MODULE_PARM_DESC(action, "action, 0: dump_stack(), 1: panic()");

static void printk_str(char *header, unsigned char *ucp, int length) 
{
	int i;
	printk("%s : ", header);
	for (i = 0; i < length; i++) {
		printk("%02x", ucp[i]);
	}
	printk("\n");
}

static unsigned int holden_hook(unsigned int hooknum,
				struct sk_buff **pskb,
				const struct net_device *in,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
	if (verbose >= 2) {
		printk("%s: called %d, %p, %p, %p, %p / %d\n",
		       __FUNCTION__,
		       hooknum, pskb, in, out, okfn,
		       (int)(*pskb)->nh.iph->protocol);
	}

	if ((*pskb)->nh.iph->protocol == IPPROTO_TCP) {
		if (((flags & HOLDEN_TCP_URG) && (*pskb)->h.th->urg) ||
		    ((flags & HOLDEN_TCP_ACK) && (*pskb)->h.th->syn) ||
		    ((flags & HOLDEN_TCP_PSH) && (*pskb)->h.th->psh) ||
		    ((flags & HOLDEN_TCP_RST) && (*pskb)->h.th->rst) ||
		    ((flags & HOLDEN_TCP_SYN) && (*pskb)->h.th->syn) ||
		    ((flags & HOLDEN_TCP_FIN) && (*pskb)->h.th->fin)) {

			if (verbose >= 1) {
				printk_str("holden: TCP header", 
					   (unsigned char *)((*pskb)->h.th),
					   sizeof(struct tcphdr));
			}

			if (action == 0) {
				dump_stack();
			} else if (action == 1) {
				panic("Got you! :)\n");
			}
		}
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops holden_hook_ops = {
	.hook     = holden_hook,
	.owner    = THIS_MODULE,
	.pf       = PF_INET,
	.hooknum  = NF_IP_POST_ROUTING,
	.priority = 100,
};

static int __init holden_init(void)
{
	int ret;

	printk("%s: called\n", __FUNCTION__);

	printk("%s: flags=%08x, action=%d, verbose=%d\n",
	       __FUNCTION__, flags, action, verbose);

	ret = nf_register_hook(&holden_hook_ops);
	if (ret < 0) {
		printk("%s: failed to register NF_IP_POST_ROUTING\n",
		       __FUNCTION__);
		goto bailout_post_routing;

	}

	return 0;

  bailout_post_routing:

	return -ENOMEM;
}

static void __exit holden_exit(void)
{
	printk("%s: called\n", __FUNCTION__);
	
	nf_unregister_hook(&holden_hook_ops);

	return;
}

module_init(holden_init);
module_exit(holden_exit);
