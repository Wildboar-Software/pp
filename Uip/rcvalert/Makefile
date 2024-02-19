# Rcvalert program
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/rcvalert/RCS/Makefile,v 6.0 1991/12/18 20:39:41 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:39:41  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)
SRCS	= flagmail.c parsesbr.c ttyalert.c \
	ttybiff.c ttysbr.c udpsbr.c xalert.c

############################################################
#
# Building Rules
#
############################################################

default: xalert-$(X11) flagmail # ttyalert ttybiff

install: inst-flagmail inst-xalert-$(X11) # inst-ttyalert ttybiff

############################################################
#
# Xclient
#
############################################################

FLAGSRC	= flagmail.c ttysbr.c parsesbr.c
FLAGOBJ	= flagmail.o ttysbr.o parsesbr.o

inst-flagmail: $(USRBINDIR)/flagmail

$(USRBINDIR)/flagmail: xflagmail
	-$(BACKUP) $@ zflagmail
	rm -f $@
	$(INSTALL) xflagmail $@
	-$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X alert client program install normally"; echo ""

flagmail: xflagmail
	@true

xflagmail: $(FLAGOBJ) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(FLAGOBJ) $(LIBPP) $(LIBSYS)

l-flagmail: flagmail.c
	$(LINT) $(LLFLAGS) $(FLAGSRC) $(LINTLIBS)

############################################################
#
# ttyalert
#
############################################################

TTYSRC	= ttyalert.c ttysbr.c parsesbr.c
TTYOBJ	= ttyalert.o ttysbr.o parsesbr.o

inst-ttyalert: $(USRBINDIR)/ttyalert

$(USRBINDIR)/ttyalert: xttyalert
	-$(BACKUP) $@ zttyalert
	rm -f $@
	$(INSTALL) xttyalert $@
	-$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "tty alert program installed normally"; echo ""

ttyalert: xttyalert
xttyalert: $(TTYOBJ)
	$(CC) $(LDFLAGS) -o $@ $(TTYOBJ) $(LIBPP) $(LIBSYS)

l-ttyalert: $(TTYSRC)
	$(LINT) $(LLFLAGS) $(TTYSRC) $(LINTLIBS)

############################################################
#
# xalert
#
############################################################

XALERTSRC	= dirsbr.c xalert.c ttysbr.c udpsbr.c
XALERTOBJ	= dirsbr.o xalert.o ttysbr.o udpsbr.o
# uncomment the next line to switch off photo lookup code.
#NOPHOTO = -DNOPHOTOS

inst-xalert-true: inst-xalert inst-xalert-appdef
	@true

inst-xalert- inst-xalert-false:
	@echo "X11 not defined so not installing xalert"

inst-xalert: $(USRBINDIR)/xalert
$(USRBINDIR)/xalert: xxalert
	-$(BACKUP) $@ zxalert
	rm -f $@
	$(INSTALL) xxalert $@
	$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X photo alert program installed normally"

inst-xalert-appdef: $(APPDEFAULTS)/XAlert
$(APPDEFAULTS)/XAlert: XAlert.ad
	-$(BACKUP) $@ zXAlert.ad
	rm -rf $@
	$(INSTALL) XAlert.ad $@
	-$(CHMOD) 444 $@
	-$(CHOWN) $(ROOTUSER) $@
	-@ls -ls $@
	-@echo "XAlert application defaults installed normally"; echo ""

xalert-true: xalert
	@true
xalert-false xalert-:; @echo "X11 not defined so not builing xalert"

xalert:xxalert
	@true
xxalert: $(XALERTOBJ)
	$(CC) $(LDFLAGS) -o $@ $(XALERTOBJ) $(LIBPHOTO) $(LIBPP) $(LIBSYS) $(LIBX)

xalert.o: xalert.c
	$(CC) $(CFLAGS) -c $(NOPHOTO) xalert.c

l-xalert: $(XALERTSRC)
	$(LINT) $(LLFLAGS) $(XALERTSRC)  $(LINTLIBS) \
		/usr/local/lib/lint/llib-ldsap.ln \
		/usr/local/lib/lint/llib-lisode.ln

saber_xalert: $(XALERTSRC)
	#load -C $(CFLAGS) $(XALERTSRC) $(LIBPHOTO) $(LIBSYS)

############################################################
#
# TtyBiff
#
############################################################

TTYBSRC	= ttybiff.c ttysbr.c udpsbr.c
TTYBOBJ	= ttybiff.o ttysbr.o udpsbr.o

inst-ttybiff: $(USRBINDIR)/ttybiff

$(USRBINDIR)/ttybiff: xttybiff
	-$(BACKUP) $@ zttybiff
	rm -f $@
	$(INSTALL) xttybiff $@
	-$(CHMOD) $(PGMPROT) $@
	-@$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "tty alert program installed normally"; echo ""

ttybiff: xttybiff
xttybiff: $(TTYBOBJ)
	$(CC) $(LDFLAGS) -o $@ $(TTYBOBJ) $(LIBSYS)

l-ttybiff: $(TTYSRC)
	$(LINT) $(LLFLAGS) $(TTYBSRC) $(LINTLIBS)

############################################################
#
# Misc stuff
#
############################################################

clean: tidy
tidy:
	rm -f *.o core a.out Makefile.old z* \
		xxalert xttyalert xflagmail xttybiff 

lint: l-xalert l-flagmail l-ttyalert



depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
flagmail.o: flagmail.c
flagmail.o: ./data.h
parsesbr.o: parsesbr.c
parsesbr.o: ../../h/util.h
parsesbr.o: ../../h/config.h
parsesbr.o: ../../h/ll_log.h
ttyalert.o: ttyalert.c
ttyalert.o: ./data.h
ttybiff.o: ttybiff.c
ttybiff.o: ./data.h
ttysbr.o: ttysbr.c
ttysbr.o: ../../h/config.h
udpsbr.o: udpsbr.c
udpsbr.o: ./data.h
xalert.o: xalert.c
xalert.o: ./data.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
