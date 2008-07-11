#
# $Id$
#
#
INSTALLPATH ?= /usr/bin
VMWARE_TOOLS_PATH=/usr/lib/vmware-tools
INCPATH=$(VMWARE_TOOLS_PATH)/GuestSDK 
ARCH := $(shell uname -m)
ifeq "$(ARCH)" "x86_64"
	LIBPATH=$(VMWARE_TOOLS_PATH)/lib64
else
	LIBPATH=$(VMWARE_TOOLS_PATH)/lib32
endif
#
COPT=-g -Wall -I $(INCPATH) -DRESPOOLPATH
LOPT=-lvmGuestLib -L $(LIBPATH) -Wl,-rpath $(LIBPATH)
all:
	gcc $(COPT) $(LOPT) -o vgstat vgstat.c 

install: all  
	install -o root -g root -m 755 vgstat $(INSTALLPATH)/vgstat

clean:
	/bin/rm -f *~ *.o vgstat

