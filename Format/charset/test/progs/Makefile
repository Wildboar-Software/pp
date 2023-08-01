# Makefile for the bpfilt test progs 
#
############################################################
#
# @(#) $Header$
#
# $Log$
#
############################################################
#
# Definitions
#
############################################################

SRCS =  bit2ch.c ch255.c ch2val.c translate.c
OBJS =  bit2ch.o ch255.o ch2val.o translate.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS         = ../../../../h
LIBPP		=

CFLAGS          = $(CCOPTIONS) -I$(HEADERS) $(LCF) -D
LDFLAGS         = $(LDOPTIONS) $(LLF)

LLFLAGS         = $(LINTFLAGS)


############################################################
#
# Building Rules
#
############################################################

PROGS  = ch255 bit2ch ch2val translate

default: $(OBJS) $(PROGS)
all:	$(PROGS)


bit2ch:	bit2ch.o
	$(CC) $(LDFLAGS) bit2ch.o -o $@

ch2val:	ch2val.o
	$(CC) $(LDFLAGS) ch2val.o -o $@

ch255:	ch255.o
	$(CC) $(LDFLAGS) ch255.o -o $@


translate: translate.o
	$(CC) $(LDFLAGS) translate.o -o $@


install: ; 

clean: tidy
	rm -f $(OBJS) $(PROGS)

tidy:
	rm -rf core a.out *.old *.BAK 

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
bit2ch.o: bit2ch.c
ch255.o: ch255.c
ch2val.o: ch2val.c
translate.o: translate.c

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
