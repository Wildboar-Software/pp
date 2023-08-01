#! /bin/sh

# SMTP running under inetd
#cd /usr/pp/pp/lib/chans
#if [ -f smtpd -a -f smtpsrvr ]; then
#	(./smtpd `pwd`/smtpsrvr smtp > /dev/null 2>&1 ) & 
#	echo -n ' smtp'
#fi

cd /usr/pp/pp/lib
su pp <<EOF
echo -n ' pp.startaspp : '
if [ -f pptsapd ]; then
	echo -n ' pptsapd '
	pptsapd > /dev/null 2>&1 
fi
if [ -f qmgr ]; then
	echo -n ' qmgr '
	qmgr > /dev/null 2>&1
fi
echo '.'
EOF
