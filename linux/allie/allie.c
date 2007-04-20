/*
 * allie.c is a simple helper module for trouble shooting:
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
#include <linux/kernel.h> /* for dump_stack(), panic() */
#include <linux/types.h>

#include <linux/kprobes.h>
#include <linux/ptrace.h>

MODULE_AUTHOR("Masanori ITOH");
MODULE_DESCRIPTION("A simple helper module for trouble shooting.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

static char *symbol = NULL;
module_param(symbol, charp, 0644);
MODULE_PARM_DESC(symbol, "symbol");

static int flags = 0;
module_param(flags, int, 0644);
MODULE_PARM_DESC(flags, "flags");

static int verbose = 0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "verbose mode");

static int action = 0;
module_param(action, int, 0644);
MODULE_PARM_DESC(action, "action, 0: nothing, 1: dump_stack(), 2: panic()");

static int count = 10;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "count ");

static void printk_str(char *header, unsigned char *ucp, int length) 
{
	int i;
	printk("%s : ", header);
	for (i = 0; i < length; i++) {
		printk("%02x", ucp[i]);
	}
	printk("\n");
}

static struct kprobe kp;
static int pre = 1;   /* must be atomic_t */
static int post = 1;  /* must be atomic_t */

int allie_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	if (pre <= count) {
		if (verbose) {
			printk("%s: %p %p\n", __FUNCTION__, p, regs);
		}
		if (action == 1) {
			dump_stack();
		} else if (action == 2) {
			panic("%s: Got you! %s",
			      __FUNCTION__, symbol);
		}
	}
	pre++;
	return 0;
}

void allie_post_handler(struct kprobe *p, struct pt_regs *regs,
		       unsigned long flag)
{
	if (post <= count) {
		if (verbose) {		
			printk("%s: %p %p %lu\n", __FUNCTION__, p, regs, flag);
		}
	}
	post++;
	return;
}

int allie_fault_handler(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	if (verbose) {		
		printk("%s: %p %p %d\n", __FUNCTION__, p, regs, trapnr);
	}
	return 0;
}

int allie_break_handler(struct kprobe *p, struct pt_regs *regs)

{
	if (verbose) {		
		printk("%s: %p %p\n", __FUNCTION__, p, regs);
	}
	return 0;
}

static int __init allie_init(void)
{
	int ret;

	printk("%s: called\n", __FUNCTION__);
	printk("%s: flags=%08x, action=%d, count=%d, verbose=%d\n",
	       __FUNCTION__, flags, action, count, verbose);
	printk("%s: symbol=%s\n",
	       __FUNCTION__, symbol);

	if (!symbol) {
		printk("%s: Specify a symbol.", __FUNCTION__);
		return -EINVAL;
	}
	
	kp.symbol_name = symbol;
	kp.pre_handler = allie_pre_handler;
	kp.post_handler = allie_post_handler;
	kp.fault_handler = allie_fault_handler;
	kp.break_handler = allie_break_handler;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		printk("%s: failed to register kprobe\n",
		       __FUNCTION__);
		return -EINVAL;
	}
	return 0;
}

static void __exit allie_exit(void)
{
	printk("%s: called\n", __FUNCTION__);
	
	unregister_kprobe(&kp);

	return;
}

module_init(allie_init);
module_exit(allie_exit);
