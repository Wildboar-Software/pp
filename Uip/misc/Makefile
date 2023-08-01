# misc utilities
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/misc/RCS/Makefile,v 6.0 1991/12/18 20:39:34 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:39:34  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= strip_addr.c flock.c X400_addr.c
OBJS	= 

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

default: xstrip_addr xflock xX400_addr

strip_addr: xstrip_addr
	@true

xstrip_addr: strip_addr.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ strip_addr.o $(LIBPP) $(LIBSYS)

install: inst-strip_addr

inst-strip_addr: $(USRBINDIR)/strip_addr
$(USRBINDIR)/strip_addr: xstrip_addr
	-$(BACKUP) $@ zxstrip_addr
	rm -f $@
	$(INSTALL) xstrip_addr $@
	-@$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Strip_Addr installed normally"; echo ""

flock: xflock
	@true

xflock: flock.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ flock.o $(LIBPP) $(LIBSYS)

install: inst-flock

inst-flock: $(USRBINDIR)/flock
$(USRBINDIR)/flock: xflock
	-$(BACKUP) $@ zxflock
	rm -f $@
	$(INSTALL) xflock $@
	-@$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Flock installed normally"; echo ""

X400_addr: xX400_addr
	@true

xX400_addr: X400_addr.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ X400_addr.o $(LIBPP) $(LIBSYS)

install: inst-X400_addr

inst-X400_addr: $(USRBINDIR)/X400_addr
$(USRBINDIR)/X400_addr: xX400_addr
	-$(BACKUP) $@ zxX400_addr
	rm -f $@
	$(INSTALL) xX400_addr $@
	-@$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X400_addr installed normally"; echo ""

clean: tidy
tidy:
	rm -f *.o core a.out Makefile.old x* z*

lint: l-strip_addr l-flock

l-strip_addr: strip_addr.c
	$(LINT) $(LLFLAGS) strip_addr.c $(LINTLIBS)


l-flock: flock.c
	$(LINT) $(LLFLAGS) flock.c $(LINTLIBS)

l-X400_addr: X400_addr.c
	$(LINT) $(LLFLAGS) X400_addr.c $(LINTLIBS)

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
strip_addr.o: strip_addr.c
strip_addr.o: ../../h/util.h
strip_addr.o: ../../h/config.h
strip_addr.o: ../../h/ll_log.h
strip_addr.o: ../../h/ap.h
strip_addr.o: ../../h/util.h
flock.o: flock.c
flock.o: ../../h/util.h
flock.o: ../../h/config.h
flock.o: ../../h/ll_log.h
X400_addr.o: X400_addr.c
X400_addr.o: ../../h/util.h
X400_addr.o: ../../h/config.h
X400_addr.o: ../../h/ll_log.h
X400_addr.o: ../../h/adr.h
X400_addr.o: ../../h/list_rchan.h
X400_addr.o: ../../h/chan.h
X400_addr.o: ../../h/table.h
X400_addr.o: ../../h/list_bpt.h
X400_addr.o: ../../h/auth.h
X400_addr.o: ../../h/list_bpt.h
X400_addr.o: ../../h/extension.h
X400_addr.o: ../../h/mta.h
X400_addr.o: ../../h/adr.h
X400_addr.o: ../../h/list_bpt.h
X400_addr.o: ../../h/aparse.h
X400_addr.o: ../../h/ap.h
X400_addr.o: ../../h/util.h
X400_addr.o: ../../h/or.h
X400_addr.o: ../../h/chan.h
X400_addr.o: ../../h/auth.h
X400_addr.o: ../../h/list_rchan.h
X400_addr.o: ../../h/retcode.h
X400_addr.o: ../../h/list_rchan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
