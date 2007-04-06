NAME
  rye.ko - a catcher of in-kernel events

SYSNOPSYS
  insmod rye.ko ptr=<pointer to a in-kernel function> [OPTIONs]

DESCRIPTION

  This small module, rye.ko, tries to catch a specific in-kernel event.

  At this moment the only supported event is a phenomenon called CSS
  (Context Switch Storm) [1, 2, 3].
  rye.ko creates a kernel thread on insertion, and the thread monitors
  accumulated number of context switches periodically and tests if context
  switches rate (per second) gets higher than a threshold given on the
  insertion time periodically.
  If the rate exceeded the given threshold, it calls panic(). Thus,
  using rye.ko with your favorite crash dump utility such as kdump,
  you can take good information for your investigation.

  Here are description of insmod perameters of rye.ko.
    ptr           A pointer to nr_context_switch
                  You have to provide 'ptr', otherwise insmod fails.
    interval	  Interval in seconds to monitor number of context switches
    count         Monitoring count
    threshold     Threshold value to take a CSS event
                  If cswch/s is higher than the threshold, rye calls panic().
    action         Action in case caught a CSS event
    verbose       Verbose mode

  Note that the in-kernel function 'nr_context_switches' is not a publicly
  exported function to general loadable kernel modules(lkm), so you have to get
  the address from your running kernel via /proc/kallsyms.

  Currently, tested platforms are x86 and x86_64. I'm not sure other
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
   You yould see 'rye.ko' in your current directory.


  5) Install it (if you like)

    I don't feel any need to do this, but if you like to do so, 
    type like the following:

      # make -C PATH_TO_KERNEL_SOURCE SUBDIRS=`pwd` V=1 modules_install


  6) Get nr_context_switches address and choose parameters

    Below is my FC6(x86_64) box case:

      # grep nr_context_switches /proc/kallsyms
      ffffffff80280d78 T nr_context_switches

    Here, for example, let's use threshold=30000 (30000 cswch/s), 
    interval=60 (60 secs, this is default), count=60 (in total, monitors
    one hour) and action=1 (take a crash dump).

  7) Insert it into your running kernel

    Then, insmod command line would be like the following:

      # insmod rye.ko ptr=0xffffffff80280d78 threshold=30000 count=60 action=1

    If you omit action=1, rye doesn't call panic() and just show cswch/s value
    exeeded the threshold. Do not forget to add a prefix '0x' to the ptr value.

  8) Cause a CSS by any method whatever you like


NOTE
  Originally, this module was called 'csspanic', but this module can be
  a good start point for monitoring various in-kernel quantities such as
  CPU busy rate. So,it's changed.

  Someone might feel that there might some connections with a novel
  by J.D. Salinger[4] or deeper thing[5], but never mind that. :)

LICENSE
  This module is provided under GPL2 as you can see in rye.c.

  Copyright (C) 2007 Masanori ITOH <masanori.itoh@gmail.com>

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
