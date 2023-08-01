#  Convert macro nsaps to full format
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/nsapexpand/RCS/Makefile,v 6.0 1991/12/18 20:32:03 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:03  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  nsapexpand.c
OBJS =  nsapexpand.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS		= $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xnsapexpand

default: $(PROGS)

nsapexpand: xnsapexpand
xnsapexpand: nsapexpand.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ nsapexpand.o $(LIBPP) $(LIBSYS)


install: inst-dir inst-nsapexpand

inst-nsapexpand: $(TOOLDIR)/nsapexpand
$(TOOLDIR)/nsapexpand: xnsapexpand
	-$(BACKUP) $@ zxnsapexpand
	rm -f $@
	$(INSTALL) xnsapexpand $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "nsapexpand installed normally"; echo ""

inst-dir: $(TOOLDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxnsapexpand *.old *.BAK

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile


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
