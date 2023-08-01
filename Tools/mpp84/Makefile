# mpp84
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/mpp84/RCS/Makefile,v 6.0 1991/12/18 20:31:34 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:31:34  jpo
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

CFLAGS = $(CCOPTIONS) $(LCF)
LDFLAGS	= $(LDOPTIONS) $(LLF)

############################################################
#
# Suffix rules
#
############################################################

PROGS   = xmpp84

.SUFFIXES: .py

.py.c:
	$(PEPY) $(PYFLAGS) $<

.py.o:
	$(PEPY) $(PYFLAGS) $<
	$(CC) $(CFLAGS) -c $*.c
	rm -f $*.c

##################################################################
# mpp84
##################################################################

OBJS    = UNIV.o T73.o SFD.o P1.o P3.o P2.o mpp84.o 
SRCS    = P1.c P2.c P3.c SFD.c T73.c UNIV.c mpp84.c

all: $(PROGS)


mpp84:    xmpp84
xmpp84:   $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-dir inst-mpp84
inst-dir: $(TOOLDIR)
inst-mpp84: $(TOOLDIR)/mpp84
$(TOOLDIR)/mpp84: xmpp84
	-$(BACKUP) $@ zxmpp84
	rm -f $@
	$(INSTALL) xmpp84 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "mpp84 tool installed normally"; echo ""



cfiles:         $(SRCS)

UNIV.o:         UNIV.c
UNIV.c:         UNIV.py

mpp84.o:          mpp84.c
mpp84.c:          mpp84.py

P1.o:           P1.c
P1.c:           P1.py

P2.o:           P2.c
P2.c:           P2.py

P3.o:           P3.c
P3.c:           P3.py

SFD.o:          SFD.c
SFD.c:          SFD.py

T73.o:          T73.c
T73.c:          T73.py


################################################################
# clean
################################################################

clean:  tidy
	rm -f $(SRCS) $(OBJS) *.ph

tidy:
	rm -f *.a  _* core xmpp84 *.BAK *.old zx*

depend: ;
define: ;


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE
# Dependencies follow

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
