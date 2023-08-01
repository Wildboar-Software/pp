# Sendmail Fake 
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/sendmail/RCS/Makefile,v 6.0 1991/12/18 20:42:15 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:42:15  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= sendmail.c
OBJS	= sendmail.o

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

default: xsendmail

sendmail: xsendmail
	@true

xsendmail: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-sendmail

inst-sendmail: $(CMDDIR)/sendmail
$(CMDDIR)/sendmail: xsendmail
	-$(BACKUP) $@ zxsendmail
	rm -f $@
	$(INSTALL) xsendmail $@
	-@$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Sendmail fake installed normally"; echo ""

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old x* z*

lint: l-sendmail

l-sendmail: $(SRCS)
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
sendmail.o: sendmail.c
sendmail.o: ../../h/util.h
sendmail.o: ../../h/config.h
sendmail.o: ../../h/ll_log.h
sendmail.o: ../../h/prm.h
sendmail.o: ../../h/q.h
sendmail.o: ../../h/adr.h
sendmail.o: ../../h/list_rchan.h
sendmail.o: ../../h/chan.h
sendmail.o: ../../h/table.h
sendmail.o: ../../h/list_bpt.h
sendmail.o: ../../h/auth.h
sendmail.o: ../../h/list_bpt.h
sendmail.o: ../../h/extension.h
sendmail.o: ../../h/mta.h
sendmail.o: ../../h/adr.h
sendmail.o: ../../h/list_bpt.h
sendmail.o: ../../h/aparse.h
sendmail.o: ../../h/ap.h
sendmail.o: ../../h/util.h
sendmail.o: ../../h/or.h
sendmail.o: ../../h/chan.h
sendmail.o: ../../h/auth.h
sendmail.o: ../../h/list_rchan.h
sendmail.o: ../../h/mta.h
sendmail.o: ../../h/adr.h
sendmail.o: ../../h/retcode.h
sendmail.o: ../../h/ap.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
