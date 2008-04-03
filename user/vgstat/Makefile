#
# $Id$
#
#
SDKPATH=/usr/lib/vmware-tools/GuestSDK
INCPATH=$(SDKPATH)
LIBPATH=$(SDKPATH)/lib/lib64
#
COPT=-g -Wall -I $(INCPATH) -DRESPOOLPATH
LOPT=-lvmGuestLib -L $(LIBPATH)
all:
	gcc $(COPT) $(LOPT) -o vgstat vgstat.c 

clean:
	/bin/rm -f *~ *.o vgstat
