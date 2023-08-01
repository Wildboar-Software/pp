# Makefile for MTA console - line based
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/LINEconsole/RCS/Makefile,v 6.0 1991/12/18 20:26:30 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:26:30  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= assoc.c drive.c io.c \
	channels.c mtas.c msgs.c \
	heuristics.c status.c misc.c badness.c
OBJS	= assoc.o drive.o io.o \
	channels.o mtas.o msgs.o \
	heuristics.o status.o misc.o badness.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)
MAKE	= ./make

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS	 = xconsole

############################################################
#
# Console
#
############################################################

console: xconsole

xconsole: $(OBJS) $(LIBPP)
	  $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-dir inst-LINEconsole

inst-LINEconsole: $(CMDDIR)/LINEconsole
$(CMDDIR)/LINEconsole: xconsole
	-$(BACKUP) $@ zxconsole
	rm -f $@
	$(INSTALL) xconsole $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "LINEconsole installed normally"; echo ""

inst-dir: $(CMDDIR)


lint: l-console

l-console: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:	tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old $(PROGS) zxconsole

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
assoc.o: assoc.c
assoc.o: console.h
assoc.o: ../../h/head.h
assoc.o: ../../h/util.h
assoc.o: ../../h/config.h
assoc.o: ../../h/ll_log.h
assoc.o: ../../h/retcode.h
assoc.o: ../../h/sys.file.h
assoc.o: ../../h/qmgr.h
assoc.o: ../../h/Qmgr-types.h
assoc.o: ../../h/Qmgr-ops.h
drive.o: drive.c
drive.o: console.h
drive.o: ../../h/head.h
drive.o: ../../h/util.h
drive.o: ../../h/config.h
drive.o: ../../h/ll_log.h
drive.o: ../../h/retcode.h
drive.o: ../../h/sys.file.h
drive.o: ../../h/qmgr.h
drive.o: ../../h/Qmgr-types.h
drive.o: ../../h/Qmgr-ops.h
io.o: io.c
io.o: console.h
io.o: ../../h/head.h
io.o: ../../h/util.h
io.o: ../../h/config.h
io.o: ../../h/ll_log.h
io.o: ../../h/retcode.h
io.o: ../../h/sys.file.h
io.o: ../../h/qmgr.h
io.o: ../../h/Qmgr-types.h
io.o: ../../h/Qmgr-ops.h
channels.o: channels.c
channels.o: console.h
channels.o: ../../h/head.h
channels.o: ../../h/util.h
channels.o: ../../h/config.h
channels.o: ../../h/ll_log.h
channels.o: ../../h/retcode.h
channels.o: ../../h/sys.file.h
channels.o: ../../h/qmgr.h
channels.o: ../../h/Qmgr-types.h
channels.o: ../../h/Qmgr-ops.h
mtas.o: mtas.c
mtas.o: console.h
mtas.o: ../../h/head.h
mtas.o: ../../h/util.h
mtas.o: ../../h/config.h
mtas.o: ../../h/ll_log.h
mtas.o: ../../h/retcode.h
mtas.o: ../../h/sys.file.h
mtas.o: ../../h/qmgr.h
mtas.o: ../../h/Qmgr-types.h
mtas.o: ../../h/Qmgr-ops.h
msgs.o: msgs.c
msgs.o: console.h
msgs.o: ../../h/head.h
msgs.o: ../../h/util.h
msgs.o: ../../h/config.h
msgs.o: ../../h/ll_log.h
msgs.o: ../../h/retcode.h
msgs.o: ../../h/sys.file.h
msgs.o: ../../h/qmgr.h
msgs.o: ../../h/Qmgr-types.h
msgs.o: ../../h/Qmgr-ops.h
heuristics.o: heuristics.c
heuristics.o: console.h
heuristics.o: ../../h/head.h
heuristics.o: ../../h/util.h
heuristics.o: ../../h/config.h
heuristics.o: ../../h/ll_log.h
heuristics.o: ../../h/retcode.h
heuristics.o: ../../h/sys.file.h
heuristics.o: ../../h/qmgr.h
heuristics.o: ../../h/Qmgr-types.h
heuristics.o: ../../h/Qmgr-ops.h
status.o: status.c
status.o: console.h
status.o: ../../h/head.h
status.o: ../../h/util.h
status.o: ../../h/config.h
status.o: ../../h/ll_log.h
status.o: ../../h/retcode.h
status.o: ../../h/sys.file.h
status.o: ../../h/qmgr.h
status.o: ../../h/Qmgr-types.h
status.o: ../../h/Qmgr-ops.h
misc.o: misc.c
misc.o: console.h
misc.o: ../../h/head.h
misc.o: ../../h/util.h
misc.o: ../../h/config.h
misc.o: ../../h/ll_log.h
misc.o: ../../h/retcode.h
misc.o: ../../h/sys.file.h
misc.o: ../../h/qmgr.h
misc.o: ../../h/Qmgr-types.h
misc.o: ../../h/Qmgr-ops.h
badness.o: badness.c
badness.o: console.h
badness.o: ../../h/head.h
badness.o: ../../h/util.h
badness.o: ../../h/config.h
badness.o: ../../h/ll_log.h
badness.o: ../../h/retcode.h
badness.o: ../../h/sys.file.h
badness.o: ../../h/qmgr.h
badness.o: ../../h/Qmgr-types.h
badness.o: ../../h/Qmgr-ops.h
badness.o: tai_defs.inc

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
