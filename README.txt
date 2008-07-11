NAME
    vgstat - a simple statistics collector for guest OSes on VMware ESX Server

SYNOPSIS
    vgstat [-i interval] [-c count] [-h] [-r] [-v] [-u]

DESCRIPTION
    vgstat collects statistical information from the VMKernel of VMware ESX
    Server by calling VMware Guest SDK functions.

    Available options are below:

    -i interval
       Specify the interval to sample in seconds. Default is 1.
    -c count 
       Specify the count to sample. Default is 1.
    -h show help message
    -r Print raw output format.
    -v Print verbose messages, especially on errors.
    -u Print timestamp using UNIX time format.

    Here is an example of raw mode output.

    | # vgstat -i 5 -c 3 -r -u
    | 1207206685 15178136555125225123 3000 2775 4294967295 2000 522709486 \
    |          99277134 0 2080768 30840 3070 123 346 0 0 13 7 3063 5002  13.75 
    | 1207206690 15178136555125225123 3000 2775 4294967295 2000 522714460 \
    |          99277844 0 2080768 30840 3070 123 346 0 0 13 7 3063 5002  14.27 
    | 1207206695 15178136555125225123 3000 2775 4294967295 2000 522719479 \
    |          99278815 0 2080768 30840 3070 123 346 0 0 13 7 3063 5002  19.35 

    From the 1st column, they mean return values of:
        gettimeofday() (timestamp)
        VMGuestLib_GetSessionId()
        VMGuestLib_GetHostProcessorSpeed()
        VMGuestLib_GetCpuReservationMHz()
        VMGuestLib_GetCpuLimitMHz()
        VMGuestLib_GetCpuShares()
        VMGuestLib_GetElapsedMs()
        VMGuestLib_GetCpuUsedMs()
        VMGuestLib_GetMemReservationMB()
        VMGuestLib_GetMemLimitMB()
        VMGuestLib_GetMemShares()
        VMGuestLib_GetMemMappedMB()
        VMGuestLib_GetMemActiveMB()
        VMGuestLib_GetMemOverheadMB()
        VMGuestLib_GetMemBalloonedMB()
        VMGuestLib_GetMemSwappedMB()
        VMGuestLib_GetMemSharedMB()
        VMGuestLib_GetMemSharedSavedMB()
        VMGuestLib_GetMemUsedMB()
    The last two columns are:
        milliseconds since the last sampling by gettimeofday()
	% Used calculated like the following.

        Here, elapse(now) and elapse(prev) are values derived
	by VMGuestLib_GetCpuUsedMs(), also used(now) and used(prev)
	are values by VMGuestLib_GetElapsedMs().
	
	% Used = 100 * (used(now) - used(prev)) / (elapse(now) - elapse(prev))

	The above '% Used' is the same as the %USED derived by esxtop, or
	'used' derived by VI SDK.
	If the VM guest has 4 VCPUs, the value can be up to 400%.

    An example of standard mode output is below.

    | # vgstat -i 5 -c 1 -u
    | 1207206685 5002 4974 710  14.27

    The second and the fifth columns are the same as raw output mode.
    In the above example, the second column, 5002 means 5002(ms)
    between successive two samples via gettimeofday().
    
    The third and fourth columns are elapse(now) - elapse(prev) and
    used(now) - used(prev).

PREREQUISITE
    1) The guest OS must run on VMware ESX Server, not VMware Workstation.
    2) You have to install and enable VMware Tools.
    3) If you want to compile vgstat on a native Linux environment,
       you have to download VMware Guest SDK from VMware web site. [1]

INSTALLATION
        
    a) In case compiling vgstat on a non-VMware Linux box:
        1) Extract vgstat archive. Presumably, you have already done.
	2) Extract the VMware Guest SDK package somewhere.
	3) Modify Makefile as needed.
           At least SDKPATH must be modified. Also, note that you need to
	   modify LIBPATH according to your guest operatins system is
	   32bit or 64bit.
        4) Make.
	5) Install.
	   # make install
	     or
	   # make install INSTALLPATH=where_you_like 

    b) In case compliling vgstat on a VMware Linux box:
        1) Extract vgstat archive. Presumably, you have already done.
	2) Just make, and that's all.
	   Make sure VMware Guest SDK header files exist.
 	   For example, /usr/lib/vmware-tools/GuestSDK/vmGuestLib.h.
        3) Make.
	4) Install.
	   # make install
	     or
	   # make install INSTALLPATH=where_you_like 

LICENSE
  This module is provided under GPL2.

  Copyright (C) 2008 Masanori ITOH <masanori.itoh@gmail.com>

HISTORY
  July  11, 2008 v0.6 Added yyyy/mm/dd hh:mm:ss format (No '-u' option).
  April 10, 2008 v0.5 Corrected a typo, and README.txt corrections.
  April  5, 2008 v0.4 Bug fixes, and README.txt added.
  April  1, 2008 v0.3 Bug fixes
  March 31, 2008 v0.2 Beta release

REFERENCES
  [1] http://www.vmware.com/support/developer/guest-sdk/

$Id$
