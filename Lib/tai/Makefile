# Tailoring routines
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/tai/RCS/Makefile,v 6.0 1991/12/18 20:24:59 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:24:59  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  init_chan.c init_tai.c init_uip.c pp_setuserid.c \
           tai_chan.c tai_sys.c tai_tb.c
OBJS =  init_chan.o init_tai.o init_uip.o pp_setuserid.o \
           tai_chan.o tai_sys.o tai_tb.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = tai-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../util/llib-lutil.ln \
		  ../x400/llib-lx400.ln



############################################################
#
# Building Rules
#
############################################################

default: ${SHADOW}

${SHADOW}: $(OBJS)
	$(AR) r$(ARFLAGS) ../libpp.a $?
	touch $@


install:;

lintlib:llib-ltai.ln
llib-ltai.ln: ${SRCS}
	$(LINT) -Ctai $(LLFLAGS) ${SRCS}

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

clean: tidy
	rm -f ${OBJS} llib-ltai.ln
tidy:
	rm -f core ${SHADOW} *.old  a.out *.BAK

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
init_chan.o: init_chan.c
init_tai.o: init_tai.c
init_tai.o: ../../h/head.h
init_tai.o: ../../h/util.h
init_tai.o: ../../h/config.h
init_tai.o: ../../h/ll_log.h
init_tai.o: ../../h/retcode.h
init_uip.o: init_uip.c
pp_setuserid.o: pp_setuserid.c
pp_setuserid.o: ../../h/util.h
pp_setuserid.o: ../../h/config.h
pp_setuserid.o: ../../h/ll_log.h
tai_chan.o: tai_chan.c
tai_chan.o: ../../h/head.h
tai_chan.o: ../../h/util.h
tai_chan.o: ../../h/config.h
tai_chan.o: ../../h/ll_log.h
tai_chan.o: ../../h/retcode.h
tai_chan.o: ../../h/chan.h
tai_chan.o: ../../h/table.h
tai_chan.o: ../../h/list_bpt.h
tai_chan.o: ../../h/adr.h
tai_chan.o: ../../h/list_rchan.h
tai_chan.o: ../../h/chan.h
tai_chan.o: ../../h/auth.h
tai_chan.o: ../../h/list_bpt.h
tai_chan.o: ../../h/extension.h
tai_chan.o: ../../h/mta.h
tai_chan.o: ../../h/adr.h
tai_chan.o: ../../h/list_bpt.h
tai_chan.o: ../../h/aparse.h
tai_chan.o: ../../h/ap.h
tai_chan.o: ../../h/util.h
tai_chan.o: ../../h/or.h
tai_chan.o: ../../h/chan.h
tai_chan.o: ../../h/auth.h
tai_chan.o: ../../h/list_rchan.h
tai_sys.o: tai_sys.c
tai_sys.o: ../../h/head.h
tai_sys.o: ../../h/util.h
tai_sys.o: ../../h/config.h
tai_sys.o: ../../h/ll_log.h
tai_sys.o: ../../h/retcode.h
tai_sys.o: ../../h/list_bpt.h
tai_tb.o: tai_tb.c
tai_tb.o: ../../h/util.h
tai_tb.o: ../../h/config.h
tai_tb.o: ../../h/ll_log.h
tai_tb.o: ../../h/retcode.h
tai_tb.o: ../../h/table.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
