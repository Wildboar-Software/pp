# Makefile for timeout channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/timeout/RCS/Makefile,v 6.0 1991/12/18 20:12:51 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:51  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  timeout.c 
OBJS =  timeout.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS   = xtimeout

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# Msg timeout
#
############################################################

timeout: xtimeout

xtimeout:        $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################


install: inst-dir inst-timeout

inst-timeout: $(CHANDIR)/timeout
$(CHANDIR)/timeout: xtimeout
	-$(BACKUP) $@ zxtimeout
	rm -f $@
	$(INSTALL) xtimeout $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "timeout channel installed normally"; echo ""

inst-dir: $(CHANDIR)

clean: tidy
	rm -f $(OBJS)
tidy:
	rm -f $(PROGS) zxtimeout *.BAK core a.out *.old

lint: l-timeout

l-timeout: $(SRCS) true
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

true:;

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
timeout.o: timeout.c
timeout.o: ../../h/head.h
timeout.o: ../../h/util.h
timeout.o: ../../h/config.h
timeout.o: ../../h/ll_log.h
timeout.o: ../../h/retcode.h
timeout.o: ../../h/dbase.h
timeout.o: ../../h/config.h
timeout.o: ../../h/qmgr.h
timeout.o: ../../h/Qmgr-types.h
timeout.o: ../../h/Qmgr-ops.h
timeout.o: ../../h/q.h
timeout.o: ../../h/adr.h
timeout.o: ../../h/list_rchan.h
timeout.o: ../../h/chan.h
timeout.o: ../../h/table.h
timeout.o: ../../h/list_bpt.h
timeout.o: ../../h/auth.h
timeout.o: ../../h/list_bpt.h
timeout.o: ../../h/extension.h
timeout.o: ../../h/mta.h
timeout.o: ../../h/adr.h
timeout.o: ../../h/list_bpt.h
timeout.o: ../../h/aparse.h
timeout.o: ../../h/ap.h
timeout.o: ../../h/util.h
timeout.o: ../../h/or.h
timeout.o: ../../h/chan.h
timeout.o: ../../h/auth.h
timeout.o: ../../h/list_rchan.h
timeout.o: ../../h/mta.h
timeout.o: ../../h/dr.h
timeout.o: ../../h/extension.h
timeout.o: ../../h/mta.h
timeout.o: ../../h/prm.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
