/*
 * load.c is a simple CPU load generating program:
 * 
 * Copyright (C) 2007  Masanori ITOH <masanori.itoh_at_gmail.com>
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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define PROGNAME "load"
#define VERSION "1.0"

void usage(void)  
{
    printf("%s: %s\n", PROGNAME, VERSION);
    printf("    Generate specified CPU load between 1%% to 100%\n");
    printf("    Options:\n");
    printf("      -l LOAD : CPU load 1 to 100 (%) against one physical CPU\n");
    printf("      -s      : Generate CPU load calling getpid(2)\n");
    printf("      -t      : Multi-threaded model. (not yet)\n");
}
    

int main (int argc, char **argv)
{
    pid_t child;
    int i, load_percent = 25, syscall = 0;
    char c;

    while ((c = getopt(argc, argv, "l:sth")) != -1) {
	switch(c) {
	case 'l':
	    load_percent = atoi(optarg);	    
	    break;
	    
	case 's':
	    syscall = 1;
	    break;
	    
	case 't':
	    break;

	case 'h':
	    usage();
	    exit(0);
	    break;
	    
	default:
	    printf("Unkown option '%c'\n", c);
	}
	
    }
    
    if ((load_percent <= 0) || (load_percent > 100)) {
	printf("Invalid CPU load: %d %\n");
	exit(1);
    }

    printf("generating CPU load : %d %\n", load_percent);

    if ((child = fork()) > 0) {
	/*
	 * parent process
	 */
	while (1) {
	    kill(child, SIGSTOP);
	    usleep((100 - load_percent) * 1000);
	    kill(child, SIGCONT);
	    usleep(load_percent * 1000);
	}
	/* never here at this moment */
	printf("sending SIGTERM to the child\n");
	kill(child, SIGTERM);	
	
    } else if (child == 0) {
	/*
	 * child process
	 */
	while (1) {
	    if (syscall) {
		getpid();
	    }
	}
	
    } else {
	printf("fork failed\n");
	exit(0);
    }

    return 0;
}
