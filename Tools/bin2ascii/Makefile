# bin2ascii Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/bin2ascii/RCS/Makefile,v 6.0 1991/12/18 20:28:31 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:31  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  bin2ascii.c bin2ascii.c
OBJS =  bin2ascii.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
LDFLAGS		= $(LDOPTIONS) $(LLF)
CFLAGS          =  $(CCOPTIONS) $(LCF) -DDEBUG -I$(HEADERS)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS = xbin2ascii

default:        $(PROGS)
all:            default
lint:           l-bin2ascii

install:        inst-dir inst-bin2ascii
inst-dir:       $(TOOLDIR)


bin2ascii: xbin2ascii
xbin2ascii: bin2ascii.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ bin2ascii.o $(LIBPP) $(LIBSYS)

l-bin2ascii: bin2ascii.c
		$(LINT) $(LINTFLAGS) bin2ascii.c $(LINTLIBS)


inst-bin2ascii: $(TOOLDIR)/bin2ascii
$(TOOLDIR)/bin2ascii: xbin2ascii
		-$(BACKUP) $@ zxbin2ascii
		rm -f $@
		$(INSTALL) xbin2ascii $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "bin2ascii tool inst-talled normally"; echo ""


clean:  tidy
		rm -f $(OBJS)
tidy:
		rm -f core.* $(PROGS) zx* *.old *.BAK

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
bin2ascii.o: bin2ascii.c
bin2ascii.o: bin2ascii.c

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
