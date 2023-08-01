#  Generate distribution list file from sendmail aliases
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/make-lists/RCS/Makefile,v 6.0 1991/12/18 20:30:32 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:30:32  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  make-lists.c
OBJS =  make-lists.o
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

PROGS = xmake-lists

default: $(PROGS)

make-lists: xmake-lists
xmake-lists: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBSYS)


install: inst-dir inst-make-lists

inst-make-lists: $(TOOLDIR)/make-lists
$(TOOLDIR)/make-lists: xmake-lists
	-$(BACKUP) $@ zxmake-lists
	rm -f $@
	$(INSTALL) xmake-lists $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "make-lists installed normally"; echo ""

inst-dir: $(TOOLDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxmake-lists *.old *.BAK

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
