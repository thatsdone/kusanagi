WHAT IS SIGPROF

  'sigprof' accumulates the number of SIGNALs sent in the kernel space
  from the boot time and shows per SIGNAL statistics through /proc/sigprof.
  Note that the accumulation is done for 'sent' SIGNALs, not for 'delivered'
  ones. Thus, even if a process ignored specific SIGNALs, they will be taken
  into the statistics.

WHY SIGPROF

  On heavy loaded Java based application servers, we sometimes see that
  they exhibit very high CPU usage rate, especially %sys such as 90%
  (out of 100%).

  I saw several crash dumps of the high %sys rate situations and noticed that
  many java threads got SIGSEGVs within a very short time period
  and are forced to wait for CPU in run queues.

  sigprof is inteded for research activies on those kinds of situation.

STATUS
  Hibernated. :(
  That is because systemtap looks like a better substitute of this patch.

INSTALLATION

  1) Prepare a source tree of linux kernel that you want to use.
     I tested sigprof on 2.6.23, 2.6.22, 2.6.18 and 2.6.9-42.EL but
     presumably applicable for wide range of 2.6 kernel versions.

     Tested kernels:
       linux-2.6.23 (vanilla)
       linux-2.6.22 (vanilla)
       linux-2.6.18 (vanilla, the base of RHEL5)
       linux-2.6.9-42.EL (RHEL4 U4)

  2) Extract the sigprof source archive file. (Presumably you have
      already done.)

     # tar zxpvf sigprof-x.y.tar.gz

     A directory will be created, and it's referred  as PATH_TO_SIGPROF_SRC

  4) Apply the sigprof patch to your kernel source tree.

    # cd linux-2.6.22
    # patch -p1 < PATH_TO_SIGPROF_SRC/sigprof-x.y-2.6.z.patch

  5) Configure and enable sigprof

     Before invoking configuration, make sure EXTRAVERSION in the top
     level Makefile is what you want to use.

     You can find sigprof enabling menu at the bottom of [General setup]
     sub menu like the below:

       [*] Enable SIGNAL profiling (EXPERIMENTAL)

     Turn it on the check box.


  6) Build, install and Reboot

  7) See /proc/sigprof

EXAMPLE

  | # uname -rvm
  | 2.6.18sigprof #1 SMP Fri Oct 5 21:56:08 JST 2007 x86_64
  | # head -20 /proc/sigprof
  | SIG   COUNT
  |  [ 0]          0
  |  [ 1]          9
  |  [ 2]          4
  |  [ 3]          0
  |  [ 4]          0
  |  [ 5]          0
  |  [ 6]          0
  |  [ 7]          0
  |  [ 8]          0
  |  [ 9]          0
  |  [10]          4
  |  [11]     380802  <== SIGSEGV (Because of my SEGV storm program.)
  |  [12]         12
  |  [13]          0
  |  [14]        338  <== SIGALRM
  |  [15]          6  <== SIGTERM
  |  [16]          0
  |  [17]       1231  <== SIGCHLD
  |  [18]          8  <== SIGCONT
    :
    snip
    :
  |  [64]          0


TODO

  * Test for RT-Signal profiling.
  * Better to say 'accounting' not 'profiling'?
    * Then, calling 'sigstat' would be better.
    * Also renaming /proc/sigprof to /proc/sigstat or /proc/signals?
  * Switch from /proc to /sys?
  * Per process/thread profiling needed?
  * Add resetting interface.
  * Maybe making per-SIGNAL counters 64bit is better?
  * Better to add SIGNAL 0 profiling? Otherwise, suppress displaying SIGNAL 0.
    * At this moment, you will see SIGNAL 0, and it's always 0.

LICENSE

  This patch is provided under GPL2.
  
  Copyright (C) 2007 Masanori ITOH <masanori.itoh@gmail.com>

HISTORY
  Nov 30, 2007 v0.4 linux-2.6.9(RHEL4 kernel) patch added.
  Oct 24, 2007      linux-2.6.23 patch added.
  Oct 23, 2007      Bug fix.
  Oct  5, 2007      linux-2.6.18 patch added.
  Oct  3, 2007 v0.3 Made configurable.
  Oct  2, 2007 v0.2 Other SIGNALs supported. 
  Oct  1, 2007 v0.1 Initial version worked, only SIGSEGV captured.
  
$Id$
