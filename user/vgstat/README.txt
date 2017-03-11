NAME
    vgstat - a simple statistics collector for guest OSes on VMware ESXi

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

    Here is an example of raw output mode.

|$ ./vgstat -c 5 -i 5 -r
|Timestamp SessionId HostProcessorSpeed CpuReservationMHz CpuLimitMHz CpuShares ElapsedMs CpuUsedMs CpuStolenMs MemReservationMB MemLimitMB MemShares MemMappedMB MemActiveMB MemOverheadMB MemBalloonedMB MemSwappedMB MemSharedMB MemSharedSavedMB MemUsedMB
|2017-03-12T01:16:53+0900 1654195723609587025 2597 0 4294967295 1000 603558249 5009357 5044823 0 4294967295 20480 2048 204 31 0 0 0 0 2048 5001     0.56     0.12
|2017-03-12T01:16:58+0900 1654195723609587025 2597 0 4294967295 1000 603563250 5009366 5044831 0 4294967295 20480 2048 204 31 0 0 0 0 2048 5000     0.18     0.16
|2017-03-12T01:17:03+0900 1654195723609587025 2597 0 4294967295 1000 603568250 5010831 5044837 0 4294967295 20480 2048 204 31 0 0 0 0 2048 5000    29.30     0.12
|2017-03-12T01:17:08+0900 1654195723609587025 2597 0 4294967295 1000 603573251 5015222 5044843 0 4294967295 20480 2048 204 31 0 0 0 0 2048 5000    87.80     0.12
|2017-03-12T01:17:13+0900 1654195723609587025 2597 0 4294967295 1000 603578251 5015226 5044849 0 4294967295 20480 2048 204 31 0 0 0 0 2048 5000     0.08     0.12

    From the 1st column, they mean return values of:
        gettimeofday() (timestamp)
        VMGuestLib_GetSessionId()
        VMGuestLib_GetHostProcessorSpeed()
        VMGuestLib_GetCpuReservationMHz()
        VMGuestLib_GetCpuLimitMHz()
        VMGuestLib_GetCpuShares()
        VMGuestLib_GetElapsedMs()
        VMGuestLib_GetCpuUsedMs()
        VMGuestLib_GetCpuStolenMs()
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

    The last three columns are:
        milliseconds since the last sampling by gettimeofday(),
        %used and %ready calculated like the following.

        Here, elapse(now) and elapse(prev) are values derived
        by VMGuestLib_GetCpuUsedMs(), also used(now) and used(prev)
        are values by VMGuestLib_GetElapsedMs().

        %used = 100 * (used(now) - used(prev)) / (elapse(now) - elapse(prev))

        The above '%used' and '%ready' are the same as the %USED and %READY
        derived by esxtop.
        If a VM has 4 VCPUs, theose values can be up to 400%.

    Here is an example of standard output mode.

|$ ./vgstat -c 10 -i 10
|Timestamp                intvl(g) intvl(h)     used   stolen    %used   %ready
|2017-03-12T01:06:20+0900    10001    10001       30       14     0.30     0.14
|2017-03-12T01:06:30+0900    10000    10001       30       13     0.30     0.13
|2017-03-12T01:06:40+0900    10000    10001     9189       11    91.88     0.11
|2017-03-12T01:06:50+0900    10000    10000     9142       11    91.42     0.11
|2017-03-12T01:07:00+0900    10000    10001       29       13     0.29     0.13
|2017-03-12T01:07:10+0900    10000    10001     7052       11    70.51     0.11
|2017-03-12T01:07:20+0900    10000    10000    12316       11   123.16     0.11
|2017-03-12T01:07:30+0900    10000    10001     5244       12    52.43     0.12
|2017-03-12T01:07:40+0900    10000    10000       19       13     0.19     0.13
|2017-03-12T01:07:50+0900    10000    10001       19       11     0.19     0.11

    The second and the third columns are elapsed time delived in VM and
    Hypervisor respectively.
    The fourth and fifth columns areactual used time (ms) and stolen time(ms)
    between successive two samples via gettimeofday().
    The last 2 columns are also used time(%) and stolen or ready time(%).
    
    Note that in the above example, I used a 1VCPU VM, but %used goes up to
    123.16%. In my understanding, this is because VMGuestLib_GetCpuUsedMs()
    contains time spent in hypervisor side.

PREREQUISITE
    1) You have to use this tool on a ESXi VM, not a VMware Workstation.
    2) You have to install and enable open-vm-tools[2].
       Originally, this tool was written for VMware Guest SDK[1], but
       now I switched to open-vm-tools.
    3) If you want to build vgstat on a native Linux environment,
       you have to install open-vm-tools-dev package.

INSTALLATION
        
    1) Extract vgstat archive. Presumably, you have already done.
    2) Just make, and that's all.
       Make sure that you installed both open-vm-tools-dev. You should see
       /usr/include/vmGuestLib/vmGuestLib.h
    3) Make it.
    4) Install it.
       # make install
         or
       # make install INSTALLPATH=where_you_like

LICENSE
  This module is provided under GPL2.

  Copyright (C) 2008-2017, Masanori Itoh <masanori.itoh@gmail.com>

HISTORY
  March 11, 2017 v0.9 Switch from VMware Guest SDK to open-vm-tools.
  July  11, 2008 v0.6 Added yyyy/mm/dd hh:mm:ss format (No '-u' option).
  April 10, 2008 v0.5 Corrected a typo, and README.txt corrections.
  April  5, 2008 v0.4 Bug fixes, and README.txt added.
  April  1, 2008 v0.3 Bug fixes
  March 31, 2008 v0.2 Beta release

REFERENCES
  [1] http://www.vmware.com/support/developer/guest-sdk/
  [2] https://github.com/vmware/open-vm-tools

$Id$
