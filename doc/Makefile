# Document construction
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



SUBDIRS	= manual
ALLDIRS = $(SUBDIRS) ifip6.5-88 tailor funet91 nordunet sun-89 \
		uknet90 unix-niftp


############################################################
#
# Building Rules
#
############################################################

default: 
	@for i in $(SUBDIRS); \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE) );\
	done

install:
	@for i in $(SUBDIRS); \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@);\
	done


clean tidy:
	@for i in $(ALLDIRS); \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@);\
	done
lint:;

depend:;

############################################################
#
# End of Building Rules
#
############################################################
