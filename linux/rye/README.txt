NAME
  rye.ko - a catcher of in-kernel events

SYNOPSIS
  insmod rye.ko [OPTIONs]

DESCRIPTION

  This small module, rye.ko, tries to catch a specific in-kernel event.
  At this moment, supported events are a phenomenon called CSS
  (Context Switch Storm) [1, 2, 3] and CPU usage ratio.
  rye.ko creates a kernel thread on insertion, and the thread monitors
  periodically a specified quantity such as context switches rate.
  If the quantity under monitoring exceeded the given threshold,
  it calls panic(). Thus, using rye.ko with your favorite crash dump
  utility such as kdump, you can take good information for your investigation.

  Here are description of insmod parameters.
    event         Event type to catch
                  0 : Monitors if number of context switches per second exceeds
                      the given threshold. (default)
                  1 : Monitors if specified categories of CPU consumption rate
                      exceed the given threshold.
    interval	  Interval in seconds to monitor the specified quantity.
                  Default is 60 seconds.
    count         Monitoring count. Default is 2.
    threshold     Threshold value to recognize event occurrences.
                  In case event=0, it means context switches per second.
                  In case event=1, it means CPU consumption rate. (0-100)
                  Default is 30000.
    action        Action in case rye.ko caught the specified event
                  0 : just shows a message via printk() (default)
                  1 : causes a panic via panic()
    ptr           A pointer to nr_context_switches().
                  When event is 0, you have to provide 'ptr', otherwise
                  insmod fails.
    cpu_mask      Bit field for CPU consumption categories to monitor.
                  0x8000 : %user
                  0x4000 : %nice
                  0x2000 : %system
                  0x1000 : %softirq
                  0x0800 : %irq
                  0x0400 : %idle (default)
                  0x0200 : %iowait
                  0x0100 : %steal (kernel version 2.6.12 or later)
    verbose       Verbose mode
                  0 : quiet output (default)
                  non zero : verbose output

  Here are several remarks.

  * The in-kernel function 'nr_context_switches()' is not a publicly
    exported one to general loadable kernel modules, so you have to get
    the address from your running kernel through /proc/kallsyms.

  * CPU consumption rates are calculated as the same way with common utilities 
    included in 'sysstat' package such as 'sar'.
    That is, increased CPU time in a particular category such as user mode
    will be divided by total CPU time between two sampling timings, not
    by the actual interval time between the two timings.

  * Currently, tested platforms are x86 and x86_64. I'm not sure other
    platforms cause I don't have other architecture boxes, but I believe
    basically it should work.

INSTALLATION AND USAGE

  1) Setup your favorite crash dump (kdump etc.) as you like. [6,7,8,9]

  2) Extract the tar ball you got.
    # Obviously, you have done this. :)

  3) Make sure you have installed appropriate kernel-devel(RHEL/FC) or
    kernel-headers(Debian) packages.

  4) Build it

     Move to the directory where you extracted the tar ball, and make.

      # cd SOMEWHERE
      # make -C PATH_TO_KERNEL_SOURCE SUBDIRS=`pwd` V=1 modules

   In case of my PentiumD 930 FedoraCore6 box, PATH_TO_KERNEL_SOURCE is:
      /usr/src/kernels/2.6.18-1.2798.fc6-xen-x86_64

   After compilation, you should see 'rye.ko' in your current directory.

  5) Install it (if you like)

    I don't feel any need to do this, but if you like to do so, 
    type like the following:

      # make -C PATH_TO_KERNEL_SOURCE SUBDIRS=`pwd` V=1 modules_install

  6) Get nr_context_switches address and choose parameters, if needed

    Below is my FC6(x86_64) box case:

      # grep nr_context_switches /proc/kallsyms
      ffffffff80280d78 T nr_context_switches


  7) Insert it into your running kernel

    Here, for example, let's use threshold=30000 (30000 cswch/s), 
    interval=60 (60 secs, this is default), count=60 (in total, monitors
    one hour) and action=1 (cause a panic and take a crash dump).
    Then, insmod command line would be like the following:

      # insmod rye.ko ptr=0xffffffff80280d78 threshold=30000 count=60 action=1

    If you want to catch an event in which system (kernel) consumes more than
    80% (normalized to 100%) of CPU power, do like the following:

      # insmod rye.ko event=1 threshold=80 count=60 action=1 cpu_mask=0x2000

    Note that the above example 'cpu_mask0x2000' does not include softirq nor
    irq. If you want to monitor CPU time in kernel mode, use 'cpu_mask=0x3800'
    instead.

    If you omit action=1 in both cases, rye.ko doesn't call panic() and
    just show the quantity exceeded the threshold.
    Also, do not forget to add a prefix '0x' to values of 'ptr' value.

  8) Cause the event by any method whatever you like


LICENSE
  This module is provided under GPL2 as you can see in rye.c.

  Copyright (C) 2007 Masanori ITOH <masanori.itoh@gmail.com>

HISTORY
  May 18, 2007 v2.1 Bug fix
  Apr 11, 2007 v2.0 CPU consumption rate monitoring
  Mar 16, 2007 v1.0 Initial revision with context switch monitoring

NOTES
  Originally, this module was called 'csspanic', but this module can be
  a good start point for monitoring various in-kernel quantities such as
  CPU busy rate. So,it's changed.

  Someone might feel that there could be some connections with a novel
  by J.D. Salinger[4] or deeper thing[5], but never mind that. :)


REFERENCES
  [1] http://archives.postgresql.org/pgsql-performance/2006-11/msg00037.php
  [2] http://www.cs.toronto.edu/~zhouqq/database/readings/system_r.html
  [3] http://www.dbtalk.net/comp-databases-postgresql-hackers/futex-96870.html
  [4] http://en.wikipedia.org/wiki/The_Catcher_in_the_Rye
  [5] http://en.wikipedia.org/wiki/Ghost_in_the_Shell

  [6] http://lse.sourceforge.net/kdump/
  [7] http://sourceforge.net/projects/lkdump/
  [8] http://www.redhat.com/support/wpapers/redhat/netdump/
  [9] http://sourceforge.net/projects/mkdump/

$Id$
