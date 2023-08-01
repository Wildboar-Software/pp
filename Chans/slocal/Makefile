# slocal - structured local delivery channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/slocal/RCS/Makefile,v 6.0 1991/12/18 20:12:09 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:09  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS = slocal.c loc_proto.c
OBJS = slocal.o loc_proto.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
INCLUDE         = -I$(HEADERS)

CFLAGS          = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) $(INCLUDE)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS = xslocal

default: xslocal

slocal: xslocal

xslocal: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBPP) $(LIBSYS) -o $@

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f $(PROGS) core a.out *old *.BAK zxslocal

lint: l-slocal
l-slocal: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)


install: inst-slocal
inst-slocal: $(CHANDIR)/slocal
$(CHANDIR)/slocal: xslocal
	-$(BACKUP) $@ zxslocal
	rm -f $@
	$(INSTALL) xslocal $@
	-$(CHOWN) $(ROOTUSER) $@
	-$(CHMOD) 4755 $@
	-@ls -ls $@
	-@echo "Slocal channel installed normally"; echo ""

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of Building Rules
#
############################################################


# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
slocal.o: slocal.c
slocal.o: ../../h/head.h
slocal.o: ../../h/util.h
slocal.o: ../../h/config.h
slocal.o: ../../h/ll_log.h
slocal.o: ../../h/retcode.h
slocal.o: ../../h/util.h
slocal.o: ../../h/prm.h
slocal.o: ../../h/q.h
slocal.o: ../../h/adr.h
slocal.o: ../../h/list_rchan.h
slocal.o: ../../h/chan.h
slocal.o: ../../h/table.h
slocal.o: ../../h/list_bpt.h
slocal.o: ../../h/auth.h
slocal.o: ../../h/list_bpt.h
slocal.o: ../../h/extension.h
slocal.o: ../../h/mta.h
slocal.o: ../../h/adr.h
slocal.o: ../../h/list_bpt.h
slocal.o: ../../h/aparse.h
slocal.o: ../../h/ap.h
slocal.o: ../../h/util.h
slocal.o: ../../h/or.h
slocal.o: ../../h/chan.h
slocal.o: ../../h/auth.h
slocal.o: ../../h/list_rchan.h
slocal.o: ../../h/mta.h
slocal.o: ../../h/qmgr.h
slocal.o: ../../h/Qmgr-types.h
slocal.o: ../../h/Qmgr-ops.h
slocal.o: ../../h/dr.h
slocal.o: ../../h/extension.h
slocal.o: ../../h/mta.h
slocal.o: ../../h/loc_user.h
slocal.o: ../../h/sys.file.h
loc_proto.o: loc_proto.c
loc_proto.o: ../../h/util.h
loc_proto.o: ../../h/config.h
loc_proto.o: ../../h/ll_log.h
loc_proto.o: ../../h/retcode.h
loc_proto.o: ../../h/sys.file.h
loc_proto.o: ../../h/loc_user.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
