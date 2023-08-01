# format lib Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/format/RCS/Makefile,v 6.0 1991/12/18 20:22:06 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:22:06  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  UTC2rfc.c adr2rfc.c domsinfo2rfc.c globalid2rfc.c \
           msgid2rfc.c reprecip2rfc.c rfc2UTC.c rfc2domsinfo.c rfc2encinfo.c \
           rfc2globalid.c rfc2msgid.c rfc2reprecip.c rfc2trace.c \
           rfctxtfold.c trace2rfc.c x400trc2rfc.c rfc2x400trc.c
OBJS =  UTC2rfc.o adr2rfc.o domsinfo2rfc.o globalid2rfc.o \
           msgid2rfc.o reprecip2rfc.o rfc2UTC.o rfc2domsinfo.o rfc2encinfo.o \
           rfc2globalid.o rfc2msgid.o rfc2reprecip.o rfc2trace.o \
           rfctxtfold.o trace2rfc.o x400trc2rfc.o rfc2x400trc.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = fmt-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
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

default: $(SHADOW)

$(SHADOW): $(OBJS)
	$(AR) r$(ARFLAGS) ../libpp.a $?
	touch $@

lintlib:llib-lfmt.ln
llib-lfmt.ln: $(SRCS)
	$(LINT) -Cfmt $(LLFLAGS) $(SRCS)

clean: tidy
	rm -f $(OBJS) llib-lfmt.ln
tidy:
	rm -f core $(SHADOW) *.old *.BAK

install:

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of building rules
#
############################################################


# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
UTC2rfc.o: UTC2rfc.c
UTC2rfc.o: ../../h/util.h
UTC2rfc.o: ../../h/config.h
UTC2rfc.o: ../../h/ll_log.h
adr2rfc.o: adr2rfc.c
adr2rfc.o: ../../h/util.h
adr2rfc.o: ../../h/config.h
adr2rfc.o: ../../h/ll_log.h
adr2rfc.o: ../../h/or.h
adr2rfc.o: ../../h/adr.h
adr2rfc.o: ../../h/list_rchan.h
adr2rfc.o: ../../h/chan.h
adr2rfc.o: ../../h/table.h
adr2rfc.o: ../../h/list_bpt.h
adr2rfc.o: ../../h/auth.h
adr2rfc.o: ../../h/list_bpt.h
adr2rfc.o: ../../h/extension.h
adr2rfc.o: ../../h/mta.h
adr2rfc.o: ../../h/adr.h
adr2rfc.o: ../../h/list_bpt.h
adr2rfc.o: ../../h/aparse.h
adr2rfc.o: ../../h/ap.h
adr2rfc.o: ../../h/util.h
adr2rfc.o: ../../h/or.h
adr2rfc.o: ../../h/chan.h
adr2rfc.o: ../../h/auth.h
adr2rfc.o: ../../h/list_rchan.h
domsinfo2rfc.o: domsinfo2rfc.c
domsinfo2rfc.o: ../../h/util.h
domsinfo2rfc.o: ../../h/config.h
domsinfo2rfc.o: ../../h/ll_log.h
domsinfo2rfc.o: ../../h/mta.h
domsinfo2rfc.o: ../../h/adr.h
domsinfo2rfc.o: ../../h/list_rchan.h
domsinfo2rfc.o: ../../h/chan.h
domsinfo2rfc.o: ../../h/table.h
domsinfo2rfc.o: ../../h/list_bpt.h
domsinfo2rfc.o: ../../h/auth.h
domsinfo2rfc.o: ../../h/list_bpt.h
domsinfo2rfc.o: ../../h/extension.h
domsinfo2rfc.o: ../../h/mta.h
domsinfo2rfc.o: ../../h/aparse.h
domsinfo2rfc.o: ../../h/ap.h
domsinfo2rfc.o: ../../h/util.h
domsinfo2rfc.o: ../../h/or.h
domsinfo2rfc.o: ../../h/chan.h
domsinfo2rfc.o: ../../h/auth.h
domsinfo2rfc.o: ../../h/list_rchan.h
domsinfo2rfc.o: ../../h/list_bpt.h
domsinfo2rfc.o: ../../h/tb_p1.h
globalid2rfc.o: globalid2rfc.c
globalid2rfc.o: ../../h/util.h
globalid2rfc.o: ../../h/config.h
globalid2rfc.o: ../../h/ll_log.h
globalid2rfc.o: ../../h/mta.h
globalid2rfc.o: ../../h/adr.h
globalid2rfc.o: ../../h/list_rchan.h
globalid2rfc.o: ../../h/chan.h
globalid2rfc.o: ../../h/table.h
globalid2rfc.o: ../../h/list_bpt.h
globalid2rfc.o: ../../h/auth.h
globalid2rfc.o: ../../h/list_bpt.h
globalid2rfc.o: ../../h/extension.h
globalid2rfc.o: ../../h/mta.h
globalid2rfc.o: ../../h/aparse.h
globalid2rfc.o: ../../h/ap.h
globalid2rfc.o: ../../h/util.h
globalid2rfc.o: ../../h/or.h
globalid2rfc.o: ../../h/chan.h
globalid2rfc.o: ../../h/auth.h
globalid2rfc.o: ../../h/list_rchan.h
globalid2rfc.o: ../../h/list_bpt.h
msgid2rfc.o: msgid2rfc.c
msgid2rfc.o: ../../h/util.h
msgid2rfc.o: ../../h/config.h
msgid2rfc.o: ../../h/ll_log.h
msgid2rfc.o: ../../h/mta.h
msgid2rfc.o: ../../h/adr.h
msgid2rfc.o: ../../h/list_rchan.h
msgid2rfc.o: ../../h/chan.h
msgid2rfc.o: ../../h/table.h
msgid2rfc.o: ../../h/list_bpt.h
msgid2rfc.o: ../../h/auth.h
msgid2rfc.o: ../../h/list_bpt.h
msgid2rfc.o: ../../h/extension.h
msgid2rfc.o: ../../h/mta.h
msgid2rfc.o: ../../h/aparse.h
msgid2rfc.o: ../../h/ap.h
msgid2rfc.o: ../../h/util.h
msgid2rfc.o: ../../h/or.h
msgid2rfc.o: ../../h/chan.h
msgid2rfc.o: ../../h/auth.h
msgid2rfc.o: ../../h/list_rchan.h
msgid2rfc.o: ../../h/list_bpt.h
reprecip2rfc.o: reprecip2rfc.c
reprecip2rfc.o: ../../h/util.h
reprecip2rfc.o: ../../h/config.h
reprecip2rfc.o: ../../h/ll_log.h
reprecip2rfc.o: ../../h/dr.h
reprecip2rfc.o: ../../h/extension.h
reprecip2rfc.o: ../../h/mta.h
reprecip2rfc.o: ../../h/adr.h
reprecip2rfc.o: ../../h/list_rchan.h
reprecip2rfc.o: ../../h/chan.h
reprecip2rfc.o: ../../h/table.h
reprecip2rfc.o: ../../h/list_bpt.h
reprecip2rfc.o: ../../h/auth.h
reprecip2rfc.o: ../../h/list_bpt.h
reprecip2rfc.o: ../../h/extension.h
reprecip2rfc.o: ../../h/mta.h
reprecip2rfc.o: ../../h/aparse.h
reprecip2rfc.o: ../../h/ap.h
reprecip2rfc.o: ../../h/util.h
reprecip2rfc.o: ../../h/or.h
reprecip2rfc.o: ../../h/chan.h
reprecip2rfc.o: ../../h/auth.h
reprecip2rfc.o: ../../h/list_rchan.h
reprecip2rfc.o: ../../h/list_bpt.h
rfc2UTC.o: rfc2UTC.c
rfc2UTC.o: ../../h/util.h
rfc2UTC.o: ../../h/config.h
rfc2UTC.o: ../../h/ll_log.h
rfc2UTC.o: ../../h/mta.h
rfc2UTC.o: ../../h/adr.h
rfc2UTC.o: ../../h/list_rchan.h
rfc2UTC.o: ../../h/chan.h
rfc2UTC.o: ../../h/table.h
rfc2UTC.o: ../../h/list_bpt.h
rfc2UTC.o: ../../h/auth.h
rfc2UTC.o: ../../h/list_bpt.h
rfc2UTC.o: ../../h/extension.h
rfc2UTC.o: ../../h/mta.h
rfc2UTC.o: ../../h/aparse.h
rfc2UTC.o: ../../h/ap.h
rfc2UTC.o: ../../h/util.h
rfc2UTC.o: ../../h/or.h
rfc2UTC.o: ../../h/chan.h
rfc2UTC.o: ../../h/auth.h
rfc2UTC.o: ../../h/list_rchan.h
rfc2UTC.o: ../../h/list_bpt.h
rfc2domsinfo.o: rfc2domsinfo.c
rfc2domsinfo.o: ../../h/util.h
rfc2domsinfo.o: ../../h/config.h
rfc2domsinfo.o: ../../h/ll_log.h
rfc2domsinfo.o: ../../h/mta.h
rfc2domsinfo.o: ../../h/adr.h
rfc2domsinfo.o: ../../h/list_rchan.h
rfc2domsinfo.o: ../../h/chan.h
rfc2domsinfo.o: ../../h/table.h
rfc2domsinfo.o: ../../h/list_bpt.h
rfc2domsinfo.o: ../../h/auth.h
rfc2domsinfo.o: ../../h/list_bpt.h
rfc2domsinfo.o: ../../h/extension.h
rfc2domsinfo.o: ../../h/mta.h
rfc2domsinfo.o: ../../h/aparse.h
rfc2domsinfo.o: ../../h/ap.h
rfc2domsinfo.o: ../../h/util.h
rfc2domsinfo.o: ../../h/or.h
rfc2domsinfo.o: ../../h/chan.h
rfc2domsinfo.o: ../../h/auth.h
rfc2domsinfo.o: ../../h/list_rchan.h
rfc2domsinfo.o: ../../h/list_bpt.h
rfc2domsinfo.o: ../../h/tb_p1.h
rfc2encinfo.o: rfc2encinfo.c
rfc2encinfo.o: ../../h/util.h
rfc2encinfo.o: ../../h/config.h
rfc2encinfo.o: ../../h/ll_log.h
rfc2encinfo.o: ../../h/mta.h
rfc2encinfo.o: ../../h/adr.h
rfc2encinfo.o: ../../h/list_rchan.h
rfc2encinfo.o: ../../h/chan.h
rfc2encinfo.o: ../../h/table.h
rfc2encinfo.o: ../../h/list_bpt.h
rfc2encinfo.o: ../../h/auth.h
rfc2encinfo.o: ../../h/list_bpt.h
rfc2encinfo.o: ../../h/extension.h
rfc2encinfo.o: ../../h/mta.h
rfc2encinfo.o: ../../h/aparse.h
rfc2encinfo.o: ../../h/ap.h
rfc2encinfo.o: ../../h/util.h
rfc2encinfo.o: ../../h/or.h
rfc2encinfo.o: ../../h/chan.h
rfc2encinfo.o: ../../h/auth.h
rfc2encinfo.o: ../../h/list_rchan.h
rfc2encinfo.o: ../../h/list_bpt.h
rfc2encinfo.o: ../../h/list_bpt.h
rfc2globalid.o: rfc2globalid.c
rfc2globalid.o: ../../h/util.h
rfc2globalid.o: ../../h/config.h
rfc2globalid.o: ../../h/ll_log.h
rfc2globalid.o: ../../h/mta.h
rfc2globalid.o: ../../h/adr.h
rfc2globalid.o: ../../h/list_rchan.h
rfc2globalid.o: ../../h/chan.h
rfc2globalid.o: ../../h/table.h
rfc2globalid.o: ../../h/list_bpt.h
rfc2globalid.o: ../../h/auth.h
rfc2globalid.o: ../../h/list_bpt.h
rfc2globalid.o: ../../h/extension.h
rfc2globalid.o: ../../h/mta.h
rfc2globalid.o: ../../h/aparse.h
rfc2globalid.o: ../../h/ap.h
rfc2globalid.o: ../../h/util.h
rfc2globalid.o: ../../h/or.h
rfc2globalid.o: ../../h/chan.h
rfc2globalid.o: ../../h/auth.h
rfc2globalid.o: ../../h/list_rchan.h
rfc2globalid.o: ../../h/list_bpt.h
rfc2msgid.o: rfc2msgid.c
rfc2msgid.o: ../../h/util.h
rfc2msgid.o: ../../h/config.h
rfc2msgid.o: ../../h/ll_log.h
rfc2msgid.o: ../../h/mta.h
rfc2msgid.o: ../../h/adr.h
rfc2msgid.o: ../../h/list_rchan.h
rfc2msgid.o: ../../h/chan.h
rfc2msgid.o: ../../h/table.h
rfc2msgid.o: ../../h/list_bpt.h
rfc2msgid.o: ../../h/auth.h
rfc2msgid.o: ../../h/list_bpt.h
rfc2msgid.o: ../../h/extension.h
rfc2msgid.o: ../../h/mta.h
rfc2msgid.o: ../../h/aparse.h
rfc2msgid.o: ../../h/ap.h
rfc2msgid.o: ../../h/util.h
rfc2msgid.o: ../../h/or.h
rfc2msgid.o: ../../h/chan.h
rfc2msgid.o: ../../h/auth.h
rfc2msgid.o: ../../h/list_rchan.h
rfc2msgid.o: ../../h/list_bpt.h
rfc2reprecip.o: rfc2reprecip.c
rfc2reprecip.o: ../../h/util.h
rfc2reprecip.o: ../../h/config.h
rfc2reprecip.o: ../../h/ll_log.h
rfc2reprecip.o: ../../h/dr.h
rfc2reprecip.o: ../../h/extension.h
rfc2reprecip.o: ../../h/mta.h
rfc2reprecip.o: ../../h/adr.h
rfc2reprecip.o: ../../h/list_rchan.h
rfc2reprecip.o: ../../h/chan.h
rfc2reprecip.o: ../../h/table.h
rfc2reprecip.o: ../../h/list_bpt.h
rfc2reprecip.o: ../../h/auth.h
rfc2reprecip.o: ../../h/list_bpt.h
rfc2reprecip.o: ../../h/extension.h
rfc2reprecip.o: ../../h/mta.h
rfc2reprecip.o: ../../h/aparse.h
rfc2reprecip.o: ../../h/ap.h
rfc2reprecip.o: ../../h/util.h
rfc2reprecip.o: ../../h/or.h
rfc2reprecip.o: ../../h/chan.h
rfc2reprecip.o: ../../h/auth.h
rfc2reprecip.o: ../../h/list_rchan.h
rfc2reprecip.o: ../../h/list_bpt.h
rfc2trace.o: rfc2trace.c
rfc2trace.o: ../../h/util.h
rfc2trace.o: ../../h/config.h
rfc2trace.o: ../../h/ll_log.h
rfc2trace.o: ../../h/mta.h
rfc2trace.o: ../../h/adr.h
rfc2trace.o: ../../h/list_rchan.h
rfc2trace.o: ../../h/chan.h
rfc2trace.o: ../../h/table.h
rfc2trace.o: ../../h/list_bpt.h
rfc2trace.o: ../../h/auth.h
rfc2trace.o: ../../h/list_bpt.h
rfc2trace.o: ../../h/extension.h
rfc2trace.o: ../../h/mta.h
rfc2trace.o: ../../h/aparse.h
rfc2trace.o: ../../h/ap.h
rfc2trace.o: ../../h/util.h
rfc2trace.o: ../../h/or.h
rfc2trace.o: ../../h/chan.h
rfc2trace.o: ../../h/auth.h
rfc2trace.o: ../../h/list_rchan.h
rfc2trace.o: ../../h/list_bpt.h
rfctxtfold.o: rfctxtfold.c
rfctxtfold.o: ../../h/util.h
rfctxtfold.o: ../../h/config.h
rfctxtfold.o: ../../h/ll_log.h
trace2rfc.o: trace2rfc.c
trace2rfc.o: ../../h/util.h
trace2rfc.o: ../../h/config.h
trace2rfc.o: ../../h/ll_log.h
trace2rfc.o: ../../h/mta.h
trace2rfc.o: ../../h/adr.h
trace2rfc.o: ../../h/list_rchan.h
trace2rfc.o: ../../h/chan.h
trace2rfc.o: ../../h/table.h
trace2rfc.o: ../../h/list_bpt.h
trace2rfc.o: ../../h/auth.h
trace2rfc.o: ../../h/list_bpt.h
trace2rfc.o: ../../h/extension.h
trace2rfc.o: ../../h/mta.h
trace2rfc.o: ../../h/aparse.h
trace2rfc.o: ../../h/ap.h
trace2rfc.o: ../../h/util.h
trace2rfc.o: ../../h/or.h
trace2rfc.o: ../../h/chan.h
trace2rfc.o: ../../h/auth.h
trace2rfc.o: ../../h/list_rchan.h
trace2rfc.o: ../../h/list_bpt.h
x400trc2rfc.o: x400trc2rfc.c
x400trc2rfc.o: ../../h/util.h
x400trc2rfc.o: ../../h/config.h
x400trc2rfc.o: ../../h/ll_log.h
x400trc2rfc.o: ../../h/mta.h
x400trc2rfc.o: ../../h/adr.h
x400trc2rfc.o: ../../h/list_rchan.h
x400trc2rfc.o: ../../h/chan.h
x400trc2rfc.o: ../../h/table.h
x400trc2rfc.o: ../../h/list_bpt.h
x400trc2rfc.o: ../../h/auth.h
x400trc2rfc.o: ../../h/list_bpt.h
x400trc2rfc.o: ../../h/extension.h
x400trc2rfc.o: ../../h/mta.h
x400trc2rfc.o: ../../h/aparse.h
x400trc2rfc.o: ../../h/ap.h
x400trc2rfc.o: ../../h/util.h
x400trc2rfc.o: ../../h/or.h
x400trc2rfc.o: ../../h/chan.h
x400trc2rfc.o: ../../h/auth.h
x400trc2rfc.o: ../../h/list_rchan.h
x400trc2rfc.o: ../../h/list_bpt.h
rfc2x400trc.o: rfc2x400trc.c
rfc2x400trc.o: ../../h/util.h
rfc2x400trc.o: ../../h/config.h
rfc2x400trc.o: ../../h/ll_log.h
rfc2x400trc.o: ../../h/mta.h
rfc2x400trc.o: ../../h/adr.h
rfc2x400trc.o: ../../h/list_rchan.h
rfc2x400trc.o: ../../h/chan.h
rfc2x400trc.o: ../../h/table.h
rfc2x400trc.o: ../../h/list_bpt.h
rfc2x400trc.o: ../../h/auth.h
rfc2x400trc.o: ../../h/list_bpt.h
rfc2x400trc.o: ../../h/extension.h
rfc2x400trc.o: ../../h/mta.h
rfc2x400trc.o: ../../h/aparse.h
rfc2x400trc.o: ../../h/ap.h
rfc2x400trc.o: ../../h/util.h
rfc2x400trc.o: ../../h/or.h
rfc2x400trc.o: ../../h/chan.h
rfc2x400trc.o: ../../h/auth.h
rfc2x400trc.o: ../../h/list_rchan.h
rfc2x400trc.o: ../../h/list_bpt.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
