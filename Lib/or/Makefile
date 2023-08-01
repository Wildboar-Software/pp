#  Makefile for the O/R-name library routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/or/RCS/Makefile,v 6.0 1991/12/18 20:23:08 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:23:08  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  or2pe.c or_add.c or_asc2ps.c or_basic.c \
           or_blank.c or_buildpn.c or_check.c or_default.c or_dmn2or.c \
           or_getpn.c or_init.c or_misc.c or_or2dmn.c or_or2rbits.c \
           or_or2rfc.c or_or2std.c or_ps2asc.c or_rbits2or.c or_rfc2or.c \
           or_search.c or_std2or.c or_tables.c or_util.c \
	   orn-dec.c orn-enc.c orn-util.c pe2or.c or_downgrade.c \
	   or_up_bnds.c or_valid.c
OBJS =  or2pe.o or_add.o or_asc2ps.o or_basic.o \
           or_blank.o or_buildpn.o or_check.o or_default.o or_dmn2or.o \
           or_getpn.o or_init.o or_misc.o or_or2dmn.o or_or2rbits.o \
           or_or2rfc.o or_or2std.o or_ps2asc.o or_rbits2or.o or_rfc2or.o \
           or_search.o or_std2or.o or_tables.o or_util.o \
	   orn-dec.o orn-enc.o orn-util.o pe2or.o or_downgrade.o \
	   or_up_bnds.o or_valid.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h

CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = or-made
LIBPP		= ../libpp.a

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../tai/llib-ltai.ln \
		  ../util/llib-lutil.ln \
		  ../x400/llib-lx400.ln 


############################################################
#
# Building Rules
#
############################################################

.SUFFIXES: .py

.py.c:
	$(PEPY) $(PYFLAGS) -o $@ $*.py

default: $(SHADOW)



$(SHADOW): $(OBJS)
	$(AR) r$(ARFLAGS) $(LIBPP) $?
	touch $@

lintlib:llib-lor.ln
llib-lor.ln: $(SRCS)
	$(LINT) -Cor $(LLFLAGS) $(SRCS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

install:;

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

clean:  tidy
	rm -f $(OBJS) or2pe.c pe2or.c llib-lor.ln *.ph ORN*

tidy:
	rm -f core $(SHADOW) $(PROGS) *.old *.BAK

depend: $(SRCS)
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

############################################################
#
# End of building rules
#
############################################################
#

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
or2pe.o: or2pe.c
or2pe.o: ../../h/util.h
or2pe.o: ../../h/config.h
or2pe.o: ../../h/ll_log.h
or2pe.o: ../../h/or.h
or_add.o: or_add.c
or_add.o: ../../h/util.h
or_add.o: ../../h/config.h
or_add.o: ../../h/ll_log.h
or_add.o: ../../h/or.h
or_asc2ps.o: or_asc2ps.c
or_asc2ps.o: ../../h/util.h
or_asc2ps.o: ../../h/config.h
or_asc2ps.o: ../../h/ll_log.h
or_asc2ps.o: ../../h/or.h
or_basic.o: or_basic.c
or_basic.o: ../../h/or.h
or_basic.o: ../../h/util.h
or_basic.o: ../../h/config.h
or_basic.o: ../../h/ll_log.h
or_blank.o: or_blank.c
or_blank.o: ../../h/or.h
or_blank.o: ../../h/util.h
or_blank.o: ../../h/config.h
or_blank.o: ../../h/ll_log.h
or_buildpn.o: or_buildpn.c
or_buildpn.o: ../../h/or.h
or_buildpn.o: ../../h/util.h
or_buildpn.o: ../../h/config.h
or_buildpn.o: ../../h/ll_log.h
or_check.o: or_check.c
or_check.o: ../../h/util.h
or_check.o: ../../h/config.h
or_check.o: ../../h/ll_log.h
or_check.o: ../../h/or.h
or_check.o: ../../h/table.h
or_default.o: or_default.c
or_default.o: ../../h/or.h
or_default.o: ../../h/util.h
or_default.o: ../../h/config.h
or_default.o: ../../h/ll_log.h
or_dmn2or.o: or_dmn2or.c
or_dmn2or.o: ../../h/or.h
or_dmn2or.o: ../../h/util.h
or_dmn2or.o: ../../h/config.h
or_dmn2or.o: ../../h/ll_log.h
or_getpn.o: or_getpn.c
or_getpn.o: ../../h/or.h
or_getpn.o: ../../h/util.h
or_getpn.o: ../../h/config.h
or_getpn.o: ../../h/ll_log.h
or_init.o: or_init.c
or_init.o: ../../h/head.h
or_init.o: ../../h/util.h
or_init.o: ../../h/config.h
or_init.o: ../../h/ll_log.h
or_init.o: ../../h/retcode.h
or_init.o: ../../h/mta.h
or_init.o: ../../h/adr.h
or_init.o: ../../h/list_rchan.h
or_init.o: ../../h/chan.h
or_init.o: ../../h/table.h
or_init.o: ../../h/list_bpt.h
or_init.o: ../../h/auth.h
or_init.o: ../../h/list_bpt.h
or_init.o: ../../h/extension.h
or_init.o: ../../h/mta.h
or_init.o: ../../h/aparse.h
or_init.o: ../../h/ap.h
or_init.o: ../../h/util.h
or_init.o: ../../h/or.h
or_init.o: ../../h/chan.h
or_init.o: ../../h/auth.h
or_init.o: ../../h/list_rchan.h
or_init.o: ../../h/list_bpt.h
or_init.o: ../../h/or.h
or_misc.o: or_misc.c
or_misc.o: ../../h/util.h
or_misc.o: ../../h/config.h
or_misc.o: ../../h/ll_log.h
or_misc.o: ../../h/table.h
or_misc.o: ../../h/or.h
or_misc.o: ../../h/ap.h
or_misc.o: ../../h/util.h
or_or2dmn.o: or_or2dmn.c
or_or2dmn.o: ../../h/or.h
or_or2dmn.o: ../../h/util.h
or_or2dmn.o: ../../h/config.h
or_or2dmn.o: ../../h/ll_log.h
or_or2rbits.o: or_or2rbits.c
or_or2rbits.o: ../../h/or.h
or_or2rbits.o: ../../h/util.h
or_or2rbits.o: ../../h/config.h
or_or2rbits.o: ../../h/ll_log.h
or_or2rbits.o: ../../h/table.h
or_or2rfc.o: or_or2rfc.c
or_or2rfc.o: ../../h/or.h
or_or2rfc.o: ../../h/util.h
or_or2rfc.o: ../../h/config.h
or_or2rfc.o: ../../h/ll_log.h
or_or2rfc.o: ../../h/ap.h
or_or2rfc.o: ../../h/util.h
or_or2std.o: or_or2std.c
or_or2std.o: ../../h/or.h
or_or2std.o: ../../h/util.h
or_or2std.o: ../../h/config.h
or_or2std.o: ../../h/ll_log.h
or_ps2asc.o: or_ps2asc.c
or_ps2asc.o: ../../h/util.h
or_ps2asc.o: ../../h/config.h
or_ps2asc.o: ../../h/ll_log.h
or_rbits2or.o: or_rbits2or.c
or_rbits2or.o: ../../h/or.h
or_rbits2or.o: ../../h/util.h
or_rbits2or.o: ../../h/config.h
or_rbits2or.o: ../../h/ll_log.h
or_rbits2or.o: ../../h/table.h
or_rfc2or.o: or_rfc2or.c
or_rfc2or.o: ../../h/or.h
or_rfc2or.o: ../../h/util.h
or_rfc2or.o: ../../h/config.h
or_rfc2or.o: ../../h/ll_log.h
or_rfc2or.o: ../../h/ap.h
or_rfc2or.o: ../../h/util.h
or_search.o: or_search.c
or_search.o: ../../h/or.h
or_search.o: ../../h/util.h
or_search.o: ../../h/config.h
or_search.o: ../../h/ll_log.h
or_std2or.o: or_std2or.c
or_std2or.o: ../../h/or.h
or_std2or.o: ../../h/util.h
or_std2or.o: ../../h/config.h
or_std2or.o: ../../h/ll_log.h
or_tables.o: or_tables.c
or_tables.o: ../../h/config.h
or_tables.o: ../../h/or.h
or_util.o: or_util.c
or_util.o: ../../h/util.h
or_util.o: ../../h/config.h
or_util.o: ../../h/ll_log.h
or_util.o: ../../h/or.h
orn-dec.o: orn-dec.c
orn-dec.o: ../../h/MTA-types.h
orn-dec.o: ../../h/Toks-types.h
orn-dec.o: ../../h/Auth-types.h
orn-dec.o: ../../h/Ext-types.h
orn-dec.o: ../../h/MTA-types.h
orn-dec.o: ../../h/Toks-types.h
orn-dec.o: ../../h/Auth-types.h
orn-dec.o: ../../h/util.h
orn-dec.o: ../../h/config.h
orn-dec.o: ../../h/ll_log.h
orn-dec.o: ../../h/or.h
orn-dec.o: ../../h/extension.h
orn-dec.o: ../../h/IOB-types.h
orn-dec.o: ../../h/MTA-types.h
orn-enc.o: orn-enc.c
orn-enc.o: ../../h/util.h
orn-enc.o: ../../h/config.h
orn-enc.o: ../../h/ll_log.h
orn-enc.o: ../../h/or.h
orn-enc.o: ../../h/IOB-types.h
orn-enc.o: ../../h/MTA-types.h
orn-enc.o: ../../h/Toks-types.h
orn-enc.o: ../../h/Auth-types.h
orn-enc.o: ../../h/MTA-types.h
orn-enc.o: ../../h/Ext-types.h
orn-enc.o: ../../h/MTA-types.h
orn-enc.o: ../../h/Toks-types.h
orn-enc.o: ../../h/Auth-types.h
orn-enc.o: ../../h/extension.h
orn-util.o: orn-util.c
orn-util.o: ../../h/util.h
orn-util.o: ../../h/config.h
orn-util.o: ../../h/ll_log.h
orn-util.o: ../../h/or.h
orn-util.o: ../../h/IOB-types.h
orn-util.o: ../../h/MTA-types.h
orn-util.o: ../../h/Toks-types.h
orn-util.o: ../../h/Auth-types.h
orn-util.o: ../../h/MTA-types.h
orn-util.o: ../../h/Ext-types.h
orn-util.o: ../../h/MTA-types.h
orn-util.o: ../../h/Toks-types.h
orn-util.o: ../../h/Auth-types.h
orn-util.o: ../../h/extension.h
pe2or.o: pe2or.c
pe2or.o: ../../h/util.h
pe2or.o: ../../h/config.h
pe2or.o: ../../h/ll_log.h
pe2or.o: ../../h/or.h
or_downgrade.o: or_downgrade.c
or_downgrade.o: ../../h/util.h
or_downgrade.o: ../../h/config.h
or_downgrade.o: ../../h/ll_log.h
or_downgrade.o: ../../h/or.h
or_downgrade.o: ../../h/ap.h
or_downgrade.o: ../../h/util.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
