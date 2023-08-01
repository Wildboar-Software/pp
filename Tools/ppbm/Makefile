# pp benchamark Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ppbm/RCS/Makefile,v 6.0 1991/12/18 20:32:20 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:20  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ppbm.c
OBJS =  ppbm.o

LINT	= lint
LINTFLAGS = -bhuz

CC	= cc
LDFLAGS	=
CFLAGS	= -O

############################################################
#
# Building Rules
#
############################################################

all:            xppbm
lint:           l-ppbm

install:;

results: text ps

text: ppbm-raw.txt ppbm-sorted.txt

ps: ppbm-sorted.ps ppbm-raw.ps

ppbm-raw.txt: pp-benchmarks.tbl
	tbl $? | nroff -ms | colcrt > $@

ppbm-sorted.txt: pp-benchmarks.tbl pp-bench.awk
	pp-bench.awk pp-benchmarks.tbl | tbl | nroff -ms | colcrt > $@

ppbm-sorted.ps: pp-benchmarks.tbl pp-bench.awk
	pp-bench.awk pp-benchmarks.tbl | tbl | psroff -ms -t > $@

ppbm-raw.ps: pp-benchmarks.tbl
	tbl $? | psroff -ms -t > $@

ppbm: xppbp
xppbm: $(OBJS) $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ $(OBJS)

l-ppbm: $(SRCS)
		$(LINT) $(LINTFLAGS) $(SRCS)

shar: shar-src
shar-src: Makefile ppbm.c ppbm.8
	shar -cv Makefile ppbm.c ppbm.8 > $@

inst-ppbm:
	echo "ppbm not for installation"


clean:  tidy
		rm -f $(OBJS) *.txt *.ps x*
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
