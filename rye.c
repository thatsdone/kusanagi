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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/div64.h>
#include <linux/kernel_stat.h>
#include <linux/types.h>

MODULE_AUTHOR("Masanori ITOH");
MODULE_DESCRIPTION("Catches a specified in-kernel event and causes a panic.");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");

#define RYE_CSW 0
#define RYE_CPU 1
#define RYE_CPU_USER    0x8000
#define RYE_CPU_NICE    0x4000
#define RYE_CPU_SYSTEM  0x2000
#define RYE_CPU_SOFTIRQ 0x1000
#define RYE_CPU_IRQ     0x0800
#define RYE_CPU_IDLE    0x0400
#define RYE_CPU_IOWAIT  0x0200
#define RYE_CPU_STEAL   0x0100

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

static int event = 0;
module_param(event, int, 0644);
MODULE_PARM_DESC(event, "event to monitor, 0:csw, 1:cpu");

static int cpu_mask = RYE_CPU_IDLE;
module_param(cpu_mask, int, 0644);
MODULE_PARM_DESC(cpu_mask, "user/nice/system/softirq/irq/idle/steal...");


struct rye_sample {
    union {
	struct {
	    u64 user;
	    u64 nice;
	    u64 system;
	    u64 softirq;
	    u64 irq;
	    u64 idle;
	    u64 iowait;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
	    u64 steal;
#endif
	    u64 total;
	} cpu;
	unsigned long long csw;
    } s;
    int num_cpu;
};

/*
 * In case of 'nr_context_switches()' defined in kernel/sched.c,
 * it refers per cpu structures, runqueue.
 */
unsigned long long (*csw_func)(void);

static void (*rye_sample)(struct rye_sample *) ;
static int (*rye_check)(struct rye_sample *, struct rye_sample *, long, long);

/*
 * event == 0 (RYE_CSW)
 */
static int rye_check_csw(struct rye_sample *now,
			 struct rye_sample *before,
			 long sec, long usec)
{
    unsigned long divisor;
    unsigned long long csw_sec;
    
    csw_sec = (now->s.csw - before->s.csw) * (1000 * 1000);
    divisor = (unsigned long)(1000 * 1000 * sec + usec);
    do_div(csw_sec, divisor);
    if(csw_sec >= threshold) {
	printk("rye_check_csw: "
	       "%lld (csws/sec), %llu (csws) while %ld.%06ld (sec)\n",
	       csw_sec,
	       (now->s.csw - before->s.csw),
	       sec,
	       usec);
	return 1;
    } else if (verbose) {
	printk("rye_check_csw: %llu (cswch/s)\n", csw_sec);
    }
    
    return 0;
}


static void rye_sample_csw(struct rye_sample *p) 
{
    p->s.csw = (*csw_func)();
    return;
}


/*
 * event == 1 (RYE_CPU)
 */
static int rye_check_cpu(struct rye_sample *now,
			 struct rye_sample *before,
			 long sec, long usec)
{
    u64 diff_total, work = 0;

    diff_total = now->s.cpu.total - before->s.cpu.total;

    if (cpu_mask & RYE_CPU_USER) {
	work += (now->s.cpu.user - before->s.cpu.user);
    }
    if (cpu_mask & RYE_CPU_NICE) {
	work += (now->s.cpu.nice - before->s.cpu.nice);
    }
    if (cpu_mask & RYE_CPU_SYSTEM) {
	work += (now->s.cpu.system - before->s.cpu.system);
    }
    if (cpu_mask & RYE_CPU_SOFTIRQ) {
	work += (now->s.cpu.softirq - before->s.cpu.softirq);
    }
    if (cpu_mask & RYE_CPU_IRQ) {
	work += (now->s.cpu.irq - before->s.cpu.irq);
    }
    if (cpu_mask & RYE_CPU_IDLE) {
	work += (now->s.cpu.idle - before->s.cpu.idle);
    }
    if (cpu_mask & RYE_CPU_IOWAIT) {
	work += (now->s.cpu.iowait - before->s.cpu.iowait);
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)    
    if (cpu_mask & RYE_CPU_STEAL) {
	work += (now->s.cpu.steal - before->s.cpu.steal);
    }
#endif
    work *= 100;
#ifdef DEBUG_CPU    
    printk("rye_check_cpu: DEBUG: %llu, %llu\n", diff_total, work);
#endif
    do_div(work, (uint32_t)diff_total);
    do_div(work, (uint32_t)now->num_cpu);

    if (work >= (unsigned long long)threshold) {
	printk("rye_check_cpu: %llu (%%)\n", work);	
	return 1;
	
    } else if (verbose) {
	printk("rye_check_cpu: %llu (%%)\n", work);
    }
    return 0;
}

static void rye_sample_cpu(struct rye_sample *p)
{
    int cpu;

    memset(p, 0x0, sizeof(struct rye_sample));

    for_each_cpu(cpu) {
	p->s.cpu.user += kstat_cpu(cpu).cpustat.user;
	p->s.cpu.nice += kstat_cpu(cpu).cpustat.nice;
	p->s.cpu.system += kstat_cpu(cpu).cpustat.system;
	p->s.cpu.softirq += kstat_cpu(cpu).cpustat.softirq;
	p->s.cpu.irq += kstat_cpu(cpu).cpustat.irq;
	p->s.cpu.idle += kstat_cpu(cpu).cpustat.idle;
	p->s.cpu.iowait += kstat_cpu(cpu).cpustat.iowait;
	p->num_cpu++;
    }
    p->s.cpu.total =
	p->s.cpu.user +
	p->s.cpu.nice +
	p->s.cpu.system +
	p->s.cpu.softirq +
	p->s.cpu.irq +
	p->s.cpu.idle +
	p->s.cpu.iowait
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
	+
	p->s.cpu.steal
#endif
	;
#ifdef DEBUG_CPU
    printk("rye_sample_cpu: DEBUG: %d, %llu %llu %llu %llu %llu %llu %llu "
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
	   "%llu "
#endif	   
	   "\n", 
	   p->num_cpu,
	   p->s.cpu.user,
	   p->s.cpu.nice,
	   p->s.cpu.system,
	   p->s.cpu.softirq,
	   p->s.cpu.irq,
	   p->s.cpu.idle,
	   p->s.cpu.iowait
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
	   ,
	   p->s.cpu.steal
#endif
	   );
#endif
    return;
}

static int rye_thread(void *arg)
{
    int ret, c;
    struct timeval now, before;
    long sec, usec;
    struct rye_sample rs_before, rs_now;
    
    printk("rye_thread: created\n");
    
    (*rye_sample)(&rs_before);
    do_gettimeofday(&before);
    
    for (c = 1; c <= count; c++) {
	
	set_current_state(TASK_INTERRUPTIBLE);
	ret = schedule_timeout(interval * HZ);
	if (ret != 0) {
	    printk("rye_thread: schedule_timeout returns. %d\n", ret);
	    return 1;
	}
	
	(*rye_sample)(&rs_now);
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
	ret = (*rye_check)(&rs_now, &rs_before, sec, usec);
	if(ret) {
	    if (action) {
		panic("rye_thread: Got you! :)\n");
			/* never here*/
	    } else {
		printk("rye_thread: Got you! :)\n");
	    }
	}
	rs_before.s.csw = rs_now.s.csw;
	
	before = now;
    }
    printk("rye_thread: return.\n");
    return 0;
}

static int __init rye_init(void)
{
    int ret;

    printk("rye_init: event=%d, interval=%d, count=%d\n",
	   event, interval, count);
    printk("rye_init: threshold=%ld, action=%d, verbose=%d\n",
	   threshold, action, verbose);
    printk("rye_init: ptr=0x%p, cpu_mask=0x%0x\n",
	   (void *)ptr, cpu_mask);
    
    switch (event) {
    case RYE_CSW:
	rye_sample = rye_sample_csw;
	rye_check = rye_check_csw;
	csw_func = (unsigned long long (*)(void))ptr;
	if (csw_func == 0) {
	    printk("rye_init: specify nr_context_switch address.\n");
	    return -EINVAL;
	}
	break;

    case RYE_CPU:
	if ((threshold < 0) || (threshold > 100)) {
	    printk("rye_init: invalid threshold for event=1: %ld.\n",
		   threshold);
	    return -EINVAL;
	}
	rye_sample = rye_sample_cpu;
	rye_check = rye_check_cpu;
	break;

    default:
	printk("rye_init: invalid event: %d.\n", event);
	return -EINVAL;	
    }
    
    ret = kernel_thread(rye_thread, NULL, CLONE_KERNEL);
    if (ret < 0) {
	printk("rye_init: Failed to create a kernel thread.\n");
	return -ENOMEM;
    }
    return 0;
}

static void __exit rye_exit(void)
{
    printk("rye_exit:\n");
    return;
}

module_init(rye_init);
module_exit(rye_exit);
