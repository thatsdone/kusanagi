NAME
    shmload - a simple SysV IPC shared memory load generator

SYNOPSIS
    shmload [-s size] [-c count] [-m mode] [-k key] [-L] [-R]  [-v] [-h]

DESCRIPTION
    shmload generates memory workload using SysV IPC shared memory.
    
    Available options are below:

    -s size
       Size of the share memory segment. default is 4MB.
    -c count 
       Count to sweep whole the segment. Default is 1024.
    -m mode
       How the share memory segment will be accessed.
       Currently three modes are supported:
       - memset_once
           memory access will be done using a single memset() for the target
           share memory segment.
       - stride
           memory access will be done in page unit (4096bytes) manner.
       - simple_sweep
           memory access will be done in byte-by-byte manner.
    -k key
       Key to pass to shmget(). Default is 1234.
    -L 
       Use large page.
    -R 
       Show resource usage statistics using getrusage().
    -v 
       Verbose mode
    -h 
       Show help message

    Here are some examples to see how large page is useful under
    virtualized environment using VMware Workstation.
    

(1) 'memset_once' case (default), roughly 20% improvement.

|# ./shmload -c 10240
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 0, size= 4194304, count = 10240, large_page_size = 0
|shmload: 2008/08/02 23:02:11
|shmload: elapse: 16.914024 (seconds)
|# ./shmload -c 10240  -L
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 1, size= 4194304, count = 10240, large_page_size = 4194304
|shmload: 2008/08/02 23:02:31
|shmload: elapse: 13.679187 (seconds)

(2) 'stride' case, roughly 15% improvement.

|# ./shmload -c 10240 -m stride
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 0, size= 4194304, count = 10240, large_page_size = 0
|shmload: 2008/08/02 23:01:12
|shmload: elapse: 18.974205 (seconds)
|# ./shmload -c 10240 -m stride  -L
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 1, size= 4194304, count = 10240, large_page_size = 4194304
|shmload: 2008/08/02 23:01:44
|shmload: elapse: 16.062905 (seconds)

(3) 'simple_sweep' case, no improvement.

|# ./shmload -c 1024  -m simple_sweep
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 0, size= 4194304, count = 1024, large_page_size = 0
|shmload: 2008/08/02 23:05:23
|shmload: elapse: 70.289427 (seconds)
|# ./shmload -c 1024  -m simple_sweep  -L
|shmload v0.1 -- a simple shm load generator.
|shmload: large_page = 1, size= 4194304, count = 1024, large_page_size = 4194304
|shmload: 2008/08/02 23:06:38
|shmload: elapse: 71.312925 (seconds)


LICENSE
  This module is provided under GPL2.

  Copyright (C) 2008 Masanori ITOH <masanori.itoh@gmail.com>

HISTORY
  August 2, 2008 v0.1 Beta release

REFERENCES

$Id$
