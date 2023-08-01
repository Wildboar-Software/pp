# Makefile for qmgr - ros version
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/qmgr/RCS/Makefile,v 6.0 1991/12/18 20:27:38 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:27:38  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  assoc.c chans.c misc.c qmgr.c ryresponder.c structs.c tables.c
OBJS =  assoc.o chans.o misc.o qmgr.o ryresponder.o structs.o tables.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS = ../../h
INCLUDE = -I$(HEADERS)
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE) $(PROF)
LDFLAGS = $(LDOPTIONS) $(LLF) $(PROF)

LLFLAGS = -bhuz -u $(INCLUDE)
LINTLIBS =  ../../Lib/llib-lpp.ln
PROGS = xqmgr

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# Queue manager
#
############################################################

inst-qmgr: $(CMDDIR)/qmgr

$(CMDDIR)/qmgr: xqmgr
	-$(BACKUP) $@ zxqmgr
	rm -f $@
	$(INSTALL) xqmgr $@
	-$(CHOWN) $(PPUSER) $@
	-$(CHMOD) $(PGMPROT) $@
	-@ls -ls $@
	-@echo "Queue Manager installed normally"; echo ""

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS)  $(LIBPP) $(LIBSYS)

qmgr: xqmgr

xqmgr: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

qmgr.o: ryresponder.h

############################################################
#
# Misc stuff
#
############################################################

install: inst-dir inst-qmgr

tags: TAGS
TAGS: $(SRCS)
	etags -t $(SRCS)

inst-dir: $(CMDDIR)


clean:  tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old *.BAK $(PROGS) zx*


lint: l-qmgr

l-qmgr: $(SRCS) true
	$(LINT) $(LLFLAGS) -DPERFORMER -DINVOKER $(SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

true:;

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
assoc.o: assoc.c
assoc.o: ../../h/util.h
assoc.o: ../../h/config.h
assoc.o: ../../h/ll_log.h
assoc.o: ./types.h
assoc.o: ../../h/Qmgr-types.h
assoc.o: ../../h/table.h
assoc.o: ../../h/chan.h
assoc.o: ../../h/table.h
assoc.o: ../../h/list_bpt.h
assoc.o: ../../h/list_bpt.h
assoc.o: ../../h/list_rchan.h
assoc.o: ../../h/chan.h
assoc.o: ../../h/auth.h
assoc.o: ../../h/list_bpt.h
assoc.o: ../../h/mta.h
assoc.o: ../../h/adr.h
assoc.o: ../../h/list_rchan.h
assoc.o: ../../h/extension.h
assoc.o: ../../h/mta.h
assoc.o: ../../h/aparse.h
assoc.o: ../../h/ap.h
assoc.o: ../../h/util.h
assoc.o: ../../h/or.h
assoc.o: ../../h/chan.h
assoc.o: ../../h/auth.h
assoc.o: ../../h/list_rchan.h
assoc.o: ../../h/list_bpt.h
assoc.o: ../../h/qmgr.h
assoc.o: ../../h/Qmgr-types.h
assoc.o: ../../h/Qmgr-ops.h
chans.o: chans.c
chans.o: ./types.h
chans.o: ../../h/Qmgr-types.h
chans.o: ../../h/table.h
chans.o: ../../h/chan.h
chans.o: ../../h/table.h
chans.o: ../../h/list_bpt.h
chans.o: ../../h/list_bpt.h
chans.o: ../../h/list_rchan.h
chans.o: ../../h/chan.h
chans.o: ../../h/auth.h
chans.o: ../../h/list_bpt.h
chans.o: ../../h/mta.h
chans.o: ../../h/adr.h
chans.o: ../../h/list_rchan.h
chans.o: ../../h/extension.h
chans.o: ../../h/mta.h
chans.o: ../../h/aparse.h
chans.o: ../../h/ap.h
chans.o: ../../h/util.h
chans.o: ../../h/config.h
chans.o: ../../h/ll_log.h
chans.o: ../../h/or.h
chans.o: ../../h/chan.h
chans.o: ../../h/auth.h
chans.o: ../../h/list_rchan.h
chans.o: ../../h/list_bpt.h
chans.o: ../../h/Qmgr-ops.h
chans.o: ../../h/util.h
misc.o: misc.c
misc.o: ./types.h
misc.o: ../../h/Qmgr-types.h
misc.o: ../../h/table.h
misc.o: ../../h/chan.h
misc.o: ../../h/table.h
misc.o: ../../h/list_bpt.h
misc.o: ../../h/list_bpt.h
misc.o: ../../h/list_rchan.h
misc.o: ../../h/chan.h
misc.o: ../../h/auth.h
misc.o: ../../h/list_bpt.h
misc.o: ../../h/mta.h
misc.o: ../../h/adr.h
misc.o: ../../h/list_rchan.h
misc.o: ../../h/extension.h
misc.o: ../../h/mta.h
misc.o: ../../h/aparse.h
misc.o: ../../h/ap.h
misc.o: ../../h/util.h
misc.o: ../../h/config.h
misc.o: ../../h/ll_log.h
misc.o: ../../h/or.h
misc.o: ../../h/chan.h
misc.o: ../../h/auth.h
misc.o: ../../h/list_rchan.h
misc.o: ../../h/list_bpt.h
qmgr.o: qmgr.c
qmgr.o: ../../h/util.h
qmgr.o: ../../h/config.h
qmgr.o: ../../h/ll_log.h
qmgr.o: ./ryresponder.h
qmgr.o: ../../h/qmgr.h
qmgr.o: ../../h/Qmgr-types.h
qmgr.o: ../../h/Qmgr-ops.h
qmgr.o: ./types.h
qmgr.o: ../../h/Qmgr-types.h
qmgr.o: ../../h/table.h
qmgr.o: ../../h/chan.h
qmgr.o: ../../h/table.h
qmgr.o: ../../h/list_bpt.h
qmgr.o: ../../h/list_bpt.h
qmgr.o: ../../h/list_rchan.h
qmgr.o: ../../h/chan.h
qmgr.o: ../../h/auth.h
qmgr.o: ../../h/list_bpt.h
qmgr.o: ../../h/mta.h
qmgr.o: ../../h/adr.h
qmgr.o: ../../h/list_rchan.h
qmgr.o: ../../h/extension.h
qmgr.o: ../../h/mta.h
qmgr.o: ../../h/aparse.h
qmgr.o: ../../h/ap.h
qmgr.o: ../../h/util.h
qmgr.o: ../../h/or.h
qmgr.o: ../../h/chan.h
qmgr.o: ../../h/auth.h
qmgr.o: ../../h/list_rchan.h
qmgr.o: ../../h/list_bpt.h
ryresponder.o: ryresponder.c
ryresponder.o: ../../h/util.h
ryresponder.o: ../../h/config.h
ryresponder.o: ../../h/ll_log.h
ryresponder.o: ../../h/qmgr.h
ryresponder.o: ../../h/Qmgr-types.h
ryresponder.o: ../../h/Qmgr-ops.h
ryresponder.o: ./ryresponder.h
structs.o: structs.c
structs.o: ../../h/util.h
structs.o: ../../h/config.h
structs.o: ../../h/ll_log.h
structs.o: ./types.h
structs.o: ../../h/Qmgr-types.h
structs.o: ../../h/table.h
structs.o: ../../h/chan.h
structs.o: ../../h/table.h
structs.o: ../../h/list_bpt.h
structs.o: ../../h/list_bpt.h
structs.o: ../../h/list_rchan.h
structs.o: ../../h/chan.h
structs.o: ../../h/auth.h
structs.o: ../../h/list_bpt.h
structs.o: ../../h/mta.h
structs.o: ../../h/adr.h
structs.o: ../../h/list_rchan.h
structs.o: ../../h/extension.h
structs.o: ../../h/mta.h
structs.o: ../../h/aparse.h
structs.o: ../../h/ap.h
structs.o: ../../h/util.h
structs.o: ../../h/or.h
structs.o: ../../h/chan.h
structs.o: ../../h/auth.h
structs.o: ../../h/list_rchan.h
structs.o: ../../h/list_bpt.h
structs.o: ../../h/Qmgr-ops.h
tables.o: tables.c
tables.o: ../../h/util.h
tables.o: ../../h/config.h
tables.o: ../../h/ll_log.h
tables.o: ./types.h
tables.o: ../../h/Qmgr-types.h
tables.o: ../../h/table.h
tables.o: ../../h/chan.h
tables.o: ../../h/table.h
tables.o: ../../h/list_bpt.h
tables.o: ../../h/list_bpt.h
tables.o: ../../h/list_rchan.h
tables.o: ../../h/chan.h
tables.o: ../../h/auth.h
tables.o: ../../h/list_bpt.h
tables.o: ../../h/mta.h
tables.o: ../../h/adr.h
tables.o: ../../h/list_rchan.h
tables.o: ../../h/extension.h
tables.o: ../../h/mta.h
tables.o: ../../h/aparse.h
tables.o: ../../h/ap.h
tables.o: ../../h/util.h
tables.o: ../../h/or.h
tables.o: ../../h/chan.h
tables.o: ../../h/auth.h
tables.o: ../../h/list_rchan.h
tables.o: ../../h/list_bpt.h
tables.o: ../../h/qmgr.h
tables.o: ../../h/Qmgr-types.h
tables.o: ../../h/Qmgr-ops.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
