# Stats generation stuff
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/statgen/RCS/Makefile,v 6.0 1991/12/18 20:33:14 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:33:14  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= statp.c
OBJS	= statp.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a
LIBS	=	$(LIBPP) $(LIBSYS)

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################


default: xstatp line.awk

statp: xstatp
	@true
xstatp: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

install: inst-statp

inst-statp: $(TOOLDIR)/statp
$(TOOLDIR)/statp: xstatp
	-$(BACKUP) $@ zxstatp
	rm -f $@
	$(INSTALL) xstatp $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "statp tool installed normally"; echo ""

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old xstatp line.awk

lint: l-stat

l-stat: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS)

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

lex.o:  y.tab.h

line.awk: line.nawk
	sed -e '/^#BEGIN(nawk)/,/^#END(nawk)/d' \
		-e 's/nawk/awk/' \
		-e 's/bytes *(\([^)]*\))/\1/g' $? > $@ || rm $@
	chmod 755 $@

############################################################
#
# End of Building Rules
#
############################################################
