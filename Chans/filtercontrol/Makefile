# Makefile for filter control channel - ros version
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/filtercontrol/RCS/Makefile,v 6.0 1991/12/18 20:10:08 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:10:08  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  filtercont.c
OBJS =  filtercont.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE) #-DDBG_CHAN
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -u $(INCLUDE)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS = xfilter 

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# Filter control
#
############################################################

xfilter: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj:$(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################

install: inst-dir inst-fcontrol inst-removebp

inst-fcontrol: $(CHANDIR)/fcontrol
$(CHANDIR)/fcontrol: xfilter
	-$(BACKUP) $@ zxfilter
	rm -f $@
	$(INSTALL) xfilter $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "fcontrol channel installed normally"; echo ""

inst-dir: $(CHANDIR)

inst-removebp: $(FORMDIR)/removebp
$(FORMDIR)/removebp: removebp.sh
	-$(BACKUP) $@ zxremovebp
	rm -f $@
	$(INSTALL) removebp.sh $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "removebp script installed normally"; echo ""

clean:  tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old zxfilter $(PROGS) *.BAK


lint: l-filter

l-filter: $(SRCS) true
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
filtercont.o: filtercont.c
filtercont.o: ../../h/util.h
filtercont.o: ../../h/config.h
filtercont.o: ../../h/ll_log.h
filtercont.o: ../../h/retcode.h
filtercont.o: ../../h/expand.h
filtercont.o: ../../h/qmgr.h
filtercont.o: ../../h/Qmgr-types.h
filtercont.o: ../../h/Qmgr-ops.h
filtercont.o: ../../h/q.h
filtercont.o: ../../h/adr.h
filtercont.o: ../../h/list_rchan.h
filtercont.o: ../../h/chan.h
filtercont.o: ../../h/table.h
filtercont.o: ../../h/list_bpt.h
filtercont.o: ../../h/auth.h
filtercont.o: ../../h/list_bpt.h
filtercont.o: ../../h/extension.h
filtercont.o: ../../h/mta.h
filtercont.o: ../../h/adr.h
filtercont.o: ../../h/list_bpt.h
filtercont.o: ../../h/aparse.h
filtercont.o: ../../h/ap.h
filtercont.o: ../../h/util.h
filtercont.o: ../../h/or.h
filtercont.o: ../../h/chan.h
filtercont.o: ../../h/auth.h
filtercont.o: ../../h/list_rchan.h
filtercont.o: ../../h/mta.h
filtercont.o: ../../h/prm.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
