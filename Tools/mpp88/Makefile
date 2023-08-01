# mpp88
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/mpp88/RCS/Makefile,v 6.0 1991/12/18 20:31:43 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:31:43  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



LIBPP	= ../../Lib/libpp.a

SOCKET =
HEADERS =	../../h
CFLAGS = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)

############################################################
#
# Suffix rules
#
############################################################

PROGS		= xmpp88


##################################################################
# mpp88
##################################################################


SRCS =  mpp88.c
OBJS =  mpp88.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

all:	xmpp88

mpp88:    xmpp88
	@true

xmpp88:   $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)


install: inst-dir inst-mpp88
inst-dir: $(TOOLDIR)
inst-mpp88: $(TOOLDIR)/mpp88
$(TOOLDIR)/mpp88: xmpp88
	-$(BACKUP) $@ zxmpp88
	rm -f $@
	$(INSTALL) xmpp88 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "mpp88 tool installed normally"; echo ""

mpp88.c:mpp88.py
		$(PEPY) $(PYFLAGS) -o $@ mpp88.py



################################################################
# clean
################################################################

clean:  tidy
	rm -f $(SRCS) $(OBJS) *.ph

tidy:
	rm -f *.a  _* core xmpp88 *.BAK *.old zx*

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
mpp88.o: mpp88.c
mpp88.o: mpp88.c

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
