NAME
  rye.ko

SYSNOPSYS
  insmod rye.ko ptr=<pointer to a in-kernel function [OPTIONs]

DESCRIPTION

  This small module tries to catch a specific in-kernel event such as
  a phenomenon called CSS (Context Switch Storm) [1, 2, 3].

  On installation, this module creates a kernel thread, and the thread
  monitors accumulated number of context switches periodically and
  tests if cswch/s gets higher than a threshold given on the installation time.

  If the cswch/s is higher than the threshold, this module calles 
  panic(). Thus, using with crash dump utility such as kdump, you can take
  good information for your investigation.

  Here are descriptiion of insmod perameters.
    ptr           a pointer to nr_context_switch
                  You have to provide 'ptr', otherwise insmod fails.
    interval	  Interval in seconds to monitor number of context switches
    count         Monitoring count
    threshold     Threshold value to take a CSS event
                  If cswch/s is higher than the threshold, rye calls panic().
    panic         Action in case caught a CSS event
    verbose       Verbose mode

  Note that the in-kernel function 'nr_context_switches' is not publicly
  exported one to general loadable kernel modules(lkm), so you need to get
  the address of it from your running kernel via /proc/kallsyms.

INSTALLATION AND USAGE
  1) Setup your favorite crash dump (kdump etc.) as you like

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

  5) Install it (if you like)

    I don't feel any need to do this, but if you like to do so, 
    type like the following:

   # make -C PATH_TO_KERNEL_SOURCE SUBDIRS=`pwd` V=1 modules_install

   You yould see 'rye.ko' in your current directory.

  6) Get nr_context_switches address and choose parameters

     # grep nr_context_switches /proc/kallsyms
     ffffffff80280d78 T nr_context_switches

     Here, for example, let's use threshold=30000 (30000 cswch/s), 
     interval=60 (60secs, this is default) and count=60 (in total, monitors
     one hour). Then, insmod command line would be the following:


     # insmod rye.ko ptr=0xffffffff80280d78 threshold=30000 count=60 action=1

     If you omit action=1, rye doesn't call panic() and just show cswch/s value
     exeeded the threshold.

  7) Cause a CSS by some method whatever you like.

LICENSE
  This module is provided under GPL2 as you can see in rye.c.

NOTE
  Originally, this module was called 'csspanic', but this module can be
  a good start point for monitoring various in-kernel quantities such as
  CPU busy rate. So, I changed it.
  Someone might feel that there might some connections with a novel
  by J.D. Salinger[4], but ever mind that. :)

REFERENCES
  [1] http://archives.postgresql.org/pgsql-performance/2006-11/msg00037.php
  [2] http://www.cs.toronto.edu/~zhouqq/database/readings/system_r.html
  [3] http://www.dbtalk.net/comp-databases-postgresql-hackers/futex-96870.html
  [4] http://en.wikipedia.org/wiki/The_Catcher_in_the_Rye

$Id$
