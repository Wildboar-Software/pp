# Rcvtrip program
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/rcvtrip/RCS/Makefile,v 6.0 1991/12/18 20:42:02 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:42:02  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= rcvtrip.c
OBJS	= rcvtrip.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: xrcvtrip

install: inst-rcvtrip

inst-rcvtrip: $(USRBINDIR)/rcvtrip

$(USRBINDIR)/rcvtrip: xrcvtrip
	-$(BACKUP) $@ zrcvtrip
	rm -f $@
	$(INSTALL) xrcvtrip $@
	-$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "rcvtrip program installed normally"; echo ""

rcvtrip: xrcvtrip
	@true
xrcvtrip: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old xrcvtrip

lint: l-rcvtrip

l-rcvtrip: $(SRCS)
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
rcvtrip.o: rcvtrip.c
rcvtrip.o: ../../h/util.h
rcvtrip.o: ../../h/config.h
rcvtrip.o: ../../h/ll_log.h
rcvtrip.o: ../../h/retcode.h
rcvtrip.o: ../../h/adr.h
rcvtrip.o: ../../h/list_rchan.h
rcvtrip.o: ../../h/chan.h
rcvtrip.o: ../../h/table.h
rcvtrip.o: ../../h/list_bpt.h
rcvtrip.o: ../../h/auth.h
rcvtrip.o: ../../h/list_bpt.h
rcvtrip.o: ../../h/extension.h
rcvtrip.o: ../../h/mta.h
rcvtrip.o: ../../h/adr.h
rcvtrip.o: ../../h/list_bpt.h
rcvtrip.o: ../../h/aparse.h
rcvtrip.o: ../../h/ap.h
rcvtrip.o: ../../h/util.h
rcvtrip.o: ../../h/or.h
rcvtrip.o: ../../h/chan.h
rcvtrip.o: ../../h/auth.h
rcvtrip.o: ../../h/list_rchan.h
rcvtrip.o: ../../h/ap.h
rcvtrip.o: ../../h/chan.h
rcvtrip.o: ../../h/alias.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
