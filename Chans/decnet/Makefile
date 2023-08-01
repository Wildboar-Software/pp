# Decnet(mail11) channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/decnet/RCS/Makefile,v 6.0 1991/12/18 20:06:35 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:06:35  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  decnet.c de_wtmail.c decnetsrvr.c
OBJS =  decnet.o de_wtmail.o decnetsrvr.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a 
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) $(INCLUDE)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS=xdecnet xdecnetsrvr

default: all

all: $(PROGS)

lint: l-decnet l-decnetsrvr

decnet: xdecnet

xdecnet: decnet.o de_wtmail.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ decnet.o de_wtmail.o \
		$(LIBPP) $(LIBRESOLV) $(LIBSYS)

l-decnet: decnet.c de_wtmail.c
	$(LINT) $(LLFLAGS) decnet.c de_wtmail.c $(LINTLIBS)

decnetsrvr: xdecnetsrvr

xdecnetsrvr: decnetsrvr.o $(LIBPP)
	$(CC) $(LDFLAGS) decnetsrvr.o $(LIBPP) $(LIBSYS) $(LIBRESOLV) -o xdecnetsrvr

l-decnetsrvr: decnetsrvr.c
	$(LINT) $(LLFLAGS) decnetsrvr.c $(LINTLIBS)


install: inst-dir inst-decnet inst-decnetsrvr

inst-dir: $(CHANDIR)

inst-decnet: $(CHANDIR)/decnet

$(CHANDIR)/decnet: xdecnet
	-$(BACKUP) $@ zxdecnet
	rm -f $@
	$(INSTALL) xdecnet $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "decnet channel installed normally"; echo ""

inst-decnetsrvr: $(CHANDIR)/decnetsrvr

$(CHANDIR)/decnetsrvr: xdecnetsrvr
	-$(BACKUP) $@ zxdecnetsrvr
	rm -f $@
	$(INSTALL) xdecnetsrvr $@
	-$(CHOWN) $(PPUSER) $@
	-$(CHMOD) 4755 $@
	-@ls -ls $@
	-@echo "you must now do whatever is necessary on your"
	-@echo "system to enable $@ to be called when an incoming"
	-@echo "mail11 connection comes in"
	-@echo "decnet server installed normally"; echo " "

test: test.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ test.o \
		$(LIBPP) $(LIBRESOLV) $(LIBSYS)


clean: tidy
	rm -f $(OBJS) $(PROGS) core a.out *old *.BAK zx*

tidy:
	rm -f $(PROGS) zx* core a.out *old *.BAK

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

define:
	$(DEFINE) Makefile



 ############################################################
 #
 # End of Building Rules
 #
 ############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
decnet.o: decnet.c
decnet.o: ../../h/head.h
decnet.o: ../../h/util.h
decnet.o: ../../h/config.h
decnet.o: ../../h/ll_log.h
decnet.o: ../../h/retcode.h
decnet.o: ../../h/chan.h
decnet.o: ../../h/table.h
decnet.o: ../../h/list_bpt.h
decnet.o: ../../h/prm.h
decnet.o: ../../h/q.h
decnet.o: ../../h/adr.h
decnet.o: ../../h/list_rchan.h
decnet.o: ../../h/chan.h
decnet.o: ../../h/auth.h
decnet.o: ../../h/list_bpt.h
decnet.o: ../../h/extension.h
decnet.o: ../../h/mta.h
decnet.o: ../../h/adr.h
decnet.o: ../../h/list_bpt.h
decnet.o: ../../h/aparse.h
decnet.o: ../../h/ap.h
decnet.o: ../../h/util.h
decnet.o: ../../h/or.h
decnet.o: ../../h/chan.h
decnet.o: ../../h/auth.h
decnet.o: ../../h/list_rchan.h
decnet.o: ../../h/mta.h
decnet.o: ../../h/dr.h
decnet.o: ../../h/extension.h
decnet.o: ../../h/mta.h
decnet.o: ../../h/qmgr.h
decnet.o: ../../h/Qmgr-types.h
decnet.o: ../../h/Qmgr-ops.h
de_wtmail.o: de_wtmail.c
de_wtmail.o: ../../h/util.h
de_wtmail.o: ../../h/config.h
de_wtmail.o: ../../h/ll_log.h
de_wtmail.o: ../../h/retcode.h
de_wtmail.o: ../../h/chan.h
de_wtmail.o: ../../h/table.h
de_wtmail.o: ../../h/list_bpt.h
de_wtmail.o: ../../h/ap.h
de_wtmail.o: ../../h/util.h
decnetsrvr.o: decnetsrvr.c
decnetsrvr.o: ../../h/head.h
decnetsrvr.o: ../../h/util.h
decnetsrvr.o: ../../h/config.h
decnetsrvr.o: ../../h/ll_log.h
decnetsrvr.o: ../../h/retcode.h
decnetsrvr.o: ../../h/chan.h
decnetsrvr.o: ../../h/table.h
decnetsrvr.o: ../../h/list_bpt.h
decnetsrvr.o: ../../h/prm.h
decnetsrvr.o: ../../h/q.h
decnetsrvr.o: ../../h/adr.h
decnetsrvr.o: ../../h/list_rchan.h
decnetsrvr.o: ../../h/chan.h
decnetsrvr.o: ../../h/auth.h
decnetsrvr.o: ../../h/list_bpt.h
decnetsrvr.o: ../../h/extension.h
decnetsrvr.o: ../../h/mta.h
decnetsrvr.o: ../../h/adr.h
decnetsrvr.o: ../../h/list_bpt.h
decnetsrvr.o: ../../h/aparse.h
decnetsrvr.o: ../../h/ap.h
decnetsrvr.o: ../../h/util.h
decnetsrvr.o: ../../h/or.h
decnetsrvr.o: ../../h/chan.h
decnetsrvr.o: ../../h/auth.h
decnetsrvr.o: ../../h/list_rchan.h
decnetsrvr.o: ../../h/mta.h
decnetsrvr.o: ../../h/ap.h
decnetsrvr.o: ../../h/retcode.h
decnetsrvr.o: ../../h/list_rchan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
