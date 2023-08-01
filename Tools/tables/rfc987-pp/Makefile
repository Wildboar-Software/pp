# PP specific routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/rfc987-pp/RCS/Makefile,v 6.0 1991/12/18 20:35:14 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:35:14  jpo
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

rfc987:         awk.rfc987

install:        inst-rfc987 
inst-rfc987:	$(TBLDIR)/awk.rfc987
$(TBLDIR)/awk.rfc987: awk.rfc987
		-$(BACKUP) $@ zawk.rfc987
		rm -f $@
		$(INSTALL) awk.rfc987 $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		@echo "awk.rfc987 script installed normally"; echo ""

clean:          tidy
#		-rm -f $(OBJS) $(PROGS)

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
