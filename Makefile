# Master makefile for the whole PP
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/RCS/Makefile,v 6.0 1991/12/18 20:02:50 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:02:50  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SUBDIRS =       Lib Src Chans Format Tools Uip examples
ALLDIRS	=	$(SUBDIRS) man doc
DIRS	=	$(PPDIR) \
		$(TXTDIR) $(PRIDIR) $(SPLDIR) $(MANDIR) $(USRBINDIR) \
		$(BINDIR) $(CMDDIR) $(TBLDIR) \
		$(LOGDIR) $(QUEDIR) \
		$(CHANDIR) $(FORMDIR) $(MACDIR) $(TOOLDIR) \
		$(MANDIR) $(MANDIR)/man1 $(MANDIR)/man3 \
		$(MANDIR)/man8


############################################################
#
# Building Rules
#
############################################################

default:
	for i in $(SUBDIRS); \
	do (echo "cd $$i; $(MAKE)"; cd $$i; $(MAKE)); done

install clean tidy all lint define depend:
	for i in $(ALLDIRS); \
	do (echo "cd $$i; $(MAKE) $@"; cd $$i; $(MAKE) $@); done

dirs:
	@for i in $(DIRS); \
	do if [ ! -d $$i ]; \
	   then set -x; mkdir -p $$i; \
		$(CHOWN) $(PPUSER) $$i; \
		case "$$i" in $(LOGDIR)) $(CHMOD) a=rwx $$i;; esac; \
	   fi; \
	done
		
# $(DIRS) junk:
# 	@-base=`expr $@ : '\(.*\)/[^/]*'`; \
# 		test -r $$base || echo "	++++ you must create $$base"
# 	mkdir $@
# 	$(CHOWN) $(PPUSER) $@
# 	case "$@" in $(LOGDIR)) $(CHMOD) a=rwx $@;; esac

distribution: README CHANGES clean
	cd doc;make clean

README: man/man8/pp-gen.8
	nroff -man $? > $@

CHANGES: pp-changes.ms
	nroff -ms $? > $@

image: distribution 		# with CARE 
	case `pwd` in */pp-beta) exit 1;; esac
	rm -f Make.defs* h/config.h
	rm -f Lib/version.local Lib/ppversion.c
	rm -f CHECK-OUT
	rm -rf test_suite

