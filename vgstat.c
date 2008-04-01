/*
 * vgstat.c is a simple statistics collector for guest OSes on VMWare:
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
 * $Id$
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <vmGuestLib.h>

#define BUFSIZE 256
#define FLAG_VERBOSE   0x01
#define FLAG_RAWOUTPUT 0x02
#define FLAG_UNIXTIME  0x04

#define IS_VERBOSE(f) (f & FLAG_VERBOSE)
#define IS_RAWOUTPUT(f) (f & FLAG_RAWOUTPUT)
#define PROGNAME "vgstat"
#define VERSION "0.3"

struct vg_data {
	VMGuestLibHandle handle;
	struct timeval ts;
	VMSessionId id;           /* VMGuestLib_GetSessionId */
	uint32 hostmhz;           /* VMGuestLib_GetHostProcessorSpeed */
	uint32 reservemhz;        /* VMGuestLib_GetCpuReservationMHz */
	uint32 limitmhz;          /* VMGuestLib_GetCpuLimitMHz */
	uint32 cpushares;         /* VMGuestLib_GetCpuShares */
	uint64 elapsedms;         /* VMGuestLib_GetElapsedMs */
	uint64 usedms;            /* VMGuestLib_GetCpuUsedMs */
	uint32 reservemb;         /* VMGuestLib_GetMemReservationMB */
	uint32 limitmb;           /* VMGuestLib_GetMemLimitMB */
	uint32 memshares;         /* VMGuestLib_GetMemShares */
	uint32 mappedsize;        /* VMGuestLib_GetMemMappedMB */
	uint32 active;            /* VMGuestLib_GetMemActiveMB */
	uint32 overhead;          /* VMGuestLib_GetMemOverheadMB */
	uint32 ballooned;         /* VMGuestLib_GetMemBalloonedMB */
	uint32 swapped;           /* VMGuestLib_GetMemSwappedMB */
	uint32 sharedmb;          /* VMGuestLib_GetMemSharedMB */
	uint32 sharedsavedmb;     /* VMGuestLib_GetMemSharedSavedMB */
	uint32 usedmb;            /* VMGuestLib_GetMemUsedMB */
#ifdef RESPOOLPATH
	size_t bufsize;           /* VMGuestLib_GetResourcePoolPath */
	char   pathbuf[BUFSIZE];
#endif
};

int sample(struct vg_data *pvg, unsigned int flag)
{
	int rc;
	VMGuestLibError ret;
	
	rc = gettimeofday(&pvg->ts, NULL);
	
	ret = VMGuestLib_UpdateInfo(pvg->handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_Updateinfo: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetSessionId(pvg->handle, &pvg->id);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetSessionId: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetHostProcessorSpeed(pvg->handle, &pvg->hostmhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetHostProcessorSpeed: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetCpuReservationMHz(pvg->handle, &pvg->reservemhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuReservationMHz: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetCpuLimitMHz(pvg->handle, &pvg->limitmhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuLimitMHz: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetCpuShares(pvg->handle, &pvg->cpushares);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuShares: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetElapsedMs(pvg->handle, &pvg->elapsedms);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetElapsedMs: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetCpuUsedMs(pvg->handle, &pvg->usedms);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuUsedMs: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetMemReservationMB(pvg->handle, &pvg->reservemb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemReservationMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemLimitMB(pvg->handle, &pvg->limitmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemLimitMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemShares(pvg->handle, &pvg->memshares);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemShares: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetMemMappedMB(pvg->handle, &pvg->mappedsize);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemMappedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemActiveMB(pvg->handle, &pvg->active);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemActiveMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemOverheadMB(pvg->handle, &pvg->overhead);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemOverheadMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemBalloonedMB(pvg->handle, &pvg->ballooned);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemBalloonedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSwappedMB(pvg->handle, &pvg->swapped);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSwappedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSharedMB(pvg->handle, &pvg->sharedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSharedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSharedSavedMB(pvg->handle,
					     &pvg->sharedsavedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSharedSavedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemUsedMB(pvg->handle, &pvg->usedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemUsedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
#ifdef RESPOOLPATH
	pvg->bufsize = BUFSIZE;
	ret = VMGuestLib_GetResourcePoolPath(pvg->handle,
					     &pvg->bufsize,
					     pvg->pathbuf);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetResourcePoolPath: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
#endif
	return 0;
}


void output(struct vg_data *now, struct vg_data *prev, unsigned int flag)
{
	long long ms_now, ms_prev;

	if (now == NULL) {
		printf("output: BUG: now is NULL!n");
		exit(1);
	}
	ms_now  = (long long)now->ts.tv_sec * 1000000
		+ (long long)now->ts.tv_usec;
	ms_prev = (long long)prev->ts.tv_sec * 1000000
		+ (long long)prev->ts.tv_usec;
	
	printf("%ld ", now->ts.tv_sec);
	if (IS_RAWOUTPUT(flag)) {
		printf("%llu ", now->id);
		printf("%u ", now->hostmhz);       /* MHz */
		printf("%u ", now->reservemhz);    /* MHz */
		printf("%u ", now->limitmhz);      /* MHz */
		printf("%u ", now->cpushares);  
		printf("%llu ", now->elapsedms);   /* ms */
		printf("%llu ", now->usedms);      /* ms */
		printf("%u ", now->reservemb);     /* MB */
		printf("%u ", now->limitmb);       /* MB */
		printf("%u ", now->memshares);
		printf("%u ", now->mappedsize);    /* MB */
		printf("%u ", now->active);        /* MB */
		printf("%u ", now->overhead);      /* MB */
		printf("%u ", now->ballooned);     /* MB */
		printf("%u ", now->swapped);       /* MB */
		printf("%u ", now->sharedmb);      /* MB */
		printf("%u ", now->sharedsavedmb); /* MB */
		printf("%u ", now->usedmb);        /* MB */
		printf("%lld", (ms_now - ms_prev) / 1000); /* guest clock */
		printf("%6.2f ",
		       (double)100 * (now->usedms - prev->usedms) / 
		       (double)(now->elapsedms - prev->elapsedms));
			/* cpu usage rate */
		
	} else {
		if (prev == NULL) {
			printf("output: BUG: prev is NULL for non-raw mode!\n");
			exit(1);
		}
		printf("%llu ", now->elapsedms - prev->elapsedms);
		printf("%llu ", now->usedms - prev->usedms);
		printf("%6.2f ",
		       (double)100 * (now->usedms - prev->usedms) / 
		       (double)(now->elapsedms - prev->elapsedms));
			/* cpu usage rate */
	}
	printf("\n");
	return;
}

void usage()
{
	printf("%s: %s\n", PROGNAME, VERSION);
	printf("    Collect statistical information from VMKernel\n");
	printf("    Options:\n");
	printf("      -i INTERVAL : Interval to sample (seconds)\n");
	printf("      -c COUNT    : Count to sample\n");
	printf("      -h          : Show help message\n");
	printf("      -r          : Raw output\n");
	printf("      -v          : Verbose mode\n");
	return;
}

int main (int argc, char **argv)
{
	char c;
	unsigned int flag = 0;
	int interval = 1, count = 0, max_count = 1;
	struct vg_data vg_now, vg_prev;
	VMGuestLibError ret;

	while ((c = getopt(argc, argv, "i:c:hvr")) != -1) {
		switch(c) {
		case 'i':
			interval = atoi(optarg);
			break;
			
		case 'c':
			max_count = atoi(optarg);
			break;
			
		case 'h':
			usage();
			return 0;
			break;
			
		case 'r': /* raw output */
			flag |= FLAG_RAWOUTPUT;
			break;
			
		case 'v': /* verbose mode */
			flag |= FLAG_VERBOSE;
			break;
			
		case 'c':
			flag |= FLAG_UNIXTIME;
			break;
			
		default:
			printf("Unkown option '%c'\n", c);
		}
	}

	memset(&vg_now, 0x0, sizeof(struct vg_data));
	
	ret = VMGuestLib_OpenHandle(&vg_now.handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_OpenHandle: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	if (sample(&vg_now, flag) != 0) {
		goto bailout;
	}
#if 0
	if (IS_RAWOUTPUT(flag)) {
		output(&vg_now, NULL, flag);
	}
#endif
	for (count = 0; count < max_count; count++) {
		vg_prev = vg_now;
		sleep(interval);
		if (sample(&vg_now, flag) != 0) {
			goto bailout;
		}
		output(&vg_now, &vg_prev, flag);
	}
	
bailout:	
	ret = VMGuestLib_CloseHandle(vg_now.handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_CloseHandle: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	return 0;
}
