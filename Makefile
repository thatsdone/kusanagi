#
# $Id$
#
all:
	gcc -Wall -g pfr.c -o pfr

clean:
	/bin/rm -f *~ *.o pfr
