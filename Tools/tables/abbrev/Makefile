# abbrev - extend domain table to include abbreviations
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/abbrev/RCS/Makefile,v 6.0 1991/12/18 20:33:28 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:33:28  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  abbrev.c
OBJS =  abbrev.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../../h
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS		= $(LINTFLAGS) -I$(HEADERS)
LINTLIBS	= 

############################################################
#
# Building Rules
#
############################################################


PROGS = xabbrev

default:        all
progs:          $(PROGS)
all:            $(PROGS)
abbrev:         $(PROGS)

xabbrev:        abbrev.o
		$(CC) $(LDFLAGS) abbrev.o -o $@ $(LIBSYS)


install:        inst-dir inst-abbrev
inst-dir:       $(TOOLDIR)
inst-abbrev:    $(TOOLDIR)/abbrev
$(TOOLDIR)/abbrev: xabbrev
		-$(BACKUP) $@ zxabbrev
		rm -f $@
		$(INSTALL) xabbrev $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "abbrev tool installed normally"; echo ""

clean:          tidy
		rm -f $(OBJS)

tidy:
		rm -f *.old *.BAK *.OUT $(PROGS) zx*

depend:
		$(DEPEND) -I$(HEADERS) $(SRCS)

define:
		$(DEFINE) Makefile

lint: l-abbrev
l-abbrev: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)


############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
abbrev.o: abbrev.c
abbrev.o: abbrev.c

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
