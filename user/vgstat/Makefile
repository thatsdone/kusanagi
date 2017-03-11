#
# $Id$
#
INSTALLPATH ?= /usr/bin
VMWARE_TOOLS_PATH=/usr/lib/vmware-tools
INCPATH=/usr/include/vmGuestLib
#
SRC=vgstat.c
OBJ=vgstat.o
COPT=-g -Wall -I $(INCPATH) -DRESPOOLPATH
LOPT=-lguestlib

all: vgstat

vgstat:
	gcc $(SRC) $(COPT) $(LOPT)

install: all  
	install -o root -g root -m 755 vgstat $(INSTALLPATH)/vgstat

clean:
	/bin/rm -f *~ *.o vgstat
