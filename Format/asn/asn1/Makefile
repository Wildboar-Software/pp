# asn1 - Decoder/Encoder Routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/asn/asn1/RCS/Makefile,v 6.0 1991/12/18 20:16:32 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:16:32  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ExtDef-print.c ExtDef.c GenTxt-print.c GenTxt.c
OBJS =  ExtDef-print.o ExtDef.o GenTxt-print.o GenTxt.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../../h
#LIBPP           = ../../Lib/libpp.a

CFLAGS          = $(CCOPTIONS) -g $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) -g $(LLF)

LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)
SHADOW		= asn1-made


############################################################
#
# Suffix rules
#
############################################################




############################################################
#
# Building Rules
#
############################################################


default: ${SRCS} ${SHADOW}

${SHADOW}: $(OBJS)
	$(AR) r$(ARFLAGS) libasn1.a $?
	$(RANLIB) libasn1.a
	touch $@

GenTxt.c GenTxt-types.h: GenTxt.py
	$(PEPY) -a PY_advise GenTxt.py

GenTxt-print.c: gentxt.py
	$(PEPY) -a PY_advise -S PRINT -o $@ gentxt.py

GenTxt.py:
	$(POSY) $(POFLAGS) -f -h -o GenTxt.py gentxt.py

ExtDef.c ExtDef-types.h: ExtDef.py
	$(PEPY) -a PY_advise ExtDef.py

ExtDef-print.c: extdefbp.py
	$(PEPY) -a PY_advise -S PRINT -o $@ extdefbp.py

ExtDef.py:
	$(POSY) $(POFLAGS) -f -h -o ExtDef.py extdefbp.py
	
clean: tidy
	rm -f [A-Z]*.[ch] [A-Z]*.py *.ph $(OBJS) *.a

tidy:
	rm -f core a.out *.old *.BAK ${SHADOW} *.ph


lint: l-asn1

l-asn1: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
ExtDef-print.o: ExtDef-print.c
ExtDef.o: ExtDef.c
ExtDef.o: ./ExtDef-types.h
GenTxt-print.o: GenTxt-print.c
GenTxt.o: GenTxt.c
GenTxt.o: ./GenTxt-types.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
