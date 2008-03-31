#
# $Id$
#
INCPATH=../vi-guest-sdk/GuestSDK
LIBPATH=../vi-guest-sdk/GuestSDK/lib/lib32
COPT=-g -Wall -DRESPOOLPATH
LOPT=-lvmGuestLib
all:
	gcc $(COPT) -c -I $(INCPATH) vigstat.c

#	gcc $(COPT) -I $(INCPATH) -L$(LIBPATH) $(LOPT) -o vigstat vigstat.c 
clean:
	/bin/rm -f *~ *.o 




