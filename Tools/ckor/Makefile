# OR address mappings
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ckor/RCS/Makefile,v 6.0 1991/12/18 20:29:26 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:29:26  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  or2rfc.c rfc2or.c
OBJS =  or2rfc.o rfc2or.o
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

PROGS = xor2rfc xrfc2or

default:                $(PROGS)
all:                    default
lint:                   l-rfc2or l-or2rfc

install:                inst-dir inst-rfc2or inst-or2rfc
inst-dir:               $(TOOLDIR)


or2rfc:                 xor2rfc
xor2rfc:                or2rfc.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ or2rfc.o $(LIBPP) $(LIBSYS)
l-or2rfc:               or2rfc.c
			$(LINT) $(LINTFLAGS) or2rfc.c $(LINTLIBS)
inst-or2rfc:            $(TOOLDIR)/or2rfc
$(TOOLDIR)/or2rfc:      xor2rfc
			-$(BACKUP) $@ zxor2rfc
			rm -f $@
			$(INSTALL) xor2rfc $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "or2rfc tool inst-talled normally"; echo ""


rfc2or:                 xrfc2or
xrfc2or:                rfc2or.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ rfc2or.o $(LIBPP) $(LIBSYS)
l-rfc2or:               rfc2or.c
			$(LINT) $(LINTFLAGS) rfc2or.c $(LINTLIBS)
inst-rfc2or:            $(TOOLDIR)/rfc2or
$(TOOLDIR)/rfc2or:      xrfc2or
			-$(BACKUP) $@ zxrfc2or
			rm -f $@
			$(INSTALL) xrfc2or $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "rfc2or tool inst-talled normally"; echo ""




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
or2rfc.o: or2rfc.c
or2rfc.o: ../../h/head.h
or2rfc.o: ../../h/util.h
or2rfc.o: ../../h/config.h
or2rfc.o: ../../h/ll_log.h
or2rfc.o: ../../h/retcode.h
or2rfc.o: ../../h/or.h
rfc2or.o: rfc2or.c
rfc2or.o: ../../h/head.h
rfc2or.o: ../../h/util.h
rfc2or.o: ../../h/config.h
rfc2or.o: ../../h/ll_log.h
rfc2or.o: ../../h/retcode.h
rfc2or.o: ../../h/or.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
