# PP specific routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/pp-rfc987/RCS/Makefile,v 6.0 1991/12/18 20:34:52 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:34:52  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =
OBJS =
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../../h
CFLAGS          = $(CCOPTIONS) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS)



############################################################
#
# Building Rules
#
############################################################


PROGS =

default:        all

progs:          ${PROGS}

all:            ${PROGS}

rfc2or:         awk.rfc2or
or2rfc:         awk.or2rfc

install:	;

clean:          tidy
		rm -f *.temp $(OBJS) $(PROGS)

tidy:
		rm -f *.old *.BAK *.OUT

depend: ;
define: ;


############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
