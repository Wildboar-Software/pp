#! /bin/csh -f

cd /usr/pp/pp/lib/chans
if [ -f smtpd -a -f smtpsrvr ]; then
        (./smtpd `pwd`/smtpsrvr smtp > /dev/null 2>&1 ) & echo -n ' smtp'
fi

cd /usr/pp/pp/lib/tools
su pp

cd /usr/pp/pp/lib
echo -n ' pp.startaspp : '
if ( -f pptsapd ) then
        echo -n ' pptsapd '
        pptsapd >& /dev/null &
endif
if ( -f qmgr ) then
        echo -n ' qmgr '
       qmgr >& /dev/null &
endif
