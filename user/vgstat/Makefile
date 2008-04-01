#
# $Id$
#
INCPATH=../vmguest-sdk/GuestSDK
LIBPATH=../vmguest-sdk/GuestSDK/lib/lib32
COPT=-g -Wall -DRESPOOLPATH
LOPT=-lvmGuestLib
all:
	gcc $(COPT) -c -I $(INCPATH) vgstat.c

#	gcc $(COPT) -I $(INCPATH) -L $(LIBPATH) $(LOPT) -o vgstat vgstat.c 
clean:
	/bin/rm -f *~ *.o 




