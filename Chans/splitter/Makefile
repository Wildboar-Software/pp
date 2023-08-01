# Preliminary comments
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/splitter/RCS/Makefile,v 6.0 1991/12/18 20:12:36 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:36  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= splitter.c
OBJS	= splitter.o

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

PROGS	= xsplitter

default: $(PROGS)

xsplitter: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj:$(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-splitter inst-dir

inst-splitter:	$(CHANDIR)/splitter
$(CHANDIR)/splitter: xsplitter
	-$(BACKUP) $@ zxsplitter
	rm -f $@
	$(INSTALL) xsplitter $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "splitter channel installed normally"; echo ""

inst-dir: $(CHANDIR)

clean: tidy
tidy:
	rm -f $(OBJS) xsplitter core a.out Makefile.old

lint: l-targets

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
splitter.o: splitter.c
splitter.o: ../../h/util.h
splitter.o: ../../h/config.h
splitter.o: ../../h/ll_log.h
splitter.o: ../../h/qmgr.h
splitter.o: ../../h/Qmgr-types.h
splitter.o: ../../h/Qmgr-ops.h
splitter.o: ../../h/q.h
splitter.o: ../../h/adr.h
splitter.o: ../../h/list_rchan.h
splitter.o: ../../h/chan.h
splitter.o: ../../h/table.h
splitter.o: ../../h/list_bpt.h
splitter.o: ../../h/auth.h
splitter.o: ../../h/list_bpt.h
splitter.o: ../../h/extension.h
splitter.o: ../../h/mta.h
splitter.o: ../../h/adr.h
splitter.o: ../../h/list_bpt.h
splitter.o: ../../h/aparse.h
splitter.o: ../../h/ap.h
splitter.o: ../../h/util.h
splitter.o: ../../h/or.h
splitter.o: ../../h/chan.h
splitter.o: ../../h/auth.h
splitter.o: ../../h/list_rchan.h
splitter.o: ../../h/mta.h
splitter.o: ../../h/prm.h
splitter.o: ../../h/retcode.h
splitter.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
