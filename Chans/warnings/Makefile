# Preliminary comments
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/warnings/RCS/Makefile,v 6.0 1991/12/18 20:13:32 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:13:32  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= warnings.c
OBJS	= warnings.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS = xwarnings

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# warnings channel
#
############################################################

xwarnings: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# misc stuff
#
############################################################

install: inst-dir inst-warnings

inst-warnings: $(CHANDIR)/warnings
$(CHANDIR)/warnings: xwarnings
	-$(BACKUP) $@ zxwarnings
	rm -f $@
	$(INSTALL) xwarnings $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "warnings channel installed normally"; echo ""

inst-dir: $(CHANDIR)


clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f $(PROGS) core a.out Makefile.old

lint: l-warnings

l-warnings: $(SRCS) true
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
warnings.o: warnings.c
warnings.o: ../../h/util.h
warnings.o: ../../h/config.h
warnings.o: ../../h/ll_log.h
warnings.o: ../../h/retcode.h
warnings.o: ../../h/expand.h
warnings.o: ../../h/qmgr.h
warnings.o: ../../h/Qmgr-types.h
warnings.o: ../../h/Qmgr-ops.h
warnings.o: ../../h/q.h
warnings.o: ../../h/adr.h
warnings.o: ../../h/list_rchan.h
warnings.o: ../../h/chan.h
warnings.o: ../../h/table.h
warnings.o: ../../h/list_bpt.h
warnings.o: ../../h/auth.h
warnings.o: ../../h/list_bpt.h
warnings.o: ../../h/extension.h
warnings.o: ../../h/mta.h
warnings.o: ../../h/adr.h
warnings.o: ../../h/list_bpt.h
warnings.o: ../../h/aparse.h
warnings.o: ../../h/ap.h
warnings.o: ../../h/util.h
warnings.o: ../../h/or.h
warnings.o: ../../h/chan.h
warnings.o: ../../h/auth.h
warnings.o: ../../h/list_rchan.h
warnings.o: ../../h/mta.h
warnings.o: ../../h/prm.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
