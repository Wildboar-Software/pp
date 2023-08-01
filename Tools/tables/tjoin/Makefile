# tjoin - table join    11/04/89 jct
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/tjoin/RCS/Makefile,v 6.0 1991/12/18 20:35:29 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:35:29  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  tjoin1.c tjoin2.c tjoin3.c lex.c tjoin.c
OBJS =  tjoin1.o tjoin2.o tjoin3.o lex.o tjoin.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../../h
INCLUDE         = -I$(HEADERS)
CFLAGS          = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS         = $(LDOPTIONS) $(LLF)
YFLAGS          = -d
LLFLAGS         = $(LINTFLAGS) $(INCLUDE)
LINTLIBS        = ../../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################


PROGS = xtjoin

default:        all
progs:          ${PROGS}
all:            ${PROGS}

tjoin:          xtjoin
xtjoin:         $(OBJS)
		$(CC) $(LDFLAGS) $(OBJS) -ll -o $@ $(LIBSYS)

lex.o:          tjoin.c y.tab.h

y.tab.h: tjoin.y;

lint: l-tjoin
l-tjoin: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

install:        inst-dir inst-tjoin
inst-dir:       $(TOOLDIR)
inst-tjoin:     $(TOOLDIR)/tjoin
$(TOOLDIR)/tjoin: xtjoin
		-$(BACKUP) $@ zxtjoin
		rm -f $@
		$(INSTALL) xtjoin $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "tjoin tool installed normally"; echo ""


clean:          tidy
		rm -f $(OBJS) lex.c tjoin.c y.tab.h
tidy:
		rm -f core *.old *.BAK *.OUT $(PROGS) zx*

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
tjoin1.o: tjoin1.c
tjoin1.o: ../../../h/tjoin.h
tjoin2.o: tjoin2.c
tjoin2.o: ../../../h/tjoin.h
tjoin3.o: tjoin3.c
tjoin3.o: ../../../h/tjoin.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
