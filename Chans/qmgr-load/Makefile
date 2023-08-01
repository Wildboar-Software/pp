# makefile for qmgr loading activities
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/qmgr-load/RCS/Makefile,v 6.0 1991/12/18 20:11:26 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:11:26  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################





SRCS =  qmgr-load.c qmgr-start.c ryinitiator.c ryresponder.c
OBJS =  qmgr-load.o qmgr-start.o ryinitiator.o ryresponder.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   =       ../../Lib/libpp.a
CFLAGS  =       $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS =       $(LDOPTIONS) $(LLF)

LLFLAGS  =      $(LINTFLAGS) -u  -I$(HEADERS)
LINTLIBS =      ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

all: xqmgr-load

############################################################
#
# Q start up
#
############################################################

QSOBJS = qmgr-start.o ryinitiator.o
QSSRCS = qmgr-start.c ryinitiator.c

qmgr-start:     xqstart

xqstart:        $(QSOBJS)
		$(CC) $(LDFLAGS) -o $@ $(QSOBJS) $(LIBPP) $(LIBSYS)

qmgr-start.o:   ryinitiator.h

ryinitiator.o:  ryinitiator.h

############################################################
#
# Qmgr loading channel
#
############################################################

QLOBJS = qmgr-load.o ryresponder.o
QLSRCS = qmgr-load.c ryresponder.c

qmgr-load:      xqmgr-load

xqmgr-load:     $(QLOBJS) $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ $(QLOBJS) $(LIBPP) \
				$(LIBSYS)

saber_src: $(QLSRCS)
	#load -C $(CFLAGS) $(QLSRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(QLOBJS)
	#load -C $(QLOBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################

install:  inst-dir inst-qmgr-load

inst-qmgr-load: $(CHANDIR)/qmgr-load
$(CHANDIR)/qmgr-load: xqmgr-load
	-$(BACKUP) $@ zxqmgr-load
	rm -f $@
	$(INSTALL) xqmgr-load $(CHANDIR)/qmgr-load
	-$(CHMOD) $(PGMPROT) $(CHANDIR)/qmgr-load
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $(CHANDIR)/qmgr-load
	-@echo "qmgr-load channel installed normally"; echo ""

inst-dir: $(CHANDIR)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core a.out *.BAK *.old xqstart xqmgr-load zxqmgr-load

lint: l-load

l-load: $(SRCS) true
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
qmgr-load.o: qmgr-load.c
qmgr-load.o: ../../h/util.h
qmgr-load.o: ../../h/config.h
qmgr-load.o: ../../h/ll_log.h
qmgr-load.o: ../../h/retcode.h
qmgr-load.o: ../../h/Qmgr-ops.h
qmgr-load.o: ../../h/Qmgr-types.h
qmgr-load.o: ryresponder.h
qmgr-load.o: ../../h/q.h
qmgr-load.o: ../../h/adr.h
qmgr-load.o: ../../h/list_rchan.h
qmgr-load.o: ../../h/chan.h
qmgr-load.o: ../../h/table.h
qmgr-load.o: ../../h/list_bpt.h
qmgr-load.o: ../../h/auth.h
qmgr-load.o: ../../h/list_bpt.h
qmgr-load.o: ../../h/extension.h
qmgr-load.o: ../../h/mta.h
qmgr-load.o: ../../h/adr.h
qmgr-load.o: ../../h/list_bpt.h
qmgr-load.o: ../../h/aparse.h
qmgr-load.o: ../../h/ap.h
qmgr-load.o: ../../h/util.h
qmgr-load.o: ../../h/or.h
qmgr-load.o: ../../h/chan.h
qmgr-load.o: ../../h/auth.h
qmgr-load.o: ../../h/list_rchan.h
qmgr-load.o: ../../h/mta.h
qmgr-load.o: ../../h/prm.h
qmgr-start.o: qmgr-start.c
qmgr-start.o: ../../h/util.h
qmgr-start.o: ../../h/config.h
qmgr-start.o: ../../h/ll_log.h
qmgr-start.o: ../../h/retcode.h
qmgr-start.o: ../../h/Qmgr-ops.h
qmgr-start.o: ../../h/Qmgr-types.h
qmgr-start.o: ryinitiator.h
qmgr-start.o: ../../h/q.h
qmgr-start.o: ../../h/adr.h
qmgr-start.o: ../../h/list_rchan.h
qmgr-start.o: ../../h/chan.h
qmgr-start.o: ../../h/table.h
qmgr-start.o: ../../h/list_bpt.h
qmgr-start.o: ../../h/auth.h
qmgr-start.o: ../../h/list_bpt.h
qmgr-start.o: ../../h/extension.h
qmgr-start.o: ../../h/mta.h
qmgr-start.o: ../../h/adr.h
qmgr-start.o: ../../h/list_bpt.h
qmgr-start.o: ../../h/aparse.h
qmgr-start.o: ../../h/ap.h
qmgr-start.o: ../../h/util.h
qmgr-start.o: ../../h/or.h
qmgr-start.o: ../../h/chan.h
qmgr-start.o: ../../h/auth.h
qmgr-start.o: ../../h/list_rchan.h
qmgr-start.o: ../../h/mta.h
qmgr-start.o: ../../h/prm.h
ryinitiator.o: ryinitiator.c
ryinitiator.o: ../../h/Qmgr-types.h
ryinitiator.o: ryinitiator.h
ryinitiator.o: ../../h/qmgr.h
ryinitiator.o: ../../h/Qmgr-types.h
ryinitiator.o: ../../h/Qmgr-ops.h
ryresponder.o: ryresponder.c
ryresponder.o: ../../h/Qmgr-ops.h
ryresponder.o: ../../h/util.h
ryresponder.o: ../../h/config.h
ryresponder.o: ../../h/ll_log.h
ryresponder.o: ryresponder.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
