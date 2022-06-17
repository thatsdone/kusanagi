/*
 * shmload.c : a simple SysV IPC shared memory load generator
 *
 * Copyright (C) 2008  Masanori ITOH <masanori.itoh_at_gmail.com>
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
 * Note:
 *   - This program works on only Linux.
 *   - When you want to use large page, make sure you have enough
 *     'max locked memory' size via ulimit -a.
 *     - vm.nr_hugepages
 *     - vm.hugetlb_shm_group
 * $Id$
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/resource.h>

#define PROGNAME "shmload"
#define VERSION  "0.1"
#define LPAGE_SIZE (2 * 1024 * 1024)

#define MODE_MEMSET_ONCE  1
#define MODE_SIMPLE_SWEEP 2
#define MODE_STRIDE       3

#define KB 1024
#define BUFSIZE 1024

void show_elapse(struct timeval *start, struct timeval *end, char *banner)
{
	if (end->tv_usec < start->tv_usec) {
		printf("%s%ld.%06ld (seconds)\n",
		       banner,
		       end->tv_sec - 1 - start->tv_sec,
		       end->tv_usec + 1000000 - start->tv_usec);
	} else {
		printf("%s%ld.%06ld (seconds)\n",
		       banner,
		       end->tv_sec - start->tv_sec,
		       end->tv_usec - start->tv_usec);
	}
}

void show_rusage(struct rusage *start, struct rusage *end)
{

	show_elapse(&start->ru_utime, &end->ru_utime, "user time  : ");
	show_elapse(&start->ru_stime, &end->ru_stime, "system time: ");

        printf("ru_maxrss       = %ld\n",
		end->ru_maxrss - start->ru_maxrss);
        printf("ru_ixrss        = %ld\n",
		end->ru_ixrss - start->ru_ixrss);
        printf("ru_idrss        = %ld\n",
		end->ru_idrss - start->ru_idrss);
        printf("ru_isrss        = %ld\n",
		end->ru_isrss - start->ru_isrss);
        printf("ru_minflt       = %ld\n",
		end->ru_minflt - start->ru_minflt);
        printf("ru_majflt       = %ld\n",
		end->ru_majflt - start->ru_majflt);
        printf("ru_nswap        = %ld\n",
		end->ru_nswap - start->ru_nswap);
        printf("ru_inblock      = %ld\n",
		end->ru_inblock - start->ru_inblock);
        printf("ru_oublock      = %ld\n",
		end->ru_oublock - start->ru_oublock);
        printf("ru_msgsnd       = %ld\n",
		end->ru_msgsnd - start->ru_msgsnd);
        printf("ru_msgrcv       = %ld\n",
		end->ru_msgrcv - start->ru_msgrcv);
        printf("ru_nsignals     = %ld\n",
		end->ru_nsignals - start->ru_nsignals);
        printf("ru_nvcsw        = %ld\n",
		end->ru_nvcsw - start->ru_nvcsw);
	printf("ru_nivcsw       = %ld\n",
	       end->ru_nivcsw - start->ru_nivcsw);
}


void usage(void)
{
	printf("memload OPTIONS\n");
}

int main(int argc, char **argv)
{
	int    shmid;
	int    shflag;
	char  *addr;
	char   c;
	size_t size = 4 * 1024 * 1024;
	int    large_page = 0;
	key_t  key = 1234;
	long long i, j;
	int    ret;
	char   *mode = "memset_once";
	int imode = MODE_MEMSET_ONCE;
	long long count = 1024;
	int    verbose = 0;
	int    rusage = 0;
	struct timeval start, end;
	struct rusage rstart, rend;
	FILE *proc_meminfo;
	char buf[BUFSIZE];
	char *ptr;
	long large_page_size = 0, page_size = 4096;
	struct rlimit rlimit;
	struct tm *tm;

	printf("%s v%s -- a simple shm load generator.\n", PROGNAME, VERSION);
	
	while ((c = getopt(argc, argv, "s:c:m:k:LRvh")) != -1) {
		switch (c) {
		case 's':
			if (optarg[strlen(optarg) - 1] == 'k') {
				size = atoi(optarg) * 1024;
			} else if (optarg[strlen(optarg) - 1] == 'm') {
				size = atoi(optarg) * 1024 * 1024;
			} else {
				size = atoi(optarg);
			}
			break;
		case 'c':
			count = atoll(optarg);
			break;
		case 'k':
			key = atoi(optarg);
			break;
		case 'L':
			large_page = 1;
			break;
		case 'm':
			mode = optarg;
			break;
		case 'R':
			rusage = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			usage();
			exit(0);
			break;
		default:
			;
		}
	}
	ret = getrlimit(RLIMIT_MEMLOCK, &rlimit);
	if (ret < 0) {
		printf("%s: getrlimit returned %d %d\n", PROGNAME, ret, errno);
	}
	if (verbose) {
		printf("%s: rlim_cur = %lu, size = %lu\n",
		       PROGNAME, rlimit.rlim_cur, size);
	}
	if (rlimit.rlim_cur < size) {
		printf("%s: Warning! size is larger than rlim_cur (MEMLOCK)"
		       " (%lu > %lu)\n", PROGNAME, size, rlimit.rlim_cur);
	}
	if (large_page) {
		/*
		 * Get large page (huge page) size from /proc/meminfo.
		 */
		if ((proc_meminfo = fopen("/proc/meminfo", "r")) == NULL) {
			printf("%s: failed to open /proc/meminfo", PROGNAME);
			exit(1);
		}
		while (fgets(buf, BUFSIZE, proc_meminfo) != NULL) {
			ptr = strtok(buf, " ");
			if (strcmp(ptr, "Hugepagesize:") == 0) {
				large_page_size =
					atol(strtok(ptr +
						    strlen("Hugepagesize:") + 1,
						    " ")) * KB;
				break;
			}
		}
		if (large_page_size == 0) {
			/*
			 * This means Hugepagesize: was not found, and
			 * large page is not supported.
			 */
			printf("%s: Could not get large page size"
			       " from /proc/meminfo", PROGNAME);
			exit(1);
		}
	}
	/*
	 * Create a shared memory segment.
	 */
	shflag = IPC_CREAT | SHM_R | SHM_W;
	if (large_page) {
		shflag |= SHM_HUGETLB;
	}
	printf("%s: large_page = %d, size= %lu, count = %lld,"
	       " large_page_size = %ld\n",
	       PROGNAME, large_page, size, count, large_page_size);
	shmid = shmget(key, size, shflag);
	if (verbose || shmid < 0) {
		printf("%s: shmget(key= %d, size=%lu) returns %d %d\n",
		       PROGNAME, key, size, shmid, errno);
	}
	if (shmid < 0) {
		goto bailout1;
	}
	/*
	 * Attach the segment.
	 */
	addr = shmat(shmid, 0, 0);
	if (verbose || addr == NULL) {		
		printf("%s: shmat returns %p %d\n", PROGNAME, addr, errno);
	}
	if (addr == NULL) {
		goto bailout2;
	}
	/*
	 * Check the given work load mode.
	 */
	if (strcmp(mode, "memset_once") == 0) {
		imode = MODE_MEMSET_ONCE;
	} else if (strcmp(mode, "stride") == 0) {
		imode = MODE_STRIDE;
	} else if (strcmp(mode, "simple_sweep") == 0) {
		imode = MODE_SIMPLE_SWEEP;
	} else {
		printf("%s: Invalid mode %s %d\n", PROGNAME, mode, imode);
		exit(1);
	}

	/*
	 * Try to allocate physical pages at first.
	 */
	memset(addr, 0xff, size);
	/*
	 * Here we go. :)
	 */
	if (rusage) {
		getrusage(RUSAGE_SELF, &rstart);
	}
	gettimeofday(&start, NULL);
	switch(imode) {
	case MODE_MEMSET_ONCE:
		for (i = 0; i < count; i++) {
			memset(addr, 0xff, size);
		}
		break;
	case MODE_SIMPLE_SWEEP:
		for (i = 0; i < count; i++) {
			for (j = 0; j < size; j++) {
				*(addr + j) = 0xff;
			}
		}
		break;
	case MODE_STRIDE:
		for (i = 0; i < count; i++) {
			for (j = 0; j < size / page_size; j++) {
				memset(addr + (j * page_size) % size,
				       0xff, page_size);
			}
		}
		break;
	default:
		;
	}
	gettimeofday(&end, NULL);
	if (rusage) {	
		getrusage(RUSAGE_SELF, &rend);
	}
	/*
	 * Show timestamp.
	 */
	tm = localtime((time_t *)&(start.tv_sec));
	printf("%s: %04d/%02d/%02d %02d:%02d:%02d\n",
	       PROGNAME,
	       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	       tm->tm_hour, tm->tm_min, tm->tm_sec);
	/*
	 * Show elapse time in seconds.
	 */
	show_elapse(&start, &end, PROGNAME": elapse: ");
	if (rusage) {
		show_rusage(&rstart, &rend);
	}

	/*
	 * Detach the segment.
	 */
	ret = shmdt(addr);
	if (verbose) {		
		printf("%s: shmdt returned %d %d\n", PROGNAME, ret, errno);
	}

bailout2:
	/*
	 * Remove the SHMID.
	 */
	ret = shmctl(shmid, IPC_RMID, NULL);
	if (verbose) {		
		printf("%s: shmctl returned %d %d\n", PROGNAME, ret, errno);
	}

bailout1:

	return 0;
}

