#
# makefile for unix niftp document
#
#

nroff: unix-niftp-pp.txt
unix-niftp-pp.txt: unix-niftp-pp.ms
	nroff -ms $? > $@

clean:
	rm -f unix-niftp-pp.txt

install tidy depend:;
