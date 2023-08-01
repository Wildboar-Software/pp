# ppls dummy Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ppls/RCS/Makefile,v 6.0 1991/12/18 20:32:36 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:36  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ppls.c
OBJS =  ppls.o
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

PROGS = xppls

default:        $(PROGS)
all:            default
lint:           l-ppls

install:        inst-dir inst-ppls
inst-dir:       $(TOOLDIR)


ppls: xppls
xppls: ppls.o $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ ppls.o $(LIBPP) $(LIBSYS)

l-ppls: ppls.c
		$(LINT) $(LINTFLAGS) ppls.c $(LINTLIBS)


inst-ppls:
	echo "ppls not for installation"


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

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
