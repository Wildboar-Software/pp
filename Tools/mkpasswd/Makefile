#  Generate encoded/encrypted passwds
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/mkpasswd/RCS/Makefile,v 6.0 1991/12/18 20:31:22 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:31:22  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  mkpasswd.c
OBJS =  mkpasswd.o
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

PROGS = xmkpasswd

default: $(PROGS)

mkpasswd: xmkpasswd
xmkpasswd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBSYS)


install: inst-dir inst-mkpasswd

inst-mkpasswd: $(TOOLDIR)/mkpasswd
$(TOOLDIR)/mkpasswd: xmkpasswd
	-$(BACKUP) $@ zxmkpasswd
	rm -f $@
	$(INSTALL) xmkpasswd $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "mkpasswd installed normally"; echo ""

inst-dir: $(TOOLDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxmkpasswd *.old *.BAK

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
