# Documentation building Makefile
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



VOLS = volume0 volume1 volume2 volume3 
INPUTS = texinputs

default: all

all:
		@for x in $(VOLS) ; \
		do (echo "cd $$x;$(MAKE)"; cd $$x ; $(MAKE)); done

ps: true
		@for x in $(VOLS); \
		do (echo "cd $$x; $(MAKE) $@"; cd $$x; $(MAKE) $@); done

clean tidy:
		@for x in $(VOLS) ; \
		do (echo "cd $$x; $(MAKE) $@"; cd $$x ; $(MAKE) $@ ); done
		rm -f core *.old *.BAK

define depend lint install:;

true:
