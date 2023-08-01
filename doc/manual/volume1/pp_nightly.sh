#! /bin/sh

PATH=/crg/pp/bin:/crg/pp/cmds/tools:/usr/local/bin:/usr/ucb:/bin:/usr/bin:/usr/5bin:
export PATH
#
# Shell script run nightly to collect up things etc.
#
set -x
cd /crg/pp
L=logs
T=tables
STATDIR=/crg/pp/logs/statistics

exec 1> $L/pp-nightly.log 2>&1

freespace . 2000 || { echo NO space left on device; exit 1; }

#Save the stats files
if [ -f $L/stat ]
then
	DATE=`date +%h-%d.%T`
	[ -d $L/tmp ] || mkdir $L/tmp
	mv $L/stat $L/tmp/stat.$DATE
fi

# And then once a week...
if [ `date +%w` -eq 0 ]
then
	YDATE=`date "+%h-%d-%y`
	[ -d $STATDIR ] || mkdir $STATDIR
	if pstat $L/tmp/* /dev/null | bin/stat.awk > $STATDIR/stats.$YDATE
	then
		echo 'Sucessful - removing old files'
		rm $L/tmp/*
	fi
fi

# This should be first building command, as it updates the DERFIL2 file
echo 'building the PP tables'
(cd tables; make install)

# update the niftp stuff
echo 'building the niftp database'
(cd niftp; make ) 

echo UPDATE complete
