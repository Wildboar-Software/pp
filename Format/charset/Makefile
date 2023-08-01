# charset  for Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/charset/RCS/Makefile,v 6.0 1991/12/18 20:19:34 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:19:34  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  conv.c
OBJS =  conv.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS         = ../../h 
LIBPP		= ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS)




############################################################
#
# Building Rules
#
############################################################

PROGS  = xcharset 

default: $(OBJS) $(PROGS)
	-rm -rf README copyright design 
	-ln ../../Lib/charset/copyright 
	-ln ../../Lib/charset/design
	-ln ../../Lib/charset/README


all:	$(PROGS)


charset: xcharset 
xcharset: conv.o 
	$(CC) $(CCFLAGS) conv.o $(LIBPP) -o $@


install: inst-dir inst-charset 
inst-dir: $(FORMDIR)

inst-charset : $(FORMDIR)/charset
$(FORMDIR)/charset : xcharset
		-$(BACKUP) $@ zxcharset 
		rm -f $@           
		$(INSTALL) xcharset  $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "Character Set filter installed normally"; echo ""

clean: tidy
	rm -f $(OBJS) xcharset $(CSDIR)
tidy:
	rm -rf core a.out *.old *.BAK zxcharset  $(PROGS)

lint: l-charset
l-charset : conv.c
	$(LINT) $(LLFLAGS) conv.c

depend:
	$(DEPEND) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
conv.o: conv.c
conv.o: ../../h/charset.h
conv.o: conv.c
conv.o: ../../h/charset.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
