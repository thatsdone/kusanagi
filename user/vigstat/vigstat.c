/*
 * vigstat.c is a simple resource monitoring program for guest OSes on VMWare:
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
#define IS_VERBOSE(f) (f & FLAG_VERBOSE)

struct vig_data {
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
#if 0	
	size_t bufferSize;        /* VMGuestLib_GetResourcePoolPath */
	char pathBuffer[BUFSIZE];
#endif
};

void usage()
{
	printf("Not Yet Implemented!\n");
	return;
}

int sample(struct vig_data *pvig, unsigned int flag)
{
	int rc;
	VMGuestLibError ret;
	
	rc = gettimeofday(&pvig->ts, NULL);
	
	ret = VMGuestLib_UpdateInfo(pvig->handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_Updateinfo: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetSessionId(pvig->handle, &pvig->id);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetSessionId: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetHostProcessorSpeed(pvig->handle, &pvig->hostmhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetHostProcessorSpeed: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetCpuReservationMHz(pvig->handle, &pvig->reservemhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuReservationMHz: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetCpuLimitMHz(pvig->handle, &pvig->limitmhz);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuLimitMHz: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetCpuShares(pvig->handle, &pvig->cpushares);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuShares: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetElapsedMs(pvig->handle, &pvig->elapsedms);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetElapsedMs: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetCpuUsedMs(pvig->handle, &pvig->usedms);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetCpuUsedMs: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetMemReservationMB(pvig->handle, &pvig->reservemb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemReservationMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemLimitMB(pvig->handle, &pvig->limitmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemLimitMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemShares(pvig->handle, &pvig->memshares);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemShares: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}

	ret = VMGuestLib_GetMemMappedMB(pvig->handle, &pvig->mappedsize);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemMappedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemActiveMB(pvig->handle, &pvig->active);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemActiveMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemOverheadMB(pvig->handle, &pvig->overhead);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemOverheadMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	ret = VMGuestLib_GetMemBalloonedMB(pvig->handle, &pvig->ballooned);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemBalloonedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSwappedMB(pvig->handle, &pvig->swapped);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSwappedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSharedMB(pvig->handle, &pvig->sharedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSharedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemSharedSavedMB(pvig->handle,
					     &pvig->sharedsavedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemSharedSavedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	ret = VMGuestLib_GetMemUsedMB(pvig->handle, &pvig->usedmb);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_GetMemUsedMB: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	/*
	  VMGuestLib_GetResourcePoolPath
	*/
	return 0;
}


void output(struct vig_data *now, struct vig_data *prev, unsigned int flag)
{
	printf("%ld ", now->ts.tv_sec);
	if (flag & FLAG_RAWOUTPUT) {
		printf("%llu ", now->id);
		printf("%u ", now->hostmhz);
		printf("%u ", now->reservemhz);
		printf("%u ", now->limitmhz);
		printf("%u ", now->cpushares);
		printf("%llu ", now->elapsedms);
		printf("%llu ", now->usedms);
		printf("%u ", now->reservemb);
		printf("%u ", now->limitmb);
		printf("%u ", now->memshares);
		printf("%u ", now->mappedsize);
		printf("%u ", now->active);
		printf("%u ", now->overhead);
		printf("%u ", now->ballooned);
		printf("%u ", now->swapped);
		printf("%u ", now->sharedmb);
		printf("%u ", now->sharedsavedmb);
		printf("%u", now->usedmb);
	} else {
		if (prev == NULL) {
			printf("BUG: prev is NULL for non-raw mode!\n");
			exit(1);
		}
		printf("%llu ", now->elapsedms - prev->elapsedms);
		printf("%llu ", now->usedms - prev->usedms);
	}
	printf("\n");
	return;
}

int main (int argc, char **argv)
{
	char c;
	unsigned int flag = 0;
	int interval = 1, count = 0, max_count = 1;
	struct vig_data vig_now, vig_prev;
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
			exit(0);
			break;
		case 'r': /* raw output */
			flag |= FLAG_RAWOUTPUT;
			break;
			
		case 'v':
			flag |= FLAG_VERBOSE;
			break;
			
		default:
			printf("Unkown option '%c'\n", c);
		}
	}

	memset(&vig_now, 0x0, sizeof(struct vig_data));
	memset(&vig_prev, 0x0, sizeof(struct vig_data));
	
	ret = VMGuestLib_OpenHandle(&vig_now.handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_OpenHandle: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	
	if (sample(&vig_now, flag) != 0) {
		goto bailout;
	}
	if (flag & FLAG_RAWOUTPUT) {
		output(&vig_now, NULL, flag);
	}
	
	for (count = 0; count < max_count; count++) {
		vig_prev = vig_now;
		sleep(interval);
		if (sample(&vig_now, flag) != 0) {
			goto bailout;
		}
		output(&vig_now, &vig_prev, flag);
	}
	
bailout:	
	ret = VMGuestLib_CloseHandle(vig_now.handle);
	if (ret != VMGUESTLIB_ERROR_SUCCESS) {
		if (IS_VERBOSE(flag)) {
			printf("VMGuestLib_CloseHandle: %d (%s)\n",
			       ret, VMGuestLib_GetErrorText(ret));
		}
		return 1;
	}
	return 0;
}
    

/*
shayol5:/cygdrive/c/tmp/VIRTUALIZATION/media/vi-guest-sdk[312] grep ^VMGuestLibE
rror GuestSDK/vmGuestLib.h -A 1
VMGuestLibError VMGuestLib_OpenHandle(VMGuestLibHandle *handle);
VMGuestLibError VMGuestLib_CloseHandle(VMGuestLibHandle handle);
--
VMGuestLibError VMGuestLib_UpdateInfo(VMGuestLibHandle handle);
--
VMGuestLibError VMGuestLib_GetSessionId(VMGuestLibHandle handle,
                                        VMSessionId *id);       
--
VMGuestLibError VMGuestLib_GetCpuReservationMHz(VMGuestLibHandle handle,
                                                uint32 *cpuReservationMHz);
--
VMGuestLibError VMGuestLib_GetCpuLimitMHz(VMGuestLibHandle handle,
                                          uint32 *cpuLimitMHz);
--
VMGuestLibError VMGuestLib_GetCpuShares(VMGuestLibHandle handle,
                                        uint32 *cpuShares);     
--
VMGuestLibError VMGuestLib_GetCpuUsedMs(VMGuestLibHandle handle,
                                        uint64 *cpuUsedMs);     
--
VMGuestLibError VMGuestLib_GetHostProcessorSpeed(VMGuestLibHandle handle,
                                                 uint32 *mhz);           
--
VMGuestLibError VMGuestLib_GetMemReservationMB(VMGuestLibHandle handle,  
                                               uint32 *memReservationMB);
--
VMGuestLibError VMGuestLib_GetMemLimitMB(VMGuestLibHandle handle,
                                         uint32 *memLimitMB);    
--
VMGuestLibError VMGuestLib_GetMemShares(VMGuestLibHandle handle, 
                                        uint32 *memShares);      
--
VMGuestLibError VMGuestLib_GetMemMappedMB(VMGuestLibHandle handle, 
                                          uint32 *memMappedSizeMB); 
--
VMGuestLibError VMGuestLib_GetMemActiveMB(VMGuestLibHandle handle, 
                                          uint32 *memActiveMB);    
--
VMGuestLibError VMGuestLib_GetMemOverheadMB(VMGuestLibHandle handle, 
                                            uint32 *memOverheadMB);  
--
VMGuestLibError VMGuestLib_GetMemBalloonedMB(VMGuestLibHandle handle,
                                             uint32 *memBalloonedMB);
--
VMGuestLibError VMGuestLib_GetMemSwappedMB(VMGuestLibHandle handle, 
                                           uint32 *memSwappedMB);   
--
VMGuestLibError VMGuestLib_GetMemSharedMB(VMGuestLibHandle handle, 
                                          uint32 *memSharedMB);    
--
VMGuestLibError VMGuestLib_GetMemSharedSavedMB(VMGuestLibHandle handle, 
                                               uint32 *memSharedSavedMB); 
--
VMGuestLibError VMGuestLib_GetMemUsedMB(VMGuestLibHandle handle, // IN
                                        uint32 *memUsedMB);      // OUT
--
VMGuestLibError VMGuestLib_GetElapsedMs(VMGuestLibHandle handle, // IN
                                        uint64 *elapsedMs);      // OUT
--
VMGuestLibError VMGuestLib_GetResourcePoolPath(VMGuestLibHandle handle,
                                               size_t *bufferSize,     
shayol5:/cygdrive/c/tmp/VIRTUALIZATION/media/vi-guest-sdk[313]




shayol5:/cygdrive/c/tmp/VIRTUALIZATION/media/vi-guest-sdk[312] grep ^VMGuestLibE
rror GuestSDK/vmGuestLib.h -A 1
VMGuestLib_OpenHandle(VMGuestLibHandle *handle);
VMGuestLib_CloseHandle(VMGuestLibHandle handle);
VMGuestLib_UpdateInfo(VMGuestLibHandle handle);
VMGuestLib_GetSessionId(VMSessionId *id);

VMGuestLib_GetCpuReservationMHz(uint32 *cpuReservationMHz);
VMGuestLib_GetCpuLimitMHz(uint32 *cpuLimitMHz);
VMGuestLib_GetCpuShares(uint32 *cpuShares);     
VMGuestLib_GetCpuUsedMs(uint64 *cpuUsedMs);     
VMGuestLib_GetHostProcessorSpeed(uint32 *mhz);           
VMGuestLib_GetMemReservationMB(uint32 *memReservationMB);
VMGuestLib_GetMemLimitMB(uint32 *memLimitMB);    
VMGuestLib_GetMemShares(uint32 *memShares);      
VMGuestLib_GetMemMappedMB(uint32 *memMappedSizeMB); 
VMGuestLib_GetMemActiveMB(uint32 *memActiveMB);    
VMGuestLib_GetMemOverheadMB(uint32 *memOverheadMB);  
VMGuestLib_GetMemBalloonedMB(uint32 *memBalloonedMB);
VMGuestLib_GetMemSwappedMB(uint32 *memSwappedMB);   
VMGuestLib_GetMemSharedMB(uint32 *memSharedMB);    
VMGuestLib_GetMemSharedSavedMB(uint32 *memSharedSavedMB); 
VMGuestLib_GetMemUsedMB(uint32 *memUsedMB);     
VMGuestLib_GetElapsedMs(uint64 *elapsedMs);     
VMGuestLib_GetResourcePoolPath(size_t *bufferSize, char *pathBuffer);



	ret = VMGuestLib_UpdateInfo(handle);
	if (verbose)
		printf("VMGuestLib_Updateinfo returns %d\n", ret);
	
	ret = VMGuestLib_GetHostProcessorSpeed(handle, &hostmhz);
	if (verbose)
		printf("VMGuestLib_GetHostProcessorSpeed returns %d %u\n",
		       ret, hostmhz);

	ret = VMGuestLib_GetElapsedMs(handle, &elapsed);
	if (verbose)
		printf("VMGuestLib_GetElapsedMs returns %d %llu\n",
		       ret, elapsed);
	ret = VMGuestLib_GetCpuUsedMs(handle, &used);
	if (verbose)
		printf("VMGuestLib_GetCpuUsedMs returns %d %llu\n",
		       ret, used);
	

 */





    

