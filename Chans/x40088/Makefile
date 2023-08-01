# PP X400 1988 modules
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/x40088/RCS/Makefile,v 6.0 1991/12/18 20:14:27 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:14:27  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= util.c x400in88.c x400out88.c x400topp.c x400inmsg.c \
		ut_rts.c ut_misc.c pptox400.c
OBJS	=

HEADERS	=	../../h 
LIBPP	=	../../Lib/libpp.a
#LIBSYS  =	-lm

LIBX400	= ../x40084/libx400.a
CFLAGS	= $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: xx400out88 xx400in88

lint:	l-x400out88 l-x400in88

############################################################
#
# x400out 1988 protocol
#
############################################################

X400OUTSRCS = x400out88.c pptox400.c util.c ut_rts.c ut_misc.c
X400OUTOBJS = x400out88.o pptox400.o util.o ut_rts.o ut_misc.o

x400out88: xx400out88

xx400out88: $(X400OUTOBJS) $(LIBX400) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(X400OUTOBJS) \
		$(LIBX400) $(LIBPP) $(LIBSYS)


saber_x400out88: $(X400OUTSRCS)
	#load -C $(CFLAGS) $(X400OUTSRCS) $(LIBX400) $(LIBPP) $(LIBSYS)

inst-x400out88: $(CHANDIR)/x400out88
$(CHANDIR)/x400out88: xx400out88
	-$(BACKUP) $@ zxx400out88
	rm -f $@
	$(INSTALL) xx400out88 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X400out88 channel installed normally"; echo ""

l-x400out88: $(SRCS)
	$(LINT) $(LLFLAGS) $(X400OUTSRCS) $(LINTLIBS)



############################################################
#
# x400in 1988 protocol
#
############################################################

X400INSRCS = x400in88.c x400topp.c  x400inmsg.c \
		ut_rts.c ut_misc.c
X400INOBJS = x400in88.o x400topp.o x400inmsg.o \
		ut_rts.o ut_misc.o

x400in88: xx400in88

xx400in88: $(X400INOBJS) $(LIBX400) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(X400INOBJS) \
		$(LIBX400) $(LIBPP)  $(LIBSYS)

saber_x400in88: $(X400INSRCS)
	#load -C $(CFLAGS) $(X400INSRCS) $(LIBX400) $(LIBPP) $(LIBSYS)

inst-x400in88: $(CHANDIR)/x400in88
$(CHANDIR)/x400in88: xx400in88
	-$(BACKUP) $@ zxx400in88
	rm -f $@
	$(INSTALL) xx400in88 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X400in88 channel installed normally"; echo ""

l-x400in88: $(SRCS)
	$(LINT) $(LLFLAGS) $(X400INSRCS) $(LINTLIBS)


############################################################
#
# p88
#
############################################################

p88: xp88

xp88: p88.o 
	$(CC) $(LDFLAGS) -o $@ p88.o $(LIBPP) $(LIBSYS)

l-p88: p88.c
	$(LINT) $(LLFLAGS) p88.c $(LINTLIBS)
############################################################
#
# Other stuff
#
############################################################

tasn: xtasn

xtasn: tasn.o ps2pe.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ tasn.o ps2pe.o \
		$(LIBPP) $(LIBSYS)

l-tasn: tasn.c ps2pe.c
	$(LINT) $(LLFLAGS) tasn.c ps2pe.c $(LINTLIBS)

install: inst-dir inst-x400out88 inst-x400in88
inst-dir: $(CHANDIR)


clean: tidy
	rm -f $(OBJS) *.o *.ph 

tidy:
	rm -f core a.out *.old MTA* xx400in88 xx400out88 xp88 xtasn

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
util.o: util.c
util.o: ../../h/util.h
util.o: ../../h/config.h
util.o: ../../h/ll_log.h
x400in88.o: x400in88.c
x400in88.o: ../../h/Trans-types.h
x400in88.o: ../../h/MTA-types.h
x400in88.o: ../../h/Toks-types.h
x400in88.o: ../../h/Auth-types.h
x400in88.o: ../../h/util.h
x400in88.o: ../../h/config.h
x400in88.o: ../../h/ll_log.h
x400in88.o: ../../h/chan.h
x400in88.o: ../../h/table.h
x400in88.o: ../../h/list_bpt.h
x400in88.o: ../../h/q.h
x400in88.o: ../../h/adr.h
x400in88.o: ../../h/list_rchan.h
x400in88.o: ../../h/chan.h
x400in88.o: ../../h/auth.h
x400in88.o: ../../h/list_bpt.h
x400in88.o: ../../h/extension.h
x400in88.o: ../../h/mta.h
x400in88.o: ../../h/adr.h
x400in88.o: ../../h/list_bpt.h
x400in88.o: ../../h/aparse.h
x400in88.o: ../../h/ap.h
x400in88.o: ../../h/util.h
x400in88.o: ../../h/or.h
x400in88.o: ../../h/chan.h
x400in88.o: ../../h/auth.h
x400in88.o: ../../h/list_rchan.h
x400in88.o: ../../h/mta.h
x400in88.o: ../../h/adr.h
x400in88.o: ../../h/or.h
x400in88.o: ../../h/retcode.h
x400in88.o: ../../h/RTS84-types.h
x400out88.o: x400out88.c
x400out88.o: ../../h/Trans-types.h
x400out88.o: ../../h/MTA-types.h
x400out88.o: ../../h/Toks-types.h
x400out88.o: ../../h/Auth-types.h
x400out88.o: ../../h/head.h
x400out88.o: ../../h/util.h
x400out88.o: ../../h/config.h
x400out88.o: ../../h/ll_log.h
x400out88.o: ../../h/retcode.h
x400out88.o: ../../h/prm.h
x400out88.o: ../../h/q.h
x400out88.o: ../../h/adr.h
x400out88.o: ../../h/list_rchan.h
x400out88.o: ../../h/chan.h
x400out88.o: ../../h/table.h
x400out88.o: ../../h/list_bpt.h
x400out88.o: ../../h/auth.h
x400out88.o: ../../h/list_bpt.h
x400out88.o: ../../h/extension.h
x400out88.o: ../../h/mta.h
x400out88.o: ../../h/adr.h
x400out88.o: ../../h/list_bpt.h
x400out88.o: ../../h/aparse.h
x400out88.o: ../../h/ap.h
x400out88.o: ../../h/util.h
x400out88.o: ../../h/or.h
x400out88.o: ../../h/chan.h
x400out88.o: ../../h/auth.h
x400out88.o: ../../h/list_rchan.h
x400out88.o: ../../h/mta.h
x400out88.o: ../../h/qmgr.h
x400out88.o: ../../h/Qmgr-types.h
x400out88.o: ../../h/Qmgr-ops.h
x400out88.o: ../../h/dr.h
x400out88.o: ../../h/extension.h
x400out88.o: ../../h/mta.h
x400out88.o: ../../h/rtsparams.h
x400out88.o: ../../h/sys.file.h
x400topp.o: x400topp.c
x400topp.o: ../../h/Trans-types.h
x400topp.o: ../../h/MTA-types.h
x400topp.o: ../../h/Toks-types.h
x400topp.o: ../../h/Auth-types.h
x400topp.o: ../../h/Ext-types.h
x400topp.o: ../../h/MTA-types.h
x400topp.o: ../../h/Toks-types.h
x400topp.o: ../../h/Auth-types.h
x400topp.o: ../../h/util.h
x400topp.o: ../../h/config.h
x400topp.o: ../../h/ll_log.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/table.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/q.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/list_rchan.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/auth.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/extension.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/aparse.h
x400topp.o: ../../h/ap.h
x400topp.o: ../../h/util.h
x400topp.o: ../../h/or.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/auth.h
x400topp.o: ../../h/list_rchan.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/or.h
x400topp.o: ../../h/prm.h
x400topp.o: ../../h/dr.h
x400topp.o: ../../h/extension.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/retcode.h
x400inmsg.o: x400inmsg.c
x400inmsg.o: ../../h/Trans-types.h
x400inmsg.o: ../../h/MTA-types.h
x400inmsg.o: ../../h/Toks-types.h
x400inmsg.o: ../../h/Auth-types.h
x400inmsg.o: ../../h/util.h
x400inmsg.o: ../../h/config.h
x400inmsg.o: ../../h/ll_log.h
x400inmsg.o: ../../h/chan.h
x400inmsg.o: ../../h/table.h
x400inmsg.o: ../../h/list_bpt.h
x400inmsg.o: ../../h/q.h
x400inmsg.o: ../../h/adr.h
x400inmsg.o: ../../h/list_rchan.h
x400inmsg.o: ../../h/chan.h
x400inmsg.o: ../../h/auth.h
x400inmsg.o: ../../h/list_bpt.h
x400inmsg.o: ../../h/extension.h
x400inmsg.o: ../../h/mta.h
x400inmsg.o: ../../h/adr.h
x400inmsg.o: ../../h/list_bpt.h
x400inmsg.o: ../../h/aparse.h
x400inmsg.o: ../../h/ap.h
x400inmsg.o: ../../h/util.h
x400inmsg.o: ../../h/or.h
x400inmsg.o: ../../h/chan.h
x400inmsg.o: ../../h/auth.h
x400inmsg.o: ../../h/list_rchan.h
x400inmsg.o: ../../h/mta.h
x400inmsg.o: ../../h/adr.h
x400inmsg.o: ../../h/or.h
x400inmsg.o: ../../h/prm.h
x400inmsg.o: ../../h/dr.h
x400inmsg.o: ../../h/extension.h
x400inmsg.o: ../../h/mta.h
x400inmsg.o: ../../h/retcode.h
ut_rts.o: ut_rts.c
ut_rts.o: ../../h/util.h
ut_rts.o: ../../h/config.h
ut_rts.o: ../../h/ll_log.h
ut_rts.o: ../../h/rtsparams.h
ut_rts.o: ../../h/chan.h
ut_rts.o: ../../h/table.h
ut_rts.o: ../../h/list_bpt.h
ut_rts.o: ../../h/Trans-types.h
ut_rts.o: ../../h/MTA-types.h
ut_rts.o: ../../h/Toks-types.h
ut_rts.o: ../../h/Auth-types.h
ut_misc.o: ut_misc.c
ut_misc.o: ../../h/util.h
ut_misc.o: ../../h/config.h
ut_misc.o: ../../h/ll_log.h
ut_misc.o: ../../h/tb_bpt88.h
ut_misc.o: ../../h/list_bpt.h
pptox400.o: pptox400.c
pptox400.o: ../../h/Trans-types.h
pptox400.o: ../../h/MTA-types.h
pptox400.o: ../../h/Toks-types.h
pptox400.o: ../../h/Auth-types.h
pptox400.o: ../../h/Ext-types.h
pptox400.o: ../../h/MTA-types.h
pptox400.o: ../../h/Toks-types.h
pptox400.o: ../../h/Auth-types.h
pptox400.o: ../../h/util.h
pptox400.o: ../../h/config.h
pptox400.o: ../../h/ll_log.h
pptox400.o: ../../h/q.h
pptox400.o: ../../h/adr.h
pptox400.o: ../../h/list_rchan.h
pptox400.o: ../../h/chan.h
pptox400.o: ../../h/table.h
pptox400.o: ../../h/list_bpt.h
pptox400.o: ../../h/auth.h
pptox400.o: ../../h/list_bpt.h
pptox400.o: ../../h/extension.h
pptox400.o: ../../h/mta.h
pptox400.o: ../../h/adr.h
pptox400.o: ../../h/list_bpt.h
pptox400.o: ../../h/aparse.h
pptox400.o: ../../h/ap.h
pptox400.o: ../../h/util.h
pptox400.o: ../../h/or.h
pptox400.o: ../../h/chan.h
pptox400.o: ../../h/auth.h
pptox400.o: ../../h/list_rchan.h
pptox400.o: ../../h/mta.h
pptox400.o: ../../h/adr.h
pptox400.o: ../../h/dr.h
pptox400.o: ../../h/extension.h
pptox400.o: ../../h/mta.h
pptox400.o: ../../h/or.h
pptox400.o: ../../h/rtsparams.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
