# address parsing routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/parse/RCS/Makefile,v 6.0 1991/12/18 20:23:41 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:23:41  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ad_local.c ad_parse.c ad_getlocal.c \
     rfc822_dnorm.c rfc822_parse.c rfc822_x400.c rfc822_val.c \
     x400_parse.c x400_rfc822.c x400_val.c aparse_norm.c
OBJS =  ad_local.o ad_parse.o ad_getlocal.o \
     rfc822_dnorm.o rfc822_parse.o rfc822_x400.o rfc822_val.o \
     x400_parse.o x400_rfc822.o x400_val.o aparse_norm.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS	 = ../../h
INCLUDE	 = -I$(HEADERS)
LIBPP	= ../libpp.a

CFLAGS	  = $(CCOPTIONS) $(INCLUDE) $(LCF)
LLFLAGS	 = $(LINTFLAGS) $(INCLUDE) -u
LINTLIBS	= ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../tai/llib-ltai.ln \
		  ../util/llib-lutil.ln \
		  ../x400/llib-lx400.ln

SHADOW		= parse-made


############################################################
#
# Building rules
#
############################################################

default: $(SHADOW)

$(SHADOW): $(OBJS)
	   $(AR) r$(ARFLAGS) $(LIBPP) $?
	   touch $@

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS) 
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lintlib:llib-lparse.ln
llib-lparse.ln: ${SRCS}
	$(LINT) -Cparse $(LLFLAGS) ${SRCS}

clean: tidy
	rm -f ${OBJS} llib-lparse.ln

tidy:
	rm -f core ${SHADOW} *.old a.out *.BAK

lint: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

install: ;

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)


############################################################
#
# End of building rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
ad_local.o: ad_local.c
ad_local.o: ../../h/head.h
ad_local.o: ../../h/util.h
ad_local.o: ../../h/config.h
ad_local.o: ../../h/ll_log.h
ad_local.o: ../../h/retcode.h
ad_local.o: ../../h/adr.h
ad_local.o: ../../h/list_rchan.h
ad_local.o: ../../h/chan.h
ad_local.o: ../../h/table.h
ad_local.o: ../../h/list_bpt.h
ad_local.o: ../../h/auth.h
ad_local.o: ../../h/list_bpt.h
ad_local.o: ../../h/extension.h
ad_local.o: ../../h/mta.h
ad_local.o: ../../h/adr.h
ad_local.o: ../../h/list_bpt.h
ad_local.o: ../../h/aparse.h
ad_local.o: ../../h/ap.h
ad_local.o: ../../h/util.h
ad_local.o: ../../h/or.h
ad_local.o: ../../h/chan.h
ad_local.o: ../../h/auth.h
ad_local.o: ../../h/list_rchan.h
ad_local.o: ../../h/alias.h
ad_parse.o: ad_parse.c
ad_parse.o: ../../h/head.h
ad_parse.o: ../../h/util.h
ad_parse.o: ../../h/config.h
ad_parse.o: ../../h/ll_log.h
ad_parse.o: ../../h/retcode.h
ad_parse.o: ../../h/adr.h
ad_parse.o: ../../h/list_rchan.h
ad_parse.o: ../../h/chan.h
ad_parse.o: ../../h/table.h
ad_parse.o: ../../h/list_bpt.h
ad_parse.o: ../../h/auth.h
ad_parse.o: ../../h/list_bpt.h
ad_parse.o: ../../h/extension.h
ad_parse.o: ../../h/mta.h
ad_parse.o: ../../h/adr.h
ad_parse.o: ../../h/list_bpt.h
ad_parse.o: ../../h/aparse.h
ad_parse.o: ../../h/ap.h
ad_parse.o: ../../h/util.h
ad_parse.o: ../../h/or.h
ad_parse.o: ../../h/chan.h
ad_parse.o: ../../h/auth.h
ad_parse.o: ../../h/list_rchan.h
ad_getlocal.o: ad_getlocal.c
ad_getlocal.o: ../../h/head.h
ad_getlocal.o: ../../h/util.h
ad_getlocal.o: ../../h/config.h
ad_getlocal.o: ../../h/ll_log.h
ad_getlocal.o: ../../h/retcode.h
ad_getlocal.o: ../../h/adr.h
ad_getlocal.o: ../../h/list_rchan.h
ad_getlocal.o: ../../h/chan.h
ad_getlocal.o: ../../h/table.h
ad_getlocal.o: ../../h/list_bpt.h
ad_getlocal.o: ../../h/auth.h
ad_getlocal.o: ../../h/list_bpt.h
ad_getlocal.o: ../../h/extension.h
ad_getlocal.o: ../../h/mta.h
ad_getlocal.o: ../../h/adr.h
ad_getlocal.o: ../../h/list_bpt.h
ad_getlocal.o: ../../h/aparse.h
ad_getlocal.o: ../../h/ap.h
ad_getlocal.o: ../../h/util.h
ad_getlocal.o: ../../h/or.h
ad_getlocal.o: ../../h/chan.h
ad_getlocal.o: ../../h/auth.h
ad_getlocal.o: ../../h/list_rchan.h
rfc822_dnorm.o: rfc822_dnorm.c
rfc822_dnorm.o: ../../h/head.h
rfc822_dnorm.o: ../../h/util.h
rfc822_dnorm.o: ../../h/config.h
rfc822_dnorm.o: ../../h/ll_log.h
rfc822_dnorm.o: ../../h/retcode.h
rfc822_dnorm.o: ../../h/ap.h
rfc822_dnorm.o: ../../h/util.h
rfc822_dnorm.o: ../../h/chan.h
rfc822_dnorm.o: ../../h/table.h
rfc822_dnorm.o: ../../h/list_bpt.h
rfc822_parse.o: rfc822_parse.c
rfc822_parse.o: ../../h/head.h
rfc822_parse.o: ../../h/util.h
rfc822_parse.o: ../../h/config.h
rfc822_parse.o: ../../h/ll_log.h
rfc822_parse.o: ../../h/retcode.h
rfc822_parse.o: ../../h/adr.h
rfc822_parse.o: ../../h/list_rchan.h
rfc822_parse.o: ../../h/chan.h
rfc822_parse.o: ../../h/table.h
rfc822_parse.o: ../../h/list_bpt.h
rfc822_parse.o: ../../h/auth.h
rfc822_parse.o: ../../h/list_bpt.h
rfc822_parse.o: ../../h/extension.h
rfc822_parse.o: ../../h/mta.h
rfc822_parse.o: ../../h/adr.h
rfc822_parse.o: ../../h/list_bpt.h
rfc822_parse.o: ../../h/aparse.h
rfc822_parse.o: ../../h/ap.h
rfc822_parse.o: ../../h/util.h
rfc822_parse.o: ../../h/or.h
rfc822_parse.o: ../../h/chan.h
rfc822_parse.o: ../../h/auth.h
rfc822_parse.o: ../../h/list_rchan.h
rfc822_x400.o: rfc822_x400.c
rfc822_x400.o: ../../h/head.h
rfc822_x400.o: ../../h/util.h
rfc822_x400.o: ../../h/config.h
rfc822_x400.o: ../../h/ll_log.h
rfc822_x400.o: ../../h/retcode.h
rfc822_x400.o: ../../h/adr.h
rfc822_x400.o: ../../h/list_rchan.h
rfc822_x400.o: ../../h/chan.h
rfc822_x400.o: ../../h/table.h
rfc822_x400.o: ../../h/list_bpt.h
rfc822_x400.o: ../../h/auth.h
rfc822_x400.o: ../../h/list_bpt.h
rfc822_x400.o: ../../h/extension.h
rfc822_x400.o: ../../h/mta.h
rfc822_x400.o: ../../h/adr.h
rfc822_x400.o: ../../h/list_bpt.h
rfc822_x400.o: ../../h/aparse.h
rfc822_x400.o: ../../h/ap.h
rfc822_x400.o: ../../h/util.h
rfc822_x400.o: ../../h/or.h
rfc822_x400.o: ../../h/chan.h
rfc822_x400.o: ../../h/auth.h
rfc822_x400.o: ../../h/list_rchan.h
rfc822_val.o: rfc822_val.c
rfc822_val.o: ../../h/head.h
rfc822_val.o: ../../h/util.h
rfc822_val.o: ../../h/config.h
rfc822_val.o: ../../h/ll_log.h
rfc822_val.o: ../../h/retcode.h
rfc822_val.o: ../../h/adr.h
rfc822_val.o: ../../h/list_rchan.h
rfc822_val.o: ../../h/chan.h
rfc822_val.o: ../../h/table.h
rfc822_val.o: ../../h/list_bpt.h
rfc822_val.o: ../../h/auth.h
rfc822_val.o: ../../h/list_bpt.h
rfc822_val.o: ../../h/extension.h
rfc822_val.o: ../../h/mta.h
rfc822_val.o: ../../h/adr.h
rfc822_val.o: ../../h/list_bpt.h
rfc822_val.o: ../../h/aparse.h
rfc822_val.o: ../../h/ap.h
rfc822_val.o: ../../h/util.h
rfc822_val.o: ../../h/or.h
rfc822_val.o: ../../h/chan.h
rfc822_val.o: ../../h/auth.h
rfc822_val.o: ../../h/list_rchan.h
x400_parse.o: x400_parse.c
x400_parse.o: ../../h/head.h
x400_parse.o: ../../h/util.h
x400_parse.o: ../../h/config.h
x400_parse.o: ../../h/ll_log.h
x400_parse.o: ../../h/retcode.h
x400_parse.o: ../../h/adr.h
x400_parse.o: ../../h/list_rchan.h
x400_parse.o: ../../h/chan.h
x400_parse.o: ../../h/table.h
x400_parse.o: ../../h/list_bpt.h
x400_parse.o: ../../h/auth.h
x400_parse.o: ../../h/list_bpt.h
x400_parse.o: ../../h/extension.h
x400_parse.o: ../../h/mta.h
x400_parse.o: ../../h/adr.h
x400_parse.o: ../../h/list_bpt.h
x400_parse.o: ../../h/aparse.h
x400_parse.o: ../../h/ap.h
x400_parse.o: ../../h/util.h
x400_parse.o: ../../h/or.h
x400_parse.o: ../../h/chan.h
x400_parse.o: ../../h/auth.h
x400_parse.o: ../../h/list_rchan.h
x400_rfc822.o: x400_rfc822.c
x400_rfc822.o: ../../h/head.h
x400_rfc822.o: ../../h/util.h
x400_rfc822.o: ../../h/config.h
x400_rfc822.o: ../../h/ll_log.h
x400_rfc822.o: ../../h/retcode.h
x400_rfc822.o: ../../h/adr.h
x400_rfc822.o: ../../h/list_rchan.h
x400_rfc822.o: ../../h/chan.h
x400_rfc822.o: ../../h/table.h
x400_rfc822.o: ../../h/list_bpt.h
x400_rfc822.o: ../../h/auth.h
x400_rfc822.o: ../../h/list_bpt.h
x400_rfc822.o: ../../h/extension.h
x400_rfc822.o: ../../h/mta.h
x400_rfc822.o: ../../h/adr.h
x400_rfc822.o: ../../h/list_bpt.h
x400_rfc822.o: ../../h/aparse.h
x400_rfc822.o: ../../h/ap.h
x400_rfc822.o: ../../h/util.h
x400_rfc822.o: ../../h/or.h
x400_rfc822.o: ../../h/chan.h
x400_rfc822.o: ../../h/auth.h
x400_rfc822.o: ../../h/list_rchan.h
x400_val.o: x400_val.c
x400_val.o: ../../h/head.h
x400_val.o: ../../h/util.h
x400_val.o: ../../h/config.h
x400_val.o: ../../h/ll_log.h
x400_val.o: ../../h/retcode.h
x400_val.o: ../../h/adr.h
x400_val.o: ../../h/list_rchan.h
x400_val.o: ../../h/chan.h
x400_val.o: ../../h/table.h
x400_val.o: ../../h/list_bpt.h
x400_val.o: ../../h/auth.h
x400_val.o: ../../h/list_bpt.h
x400_val.o: ../../h/extension.h
x400_val.o: ../../h/mta.h
x400_val.o: ../../h/adr.h
x400_val.o: ../../h/list_bpt.h
x400_val.o: ../../h/aparse.h
x400_val.o: ../../h/ap.h
x400_val.o: ../../h/util.h
x400_val.o: ../../h/or.h
x400_val.o: ../../h/chan.h
x400_val.o: ../../h/auth.h
x400_val.o: ../../h/list_rchan.h
x400_val.o: ../../h/or.h
aparse_norm.o: aparse_norm.c
aparse_norm.o: ../../h/util.h
aparse_norm.o: ../../h/config.h
aparse_norm.o: ../../h/ll_log.h
aparse_norm.o: ../../h/adr.h
aparse_norm.o: ../../h/list_rchan.h
aparse_norm.o: ../../h/chan.h
aparse_norm.o: ../../h/table.h
aparse_norm.o: ../../h/list_bpt.h
aparse_norm.o: ../../h/auth.h
aparse_norm.o: ../../h/list_bpt.h
aparse_norm.o: ../../h/extension.h
aparse_norm.o: ../../h/mta.h
aparse_norm.o: ../../h/adr.h
aparse_norm.o: ../../h/list_bpt.h
aparse_norm.o: ../../h/aparse.h
aparse_norm.o: ../../h/ap.h
aparse_norm.o: ../../h/util.h
aparse_norm.o: ../../h/or.h
aparse_norm.o: ../../h/chan.h
aparse_norm.o: ../../h/auth.h
aparse_norm.o: ../../h/list_rchan.h
aparse_norm.o: ../../h/aparse.h
aparse_norm.o: ../../h/alias.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
