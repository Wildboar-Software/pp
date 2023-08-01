# resend utility
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/resend/RCS/Makefile,v 6.0 1991/12/18 20:42:09 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:42:09  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= resend.c
OBJS	= resend.o

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

default: xresend

resend: xresend
	@true

xresend: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-resend

inst-resend: $(USRBINDIR)/resend
$(USRBINDIR)/resend: xresend
	-$(BACKUP) $@ zxresend
	rm -f $@
	$(INSTALL) xresend $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Resend installed normally"; echo ""

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old x* z*

lint: l-resend

l-resend: $(SRCS)
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
resend.o: resend.c
resend.o: ../../h/util.h
resend.o: ../../h/config.h
resend.o: ../../h/ll_log.h
resend.o: ../../h/prm.h
resend.o: ../../h/q.h
resend.o: ../../h/adr.h
resend.o: ../../h/list_rchan.h
resend.o: ../../h/chan.h
resend.o: ../../h/table.h
resend.o: ../../h/list_bpt.h
resend.o: ../../h/auth.h
resend.o: ../../h/list_bpt.h
resend.o: ../../h/extension.h
resend.o: ../../h/mta.h
resend.o: ../../h/adr.h
resend.o: ../../h/list_bpt.h
resend.o: ../../h/aparse.h
resend.o: ../../h/ap.h
resend.o: ../../h/util.h
resend.o: ../../h/or.h
resend.o: ../../h/chan.h
resend.o: ../../h/auth.h
resend.o: ../../h/list_rchan.h
resend.o: ../../h/mta.h
resend.o: ../../h/adr.h
resend.o: ../../h/retcode.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
