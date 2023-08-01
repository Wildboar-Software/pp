# Makefile for MTA console - x version
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/MTAconsole/RCS/Makefile,v 6.0 1991/12/18 20:26:48 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:26:48  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  assoc.c auto.c badness.c bops.c create.c data.c \
	   display.c drive.c misc.c ops.c parse.c popup.c workops.c
# WidgetWrap.c
OBJS =  assoc.o auto.o badness.o bops.o create.o data.o \
	   display.o drive.o misc.o ops.o parse.o popup.o workops.o 
#	   WidgetWrap.o 
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS = ../../h 
LIBPP   = ../../Lib/libpp.a
LM	= #/crg/users/jpo/tmp/malloc_debug/libmalloc.a
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE) 
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) -u $(INCLUDE) #-DXAW_BC 
LINTLIBS = $(LINTISODE) ../../Lib/llib-lpp.ln $(LINTX)

PROGS = xconsole

############################################################
#
# Building Rules
#
############################################################

default: default-$(X11)

default-true: $(PROGS)

default-false default-:; @echo "X11 not defined so not builing the MTAconsole"

############################################################
#
# Console
#
############################################################

console: xconsole
display.o: back.bit
badness.o: tai_defs.inc

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

xconsole: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LM)\
		$(LIBPP) $(LIBSYS) \
		$(LIBX) $(LM)

############################################################
#
# Misc stuff
#
############################################################

install: install-$(X11)

install- install-false:; @echo "X11 not defined so not installing MTAconsole"

install-true: inst-dir inst-MTAconsole;

inst-MTAconsole: $(CMDDIR)/MTAconsole $(APPDEFAULTS)/Mtaconsole

$(CMDDIR)/MTAconsole: xconsole
	-$(BACKUP) $@ zxconsole
	rm -f $@
	$(INSTALL) xconsole $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "MTAconsole installed normally"; echo ""

$(APPDEFAULTS)/Mtaconsole: Mtaconsole.ad
	-$(BACKUP) $@ zMtaconsole.ad
	rm -rf $@
	$(INSTALL) Mtaconsole.ad $@
	-$(CHMOD) 444 $@
	-$(CHOWN) $(ROOTUSER) $@
	-@ls -ls $@
	-@echo "MTAconsole application defaults installed normally"; echo ""

inst-dir: $(CMDDIR)

clean:  tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old $(PROGS) zxconsole


lint: l-console

l-console: $(SRCS) true
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

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
assoc.o: console.h
assoc.o: ../../h/util.h
assoc.o: ../../h/Qmgr-types.h
assoc.o: ../../h/Qmgr-ops.h
assoc.o: ../../h/qmgr.h
assoc.o: ../../h/Qmgr-types.h
assoc.o: ../../h/Qmgr-ops.h
auto.o: auto.c
auto.o: console.h
auto.o: ../../h/util.h
auto.o: ../../h/config.h
auto.o: ../../h/ll_log.h
auto.o: ../../h/Qmgr-types.h
badness.o: badness.c
badness.o: console.h
badness.o: ../../h/util.h
badness.o: ../../h/config.h
badness.o: ../../h/ll_log.h
badness.o: ../../h/Qmgr-types.h
badness.o: tai_defs.inc
bops.o: bops.c
bops.o: console.h
bops.o: ../../h/util.h
bops.o: ../../h/config.h
bops.o: ../../h/ll_log.h
bops.o: ../../h/Qmgr-types.h
create.o: create.c
create.o: console.h
create.o: ../../h/util.h
create.o: ../../h/config.h
create.o: ../../h/ll_log.h
create.o: ../../h/Qmgr-types.h
data.o: data.c
data.o: console.h
data.o: ../../h/util.h
data.o: ../../h/config.h
data.o: ../../h/ll_log.h
data.o: ../../h/Qmgr-types.h
data.o: ../../h/Qmgr-types.h
display.o: display.c
display.o: console.h
display.o: ../../h/util.h
display.o: ../../h/config.h
display.o: ../../h/ll_log.h
display.o: ../../h/Qmgr-types.h
drive.o: drive.c
drive.o: console.h
drive.o: ../../h/util.h
drive.o: ../../h/config.h
drive.o: ../../h/ll_log.h
drive.o: ../../h/Qmgr-types.h
misc.o: misc.c
misc.o: console.h
misc.o: ../../h/util.h
misc.o: ../../h/config.h
misc.o: ../../h/ll_log.h
misc.o: ../../h/Qmgr-types.h
misc.o: back.bit
ops.o: ops.c
ops.o: console.h
ops.o: ../../h/util.h
ops.o: ../../h/config.h
ops.o: ../../h/ll_log.h
ops.o: ../../h/Qmgr-types.h
ops.o: ../../h/Qmgr-types.h
parse.o: parse.c
parse.o: console.h
parse.o: ../../h/util.h
parse.o: ../../h/config.h
parse.o: ../../h/ll_log.h
parse.o: ../../h/Qmgr-types.h
popup.o: popup.c
popup.o: console.h
popup.o: ../../h/util.h
popup.o: ../../h/config.h
popup.o: ../../h/ll_log.h
popup.o: ../../h/Qmgr-types.h
workops.o: workops.c
workops.o: console.h
workops.o: ../../h/util.h
workops.o: ../../h/config.h
workops.o: ../../h/ll_log.h
workops.o: ../../h/Qmgr-types.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
