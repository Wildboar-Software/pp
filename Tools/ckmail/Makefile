# Makefile for checkmail program
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ckmail/RCS/Makefile,v 6.0 1991/12/18 20:29:15 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:29:15  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ckmail.c ryinitiator.c
OBJS =  ckmail.o ryinitiator.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h -I/usr/include/isode
LIBPP   = ../../Lib/libpp.a
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) -u $(INCLUDE)
LINTLIBS = $(LINTISODE) ../../Lib/llib-lpp.ln

PROGS = xcheckmail

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# CheckMail
#
############################################################

checkmail: xcheckmail

xcheckmail: $(OBJS) $(LIBPP)
	    -rm -f $@
	    $(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

ckmail.o:   ryinitiator.h
ryinitiator.o:  ryinitiator.h

saber_ckmail: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################


install: inst-dir inst-checkmail
inst-checkmail: $(USRBINDIR)/checkmail
$(USRBINDIR)/checkmail: xcheckmail
	-$(BACKUP) $@ zxcheckmail
	-rm -f $@
	$(INSTALL) xcheckmail $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "checkmail tool installed normally"; echo ""

inst-dir: $(TOOLDIR)


clean:  tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old $(PROGS) *.BAK zx*


lint: l-checkmail

l-checkmail: $(SRCS) true
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
ckmail.o: ckmail.c
ckmail.o: ../../h/util.h
ckmail.o: ../../h/config.h
ckmail.o: ../../h/ll_log.h
ckmail.o: ../../h/Qmgr-ops.h
ckmail.o: ../../h/Qmgr-types.h
ckmail.o: ../../h/retcode.h
ckmail.o: ../../h/adr.h
ckmail.o: ../../h/list_rchan.h
ckmail.o: ../../h/chan.h
ckmail.o: ../../h/table.h
ckmail.o: ../../h/list_bpt.h
ckmail.o: ../../h/auth.h
ckmail.o: ../../h/list_bpt.h
ckmail.o: ../../h/extension.h
ckmail.o: ../../h/mta.h
ckmail.o: ../../h/adr.h
ckmail.o: ../../h/list_bpt.h
ckmail.o: ../../h/aparse.h
ckmail.o: ../../h/ap.h
ckmail.o: ../../h/util.h
ckmail.o: ../../h/or.h
ckmail.o: ../../h/chan.h
ckmail.o: ../../h/auth.h
ckmail.o: ../../h/list_rchan.h
ckmail.o: ../../h/alias.h
ckmail.o: ../../h/ap.h
ckmail.o: ../../h/or.h
ckmail.o: ./ryinitiator.h
ryinitiator.o: ryinitiator.c
ryinitiator.o: ../../h/util.h
ryinitiator.o: ../../h/config.h
ryinitiator.o: ../../h/ll_log.h
ryinitiator.o: ../../h/qmgr.h
ryinitiator.o: ../../h/Qmgr-types.h
ryinitiator.o: ../../h/Qmgr-ops.h
ryinitiator.o: ./ryinitiator.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
