# Makefile for line based console
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/lconsole/RCS/Makefile,v 6.0 1991/12/18 20:27:16 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:27:16  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	=	lconsole.c functions.c channel.c mtas.c msgs.c show.c
OBJS	=	lconsole.o functions.o channel.o mtas.o msgs.o show.o


HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: lconsole

lconsole: xlconsole
	@true

xlconsole:	$(OBJS) $(LIBPP)
	$(CC) -o $@ $(LDOPTIONS) $(OBJS) $(LIBPP) $(LIBSYS) $(LM)

install: inst-xlconsole

inst-xlconsole: $(CMDDIR)/lconsole
$(CMDDIR)/lconsole: xlconsole
	-$(BACKUP) $@ zxlconsole
	rm -f $@
	$(INSTALL) xlconsole $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "lconsole installed normally"; echo ""

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS) $(LM)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS) $(LM)

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old xlconsole

lint: l-lconsole

l-lconsole: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)
depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
lconsole.o: lconsole.c
lconsole.o: lconsole.h
lconsole.o: ../../h/util.h
lconsole.o: ../../h/config.h
lconsole.o: ../../h/ll_log.h
lconsole.o: ../../h/qmgr-int.h
functions.o: functions.c
functions.o: lconsole.h
functions.o: ../../h/util.h
functions.o: ../../h/config.h
functions.o: ../../h/ll_log.h
functions.o: ../../h/qmgr.h
functions.o: ../../h/Qmgr-types.h
functions.o: ../../h/Qmgr-ops.h
functions.o: ../../h/qmgr-int.h
channel.o: channel.c
channel.o: lconsole.h
channel.o: ../../h/util.h
channel.o: ../../h/config.h
channel.o: ../../h/ll_log.h
channel.o: ../../h/qmgr.h
channel.o: ../../h/Qmgr-types.h
channel.o: ../../h/Qmgr-ops.h
channel.o: ../../h/qmgr-int.h
mtas.o: mtas.c
mtas.o: lconsole.h
mtas.o: ../../h/util.h
mtas.o: ../../h/config.h
mtas.o: ../../h/ll_log.h
mtas.o: ../../h/qmgr.h
mtas.o: ../../h/Qmgr-types.h
mtas.o: ../../h/Qmgr-ops.h
mtas.o: ../../h/qmgr-int.h
msgs.o: msgs.c
msgs.o: lconsole.h
msgs.o: ../../h/util.h
msgs.o: ../../h/config.h
msgs.o: ../../h/ll_log.h
msgs.o: ../../h/qmgr.h
msgs.o: ../../h/Qmgr-types.h
msgs.o: ../../h/Qmgr-ops.h
msgs.o: ../../h/qmgr-int.h
show.o: show.c
show.o: lconsole.h
show.o: ../../h/util.h
show.o: ../../h/config.h
show.o: ../../h/ll_log.h
show.o: ../../h/qmgr.h
show.o: ../../h/Qmgr-types.h
show.o: ../../h/Qmgr-ops.h
show.o: ../../h/qmgr-int.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
