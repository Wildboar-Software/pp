# Makefile for rfc934 filter channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/rfc934/RCS/Makefile,v 6.0 1991/12/18 20:21:02 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:21:02  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  do-rfc934.c rfc934.c
OBJS =  do-rfc934.o rfc934.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)


PROGS   = xrfc934 


############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# rfc934 filter
#
############################################################

rfc934: xrfc934

xrfc934: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) \
		$(LIBPP) $(LIBSYS)



############################################################
#
# Misc stuff
#
############################################################

install: inst-dir inst-rfc934

inst-rfc934: $(CHANDIR)/rfc934
$(CHANDIR)/rfc934: xrfc934
	-$(BACKUP) $@ zxrfc934
	rm -f $@
	$(INSTALL) xrfc934 $@ 
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@ 
	-@ls -ls $@
	-@echo "rfc934 channel installed normally"; echo ""

inst-dir: $(CHANDIR)

clean: tidy
	rm -f $(OBJS)
tidy:
	rm -f $(PROGS) zxrfc934 core a.out *.old *.BAK

lint: l-rfc934

l-rfc934: $(SRCS) true
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
do-rfc934.o: do-rfc934.c
do-rfc934.o: ../../h/util.h
do-rfc934.o: ../../h/config.h
do-rfc934.o: ../../h/ll_log.h
do-rfc934.o: ../../h/retcode.h
do-rfc934.o: ../../h/tb_bpt84.h
rfc934.o: rfc934.c
rfc934.o: ../../h/util.h
rfc934.o: ../../h/config.h
rfc934.o: ../../h/ll_log.h
rfc934.o: ../../h/head.h
rfc934.o: ../../h/util.h
rfc934.o: ../../h/retcode.h
rfc934.o: ../../h/qmgr.h
rfc934.o: ../../h/Qmgr-types.h
rfc934.o: ../../h/Qmgr-ops.h
rfc934.o: ../../h/q.h
rfc934.o: ../../h/adr.h
rfc934.o: ../../h/list_rchan.h
rfc934.o: ../../h/chan.h
rfc934.o: ../../h/table.h
rfc934.o: ../../h/list_bpt.h
rfc934.o: ../../h/auth.h
rfc934.o: ../../h/list_bpt.h
rfc934.o: ../../h/extension.h
rfc934.o: ../../h/mta.h
rfc934.o: ../../h/adr.h
rfc934.o: ../../h/list_bpt.h
rfc934.o: ../../h/aparse.h
rfc934.o: ../../h/ap.h
rfc934.o: ../../h/util.h
rfc934.o: ../../h/or.h
rfc934.o: ../../h/chan.h
rfc934.o: ../../h/auth.h
rfc934.o: ../../h/list_rchan.h
rfc934.o: ../../h/mta.h
rfc934.o: ../../h/prm.h
rfc934.o: ../../h/chan.h
rfc934.o: ../../h/dr.h
rfc934.o: ../../h/extension.h
rfc934.o: ../../h/mta.h
rfc934.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
