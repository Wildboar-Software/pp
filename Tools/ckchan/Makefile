# makefile for q checker
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ckchan/RCS/Makefile,v 6.0 1991/12/18 20:28:55 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:55  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ckchan.c qcheck.c
OBJS =  ckchan.o qcheck.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a
INCLUDE =       -I$(HEADERS)
CFLAGS  =       $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS =       $(LDOPTIONS) $(LLF)

LLFLAGS  =      $(LINTFLAGS) -u  $(INCLUDE)
LINTLIBS =      ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS    =      xckchan


############################################################
#
# Building Rules
#
############################################################


default: $(PROGS)

queue:  xqcheck

xqcheck:        qcheck.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ qcheck.o $(LIBPP) $(LIBSYS)

chan:           xckchan

xckchan:     ckchan.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ ckchan.o $(LIBPP) $(LIBSYS)

saber_src: ckchan.c
	#load -C $(CFLAGS) ckchan.c $(LIBPP) $(LIBSYS)

saber_obj: ckchan.o
	#load -C ckchan.o $(LIBPP) $(LIBSYS)


install: inst-dir inst-ckchan

inst-qcheck: $(CMDDIR)/qcheck
$(CMDDIR)/qcheck: xqcheck
	-$(BACKUP) $@ zxqcheck
	rm -f $@
	$(INSTALL) xqcheck $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "qcheck installed normally"; echo ""

inst-ckchan: $(CMDDIR)/ckchan
$(CMDDIR)/ckchan: xckchan
	-$(BACKUP) $@ zxckchan
	rm -f $@
	$(INSTALL) xckchan $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "ckchan installed normally"; echo ""

inst-dir: $(CMDDIR)


clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core a.out xckchan xqcheck *.old *.BAK zxckchan zxqcheck

lint: l-check

l-check:        $(SRCS) true
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
ckchan.o: ckchan.c
ckchan.o: ../../h/util.h
ckchan.o: ../../h/config.h
ckchan.o: ../../h/ll_log.h
ckchan.o: ../../h/q.h
ckchan.o: ../../h/adr.h
ckchan.o: ../../h/list_rchan.h
ckchan.o: ../../h/chan.h
ckchan.o: ../../h/table.h
ckchan.o: ../../h/list_bpt.h
ckchan.o: ../../h/auth.h
ckchan.o: ../../h/list_bpt.h
ckchan.o: ../../h/extension.h
ckchan.o: ../../h/mta.h
ckchan.o: ../../h/adr.h
ckchan.o: ../../h/list_bpt.h
ckchan.o: ../../h/mta.h
ckchan.o: ../../h/retcode.h
ckchan.o: ../../h/prm.h
qcheck.o: qcheck.c
qcheck.o: ../../h/util.h
qcheck.o: ../../h/config.h
qcheck.o: ../../h/ll_log.h
qcheck.o: ../../h/q.h
qcheck.o: ../../h/adr.h
qcheck.o: ../../h/list_rchan.h
qcheck.o: ../../h/chan.h
qcheck.o: ../../h/table.h
qcheck.o: ../../h/list_bpt.h
qcheck.o: ../../h/auth.h
qcheck.o: ../../h/list_bpt.h
qcheck.o: ../../h/extension.h
qcheck.o: ../../h/mta.h
qcheck.o: ../../h/adr.h
qcheck.o: ../../h/list_bpt.h
qcheck.o: ../../h/mta.h
qcheck.o: ../../h/retcode.h
qcheck.o: ../../h/prm.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
