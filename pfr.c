/*
 * pfr.c : a simple periodical file content recorder:
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
 *   - A typical use case is sampling files under /proc or /sys periodically.
 *   - pfr tries to read content of the input file by only one read().
 *     If you see messages saying"# Warning! Need more than 2 read()s.",
 *     try to increase size of the read buffer using -b option.
 * 
 * $Id$
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE 4096
#define PROGNAME "pfr"
#define VERSION "1.0"

void usage(void) 
{
	printf("%s: %s\n", PROGNAME, VERSION);
	printf("    Record content of a file periodically with timestamps.\n");
	printf("    Options:\n");
	printf("      -f INPUT     : Input filename.\n");
	printf("      -i OUTPUT    : Output filename. (default: STDOUT)\n");
	printf("      -c COUNT     : Count to record.\n");
	printf("      -i INTERVAL  : Interval to record.\n");
	printf("      -b BUFSIZE   : Buffer size for the input file(bytes).\n");
	printf("      -d           : Debug mode.\n");
}
	
int main(int argc, char **argv)
{
	int ret, count = 0, max_count = 1, bufsize = BUFSIZE;
	int c, interval = 1, debug = 0;
	int in, out = 1; /* STDOUT */ 
	char *pbuf, buf[BUFSIZE], bufo[BUFSIZE];
	char *cr = "\n";
	char *ifile = NULL, *ofile = NULL;
	time_t now;
	struct tm *tm;

	pbuf = buf;
	while ((c = getopt(argc, argv, "f:c:i:o:b:d")) != -1) {
		switch (c) {
		case 'f':
			ifile = optarg;
			break;

		case 'c':
			max_count = atoi(optarg);
			break;

		case 'b':
			bufsize = atoi(optarg);
			pbuf = malloc(bufsize);
			if (pbuf == NULL) {
				printf("maloc failed.\n");
				exit(1);
			}
			break;

		case 'i':
			interval = atoi(optarg);
			break;

		case 'o':
			ofile = optarg;
			break;

		case 'd':
			debug = 1;
			break;

		default:
			usage();
			exit(1);
		}
	}
	if (debug) {
		printf("#ifile     = '%s'\n", ifile);
		printf("#ofile    = '%s'\n",
		       (ofile == NULL) ? "STDOUT" : ofile);
		printf("#count    = %d\n", max_count);
		printf("#interval = %d\n", interval);
		printf("#bufsize  = %d\n", bufsize);
	}
	
	if (ofile != NULL) {
		out = open(ofile, O_RDWR | O_CREAT | O_TRUNC, 00666);
		if (out < 0) {
			printf("Cannot open '%s'\n", ofile);
			exit(1);
		}
	}
    
	if (ifile == NULL) {
		printf("Specify a file to monitor.\n");
		usage();
		exit(1);
		
	} else {
		in = open(ifile, O_RDONLY);
		if (in < 0) {
			printf("failed to open '%s' (%d)\n", ifile, errno);
			exit(1);
		}
	}

	sprintf(bufo, "# %s\n", ifile);
	write(out, bufo, strlen(bufo));
    
	count = 0;
	while (1) {
		lseek(in, SEEK_SET, 0);
		memset(pbuf, 0x0, bufsize);

		now = time(NULL);
		ret = read(in, pbuf, bufsize);
		if (debug) {
			printf("#read returns = %d\n", ret);
		}
		if (ret < 0) {
			printf("read %s failed. (%d)\n", ifile, errno);
			exit(1);
		}
		if (ret == bufsize) {
		    sprintf(bufo, "# Warning! Need more than 2 read()s."
			       " Consider increasing buffer size (-b)\n");
		    write(out, bufo, strlen(bufo));
		    
		}
		tm = localtime(&now);
		sprintf(bufo, "%04d/%02d/%02d %02d:%02d:%02d\n",
			tm->tm_year + 1900, tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		write(out, bufo, strlen(bufo));
		write(out, cr, strlen(cr));
		write(out, pbuf, ret);
		if (ret == bufsize) {
			while ((ret = read(in, pbuf, bufsize)) > 0) {
				write(out, pbuf, ret);
			}
			if (ret < 0) {
				printf("read %s failed. (%d)\n", ifile, errno);
				exit(1);
			}
		}
		write(out, cr, strlen(cr));

		count++;
		if (count >= max_count) break;
		sleep(interval);
	}
	close(in);
	close(out);

	return 0;
}

