# Fax Channel Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/fax/RCS/Makefile,v 6.0 1991/12/18 20:07:09 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:07:09  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

PPTREE = ../..

PPOUTSRCS = fax_out.c
PPOUTOBJS = fax_out.o

PPINSRCS = fax_in.c
PPINOBJS = fax_in.o

SRCS	= $(PPOUTSRCS) $(PPINSRCS)
OBJS	= $(PPOUTOBJS) $(PPINOBJS)

ALLDRIVERS = dexNet200 ps250

HEADERS	=	$(PPTREE)/h
LIBPP	=	$(PPTREE)/Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS) $(PEPSYHEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = $(PPTREE)/Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################


target:	$(OBJS)
	@sh -c "if [ -z \"$(FAXDRIVERS)\" ];\
	then echo \"No FAXDRIVERS variable defined - must be in Make.defs\";\
		 exit 1;\
	fi"
	@for i in $(FAXDRIVERS) ; \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE));\
	done

install: $(OBJS)
	@sh -c "if [ -z \"$(FAXDRIVERS)\" ];\
	then echo \"No FAXDRIVERS defined\"; exit 1;\
	fi"
	@for i in ${FAXDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

clean: tidy
	@for i in ${ALLDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

tidy:
	rm -f $(OBJS) 
	@for i in ${ALLDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)
	@for i in ${ALLDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

define:
	@for i in ${ALLDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

lint: l-out l-in l-subs

l-out:
	$(LINT) $(LLFLAGS) $(PPOUTSRCS) $(LINTLIBS)

l-in:
	$(LINT) $(LLFLAGS) $(PPINSRCS) $(LINTLIBS)

l-subs:
	@for i in ${ALLDRIVERS} ; \
	do (echo "cd $$i; $(MAKE) lint"; cd $$i; $(MAKE) lint); done

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
