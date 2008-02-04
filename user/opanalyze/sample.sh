#!/bin/sh
#
# sample.sh is a shell script for collecting oprofile sample data:
# 
# Copyright (C) 2007,2008  Masanori ITOH
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#	
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# SYSNOPSYS
#   sample.sh [INTERVAL [COUNT [FILENAME]]]
#     INTERVAL  interval between opreport samples (in seconds) 
#     COUNT     sampling count
#     FILENAME  output file name
#
# DESCRIPTION
#   sample.sh invokes oprofiled and samples system wide profiling
#   information using 'opreport -la -p <dir>' periodically.
#   You can analyze the log file using opanalyze.pl accompanied by
#   this script.
#
# AUTHOR
#   Masanori ITOH  <masanori.itoh@gmail.com>
#
# $Id$
#
#set -x
TIMESTAMP=`date +"%Y%m%d-%H%M%S"`
#
# default values of arguments
INTERVAL=60
COUNT=30
FILENAME=/var/log/oprofile-${TIMESTAMP}.log
#
#
if [ "$1" != "" ]; then
    INTERVAL=$1
    if [ "$2" != "" ]; then
	COUNT=$2
	if [ "$3" != "" ]; then
	    FILENAME=$3
	fi
    fi
fi
#
# target kernel binary file
# RHEL/FedoreCore case 
# !!MODIFY HERE IF NEEDED!!
KDEBUGINFO=/usr/lib/debug/lib/modules/`uname -r`/vmlinux
#KDEBUGINFO=/usr/lib/debug/lib/modules/2.6.18-1.2798xen/vmlinux
KBINARY_PATH=/lib/modules/`uname -r`/kernel
#
# Set lower limit of tinterval to 60 seconds
# !!UNCOMMENT HERE IF NEEDED!!
#DURATION=`expr ${INTERVAL} \* ${COUNT}`
#INTERVAL=60
#COUNT=`expr ${DURATION} \/ ${INTERVAL}`
#RESIDUAL=`expr ${DURATION} \% ${INTERVAL}`
#if [ ${RESIDUAL} != 0 ]; then
#    COUNT=`expr ${COUNT} \+ 1`
#fi
echo Log file name is : ${FILENAME}
# Record actual running parameters
echo INTERVAL=${INTERVAL} COUNT=${COUNT} > ${FILENAME}
#
# install oprofile.ko
modprobe oprofile
#
# tell oprofile what to collect
if [ -e ${KDEBUGINFO} ]; then
    opcontrol --vmlinux=${KDEBUGINFO} >> ${FILENAME}
else
    opcontrol --no-vmlinux >> ${FILENAME}
fi
#
# start and reset oprofile
opcontrol --start >> ${FILENAME}
opcontrol --reset >> ${FILENAME}
#
# Record target kernel version
uname -a >> ${FILENAME}
#
# Here we go :)
c=0
while /bin/true
do
        env LANG=C date >> ${FILENAME}
	opcontrol --dump >> ${FILENAME}
# opreport 0.9.0
	opreport -la -p ${KBINARY_PATH} >> ${FILENAME}
# opreport 0.8.0
#	opreport -lc -p ${KBINARY_PATH} >> ${FILENAME}
# if you want to collect kernel information only,
# use below instead of above.
# opreport 0.9.0 or later
#	opreport -la -p ${KBINARY_PATH} ${KDEBUGINFO} >> ${FILENAME}
# opreport 0.8.0
#	opreport -lc -p ${KBINARY_PATH} ${KDEBUGINFO} >> ${FILENAME}
	echo  >> ${FILENAME}
	c=`expr ${c} \+ 1`
	if [ ${c} -ge ${COUNT} ]; then
		break
	fi
	sleep ${INTERVAL}
done
#
opcontrol --shutdown
