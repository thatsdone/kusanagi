/*
 * rye.c is a simple helper module for trouble shooting:
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
#include <linux/kernel.h>
#include <asm/div64.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>

MODULE_AUTHOR("Masanori ITOH");
MODULE_DESCRIPTION("Catches a CSS and causes a panic.");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

/*
 * Ugly workaround.
 * Just wanted a variable with the same size as a pointer.
 * At least x86 and x86_64, it works.
 */
static long ptr = 0x0;
module_param(ptr, long, 0644);
MODULE_PARM_DESC(ptr, "address of nr_context_switches");

static long threshold = 30000;
module_param(threshold, long, 0644);
MODULE_PARM_DESC(threshold, "threshold of cswch/s");

static int interval = 60;
module_param(interval, int, 0644);
MODULE_PARM_DESC(interval, "monitoring interval");

static int count = 2;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "monitoring count");

static int verbose = 0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "verbose mode");

static int action = 0;
module_param(action, int, 0644);
MODULE_PARM_DESC(action, "action, set non-zero to cause a panic");

/*
 * Some global variables
 */
static unsigned long long max_csw;
unsigned long long (*func)(void);

static int rye_thread(void *arg)
{
    int ret, c;
    struct timeval now, before;
    long sec, usec;
    unsigned long long csw_before, csw_now, csw_sec;
    unsigned long divisor;
    
    printk("rye_thread: created\n");

    csw_before = (*func)();
    do_gettimeofday(&before);
    
    for (c = 1; c <= count; c++) {
	
	set_current_state(TASK_INTERRUPTIBLE);
	ret = schedule_timeout(interval * HZ);
	if (ret != 0) {
	    printk("rye_thread: schedule_timeout returns. %d\n", ret);
	    return 1;
	}

	csw_now = (*func)();
	do_gettimeofday(&now);
	if (now.tv_usec < before.tv_usec) {
	    /*
	     * wrap around
	     */
	    sec = now.tv_sec - before.tv_sec - 1;
	    usec = 1000 * 1000 + now.tv_usec - before.tv_usec;
	} else {
	    sec = now.tv_sec - before.tv_sec;
	    usec = now.tv_usec - before.tv_usec;
	}
	
	csw_sec = (csw_now - csw_before) * (1000 * 1000);
	divisor = (unsigned long)(1000 * 1000 * sec + usec);
	do_div(csw_sec, divisor);
	if (verbose) {
	    printk("rye_thread: %llu (cswch/s)\n", csw_sec);
	}
	
	if(csw_sec >= max_csw) {
	    if (action) {
		panic("Got you! :)  "
		      "%lld (csws/sec), %llu (csws) while %ld.%06ld (sec)\n",
		      csw_sec,
		      (csw_now - csw_before),
		      sec,
		      usec);
	    } else {
		printk("Got you! :)  "
		      "%lld (csws/sec), %llu (csws) while %ld.%06ld (sec)\n",
		      csw_sec,
		      (csw_now - csw_before),
		      sec,
		      usec);
		
	    }
	    /* never here*/
	}
	
	csw_before = csw_now;
	before = now;
    }
    printk("rye_thread: return.\n");
    return 0;
}

static int __init rye_init(void)
{
    int ret;
    
    printk("rye_init: interval=%d, count=%d threshold=%ld ptr=%p\n",
	   interval, count, threshold, (void *)ptr);
    printk("rye_init: verbose=%d, action=%d\n",
	   verbose, action);
    
    max_csw = threshold;

    func = (unsigned long long (*)(void))ptr;
    if (func == 0) {
	printk("rye_init: specify nr_context_switch address.\n");
	return -EINVAL;
    }

    ret = kernel_thread(rye_thread, NULL, CLONE_KERNEL);
    
    return 0;
}

static void __exit rye_exit(void)
{
    printk("rye_exit:\n");
    return;
}

module_init(rye_init);
module_exit(rye_exit);
