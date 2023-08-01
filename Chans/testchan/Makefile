# test channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/testchan/RCS/Makefile,v 6.0 1991/12/18 20:12:42 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:42  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS = testchan.c
OBJS = testchan.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a 
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) $(INCLUDE)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

default: testchan

lint: l-testchan

############################################################
#
# test channel
#
############################################################

testchan: xtestchan

xtestchan: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj:$(OBJS)
	#load -C smtp.o $(SMTPOBJS) $(LIBPP) $(LIBSYS)

l-testchan: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

install: inst-dir inst-testchan

inst-dir: $(CHANDIR)

inst-testchan: $(CHANDIR)/testchan

$(CHANDIR)/testchan: xtestchan
	-$(BACKUP) $@ zxtestchan
	rm -f $@
	$(INSTALL) xtestchan $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "testchan channel installed normally"; echo ""


clean: tidy
	rm -f $(OBJS) $(PROGS) core a.out *old *.BAK z* x*

tidy:
	rm -f $(PROGS) core a.out *old *.BAK

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
testchan.o: testchan.c
testchan.o: ../../h/head.h
testchan.o: ../../h/util.h
testchan.o: ../../h/config.h
testchan.o: ../../h/ll_log.h
testchan.o: ../../h/retcode.h
testchan.o: ../../h/chan.h
testchan.o: ../../h/table.h
testchan.o: ../../h/list_bpt.h
testchan.o: ../../h/prm.h
testchan.o: ../../h/q.h
testchan.o: ../../h/adr.h
testchan.o: ../../h/list_rchan.h
testchan.o: ../../h/chan.h
testchan.o: ../../h/auth.h
testchan.o: ../../h/list_bpt.h
testchan.o: ../../h/extension.h
testchan.o: ../../h/mta.h
testchan.o: ../../h/adr.h
testchan.o: ../../h/list_bpt.h
testchan.o: ../../h/aparse.h
testchan.o: ../../h/ap.h
testchan.o: ../../h/util.h
testchan.o: ../../h/or.h
testchan.o: ../../h/chan.h
testchan.o: ../../h/auth.h
testchan.o: ../../h/list_rchan.h
testchan.o: ../../h/mta.h
testchan.o: ../../h/dr.h
testchan.o: ../../h/extension.h
testchan.o: ../../h/mta.h
testchan.o: ../../h/qmgr.h
testchan.o: ../../h/Qmgr-types.h
testchan.o: ../../h/Qmgr-ops.h
testchan.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
