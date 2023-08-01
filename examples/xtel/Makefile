# Make new xtel example
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

TABLES = aliases auth.user auth.channel auth.mta \
	auth.qmgr ch.fax ch.list ch.local ch.mslocal \
	ch.shell ch.uucp ch.x400in84 ch.x400out84 \
	ch.x400in88 ch.x400out88 channel domain or or2rfc \
	rfc1148gate rfc2or users

WARNINGS = warning.1 warning.2
WARNDIR	= $(TBLDIR)/warnings

############################################################
#
# Building Rules
#
############################################################

all default: import

import: tailor Make.defs config.h tables warnings

tailor: $(TAILOR)
	cp $? $@

Make.defs: ../../Make.defs
	cp $? $@

config.h: ../../h/config.h
	cp $? $@

tables: ;
	@for i in $(TABLES); \
	do make cp-table THETABLE=$$i; \
	done

cp-table: $(THETABLE)
$(THETABLE): $(TBLDIR)/$(THETABLE)
	head -500 $? > $@

warnings:;
	@for i in $(WARNINGS); \
	do make cp-warn WARNMSG=$$i; \
	done

cp-warn: $(WARNMSG)
$(WARNMSG): $(WARNDIR)/$(WARNMSG)
	cp $? $@
	
install:;

clean: tidy
	rm -f Make.defs tailor config.h $(TABLES)
tidy:
	rm -f $(OBJS) core a.out Makefile.old

lint: ;

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
