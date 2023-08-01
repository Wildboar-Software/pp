# Input/Output primitives for PP
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/io/RCS/Makefile,v 6.0 1991/12/18 20:22:26 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:22:26  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  io_lib.c rd_adr.c rd_buf.c rd_dr.c rd_msg.c rd_prm.c rd_q.c wr_adr.c \
           wr_dr.c wr_prm.c wr_q.c wr_q2drfile.c
OBJS =  io_lib.o rd_adr.o rd_buf.o rd_dr.o rd_msg.o rd_prm.o rd_q.o wr_adr.o \
           wr_dr.o wr_prm.o wr_q.o wr_q2drfile.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = io-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
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

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS) 
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lintlib:llib-lio.ln
llib-lio.ln: $(SRCS)
	$(LINT) $(LLFLAGS) -Cio $(SRCS)

clean: tidy
	rm -f $(OBJS) llib-lio.ln
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
io_lib.o: io_lib.c
io_lib.o: ../../h/head.h
io_lib.o: ../../h/util.h
io_lib.o: ../../h/config.h
io_lib.o: ../../h/ll_log.h
io_lib.o: ../../h/retcode.h
io_lib.o: ../../h/q.h
io_lib.o: ../../h/adr.h
io_lib.o: ../../h/list_rchan.h
io_lib.o: ../../h/chan.h
io_lib.o: ../../h/table.h
io_lib.o: ../../h/list_bpt.h
io_lib.o: ../../h/auth.h
io_lib.o: ../../h/list_bpt.h
io_lib.o: ../../h/extension.h
io_lib.o: ../../h/mta.h
io_lib.o: ../../h/adr.h
io_lib.o: ../../h/list_bpt.h
io_lib.o: ../../h/aparse.h
io_lib.o: ../../h/ap.h
io_lib.o: ../../h/util.h
io_lib.o: ../../h/or.h
io_lib.o: ../../h/chan.h
io_lib.o: ../../h/auth.h
io_lib.o: ../../h/list_rchan.h
io_lib.o: ../../h/mta.h
io_lib.o: ../../h/adr.h
io_lib.o: ../../h/prm.h
io_lib.o: ../../h/dr.h
io_lib.o: ../../h/extension.h
io_lib.o: ../../h/mta.h
rd_adr.o: rd_adr.c
rd_adr.o: ../../h/head.h
rd_adr.o: ../../h/util.h
rd_adr.o: ../../h/config.h
rd_adr.o: ../../h/ll_log.h
rd_adr.o: ../../h/retcode.h
rd_adr.o: ../../h/adr.h
rd_adr.o: ../../h/list_rchan.h
rd_adr.o: ../../h/chan.h
rd_adr.o: ../../h/table.h
rd_adr.o: ../../h/list_bpt.h
rd_adr.o: ../../h/auth.h
rd_adr.o: ../../h/list_bpt.h
rd_adr.o: ../../h/extension.h
rd_adr.o: ../../h/mta.h
rd_adr.o: ../../h/adr.h
rd_adr.o: ../../h/list_bpt.h
rd_adr.o: ../../h/aparse.h
rd_adr.o: ../../h/ap.h
rd_adr.o: ../../h/util.h
rd_adr.o: ../../h/or.h
rd_adr.o: ../../h/chan.h
rd_adr.o: ../../h/auth.h
rd_adr.o: ../../h/list_rchan.h
rd_adr.o: ../../h/sys.file.h
rd_adr.o: ../../h/util.h
rd_buf.o: rd_buf.c
rd_buf.o: ../../h/head.h
rd_buf.o: ../../h/util.h
rd_buf.o: ../../h/config.h
rd_buf.o: ../../h/ll_log.h
rd_buf.o: ../../h/retcode.h
rd_dr.o: rd_dr.c
rd_dr.o: ../../h/head.h
rd_dr.o: ../../h/util.h
rd_dr.o: ../../h/config.h
rd_dr.o: ../../h/ll_log.h
rd_dr.o: ../../h/retcode.h
rd_dr.o: ../../h/dr.h
rd_dr.o: ../../h/extension.h
rd_dr.o: ../../h/mta.h
rd_dr.o: ../../h/adr.h
rd_dr.o: ../../h/list_rchan.h
rd_dr.o: ../../h/chan.h
rd_dr.o: ../../h/table.h
rd_dr.o: ../../h/list_bpt.h
rd_dr.o: ../../h/auth.h
rd_dr.o: ../../h/list_bpt.h
rd_dr.o: ../../h/extension.h
rd_dr.o: ../../h/mta.h
rd_dr.o: ../../h/aparse.h
rd_dr.o: ../../h/ap.h
rd_dr.o: ../../h/util.h
rd_dr.o: ../../h/or.h
rd_dr.o: ../../h/chan.h
rd_dr.o: ../../h/auth.h
rd_dr.o: ../../h/list_rchan.h
rd_dr.o: ../../h/list_bpt.h
rd_msg.o: rd_msg.c
rd_msg.o: ../../h/head.h
rd_msg.o: ../../h/util.h
rd_msg.o: ../../h/config.h
rd_msg.o: ../../h/ll_log.h
rd_msg.o: ../../h/retcode.h
rd_msg.o: ../../h/q.h
rd_msg.o: ../../h/adr.h
rd_msg.o: ../../h/list_rchan.h
rd_msg.o: ../../h/chan.h
rd_msg.o: ../../h/table.h
rd_msg.o: ../../h/list_bpt.h
rd_msg.o: ../../h/auth.h
rd_msg.o: ../../h/list_bpt.h
rd_msg.o: ../../h/extension.h
rd_msg.o: ../../h/mta.h
rd_msg.o: ../../h/adr.h
rd_msg.o: ../../h/list_bpt.h
rd_msg.o: ../../h/aparse.h
rd_msg.o: ../../h/ap.h
rd_msg.o: ../../h/util.h
rd_msg.o: ../../h/or.h
rd_msg.o: ../../h/chan.h
rd_msg.o: ../../h/auth.h
rd_msg.o: ../../h/list_rchan.h
rd_msg.o: ../../h/mta.h
rd_msg.o: ../../h/prm.h
rd_prm.o: rd_prm.c
rd_prm.o: ../../h/head.h
rd_prm.o: ../../h/util.h
rd_prm.o: ../../h/config.h
rd_prm.o: ../../h/ll_log.h
rd_prm.o: ../../h/retcode.h
rd_prm.o: ../../h/prm.h
rd_prm.o: ../../h/tb_prm.h
rd_q.o: rd_q.c
rd_q.o: ../../h/head.h
rd_q.o: ../../h/util.h
rd_q.o: ../../h/config.h
rd_q.o: ../../h/ll_log.h
rd_q.o: ../../h/retcode.h
rd_q.o: ../../h/q.h
rd_q.o: ../../h/adr.h
rd_q.o: ../../h/list_rchan.h
rd_q.o: ../../h/chan.h
rd_q.o: ../../h/table.h
rd_q.o: ../../h/list_bpt.h
rd_q.o: ../../h/auth.h
rd_q.o: ../../h/list_bpt.h
rd_q.o: ../../h/extension.h
rd_q.o: ../../h/mta.h
rd_q.o: ../../h/adr.h
rd_q.o: ../../h/list_bpt.h
rd_q.o: ../../h/aparse.h
rd_q.o: ../../h/ap.h
rd_q.o: ../../h/util.h
rd_q.o: ../../h/or.h
rd_q.o: ../../h/chan.h
rd_q.o: ../../h/auth.h
rd_q.o: ../../h/list_rchan.h
rd_q.o: ../../h/mta.h
rd_q.o: ../../h/tb_q.h
wr_adr.o: wr_adr.c
wr_adr.o: ../../h/head.h
wr_adr.o: ../../h/util.h
wr_adr.o: ../../h/config.h
wr_adr.o: ../../h/ll_log.h
wr_adr.o: ../../h/retcode.h
wr_adr.o: ../../h/adr.h
wr_adr.o: ../../h/list_rchan.h
wr_adr.o: ../../h/chan.h
wr_adr.o: ../../h/table.h
wr_adr.o: ../../h/list_bpt.h
wr_adr.o: ../../h/auth.h
wr_adr.o: ../../h/list_bpt.h
wr_adr.o: ../../h/extension.h
wr_adr.o: ../../h/mta.h
wr_adr.o: ../../h/adr.h
wr_adr.o: ../../h/list_bpt.h
wr_adr.o: ../../h/aparse.h
wr_adr.o: ../../h/ap.h
wr_adr.o: ../../h/util.h
wr_adr.o: ../../h/or.h
wr_adr.o: ../../h/chan.h
wr_adr.o: ../../h/auth.h
wr_adr.o: ../../h/list_rchan.h
wr_adr.o: ../../h/sys.file.h
wr_adr.o: ../../h/util.h
wr_dr.o: wr_dr.c
wr_dr.o: ../../h/head.h
wr_dr.o: ../../h/util.h
wr_dr.o: ../../h/config.h
wr_dr.o: ../../h/ll_log.h
wr_dr.o: ../../h/retcode.h
wr_dr.o: ../../h/q.h
wr_dr.o: ../../h/adr.h
wr_dr.o: ../../h/list_rchan.h
wr_dr.o: ../../h/chan.h
wr_dr.o: ../../h/table.h
wr_dr.o: ../../h/list_bpt.h
wr_dr.o: ../../h/auth.h
wr_dr.o: ../../h/list_bpt.h
wr_dr.o: ../../h/extension.h
wr_dr.o: ../../h/mta.h
wr_dr.o: ../../h/adr.h
wr_dr.o: ../../h/list_bpt.h
wr_dr.o: ../../h/aparse.h
wr_dr.o: ../../h/ap.h
wr_dr.o: ../../h/util.h
wr_dr.o: ../../h/or.h
wr_dr.o: ../../h/chan.h
wr_dr.o: ../../h/auth.h
wr_dr.o: ../../h/list_rchan.h
wr_dr.o: ../../h/mta.h
wr_dr.o: ../../h/dr.h
wr_dr.o: ../../h/extension.h
wr_dr.o: ../../h/mta.h
wr_prm.o: wr_prm.c
wr_prm.o: ../../h/head.h
wr_prm.o: ../../h/util.h
wr_prm.o: ../../h/config.h
wr_prm.o: ../../h/ll_log.h
wr_prm.o: ../../h/retcode.h
wr_prm.o: ../../h/prm.h
wr_q.o: wr_q.c
wr_q.o: ../../h/head.h
wr_q.o: ../../h/util.h
wr_q.o: ../../h/config.h
wr_q.o: ../../h/ll_log.h
wr_q.o: ../../h/retcode.h
wr_q.o: ../../h/q.h
wr_q.o: ../../h/adr.h
wr_q.o: ../../h/list_rchan.h
wr_q.o: ../../h/chan.h
wr_q.o: ../../h/table.h
wr_q.o: ../../h/list_bpt.h
wr_q.o: ../../h/auth.h
wr_q.o: ../../h/list_bpt.h
wr_q.o: ../../h/extension.h
wr_q.o: ../../h/mta.h
wr_q.o: ../../h/adr.h
wr_q.o: ../../h/list_bpt.h
wr_q.o: ../../h/aparse.h
wr_q.o: ../../h/ap.h
wr_q.o: ../../h/util.h
wr_q.o: ../../h/or.h
wr_q.o: ../../h/chan.h
wr_q.o: ../../h/auth.h
wr_q.o: ../../h/list_rchan.h
wr_q.o: ../../h/mta.h
wr_q2drfile.o: wr_q2drfile.c
wr_q2drfile.o: ../../h/head.h
wr_q2drfile.o: ../../h/util.h
wr_q2drfile.o: ../../h/config.h
wr_q2drfile.o: ../../h/ll_log.h
wr_q2drfile.o: ../../h/retcode.h
wr_q2drfile.o: ../../h/q.h
wr_q2drfile.o: ../../h/adr.h
wr_q2drfile.o: ../../h/list_rchan.h
wr_q2drfile.o: ../../h/chan.h
wr_q2drfile.o: ../../h/table.h
wr_q2drfile.o: ../../h/list_bpt.h
wr_q2drfile.o: ../../h/auth.h
wr_q2drfile.o: ../../h/list_bpt.h
wr_q2drfile.o: ../../h/extension.h
wr_q2drfile.o: ../../h/mta.h
wr_q2drfile.o: ../../h/adr.h
wr_q2drfile.o: ../../h/list_bpt.h
wr_q2drfile.o: ../../h/aparse.h
wr_q2drfile.o: ../../h/ap.h
wr_q2drfile.o: ../../h/util.h
wr_q2drfile.o: ../../h/or.h
wr_q2drfile.o: ../../h/chan.h
wr_q2drfile.o: ../../h/auth.h
wr_q2drfile.o: ../../h/list_rchan.h
wr_q2drfile.o: ../../h/mta.h
wr_q2drfile.o: ../../h/dr.h
wr_q2drfile.o: ../../h/extension.h
wr_q2drfile.o: ../../h/mta.h
wr_q2drfile.o: ../../h/sys.file.h
wr_q2drfile.o: ../../h/util.h
wr_q2drfile.o: ../../h/list_bpt.h
wr_q2drfile.o: ../../h/or.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
