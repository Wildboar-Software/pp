# Submit - the pp submission agent
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/submit/RCS/Makefile,v 6.0 1991/12/18 20:28:02 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:02  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ad_mgt.c auth.c auth_logs.c auth_tb.c auth_ut.c ch_bind.c \
           gen_io2dr.c gen_ndr.c gen_probedr.c gen_warn.c \
	   rd_rfchdr.c submit.c submit_srvr.c\
           submit_chk.c submit_mgt.c submit_prm.c submit_que.c submit_txt.c
           
OBJS =  ad_mgt.o auth.o auth_logs.o auth_tb.o auth_ut.o ch_bind.o \
           gen_io2dr.o gen_ndr.o gen_probedr.o gen_warn.o \
	   rd_rfchdr.o submit.o submit_srvr.o \
           submit_chk.o submit_mgt.o submit_prm.o submit_que.o submit_txt.o
           
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I. -I$(HEADERS)
# CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
# CFLAGS          = $(CCOPTIONS) $(LCF) -I./h -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)

############################################################
#
# Building rules
#
############################################################

PROGS = xsubmit

default: $(PROGS)

xtrdrfc:     t-rd_rfchdr.o rd_rfchdr.o $(LIBPP)
	      $(CC) $(LDFLAGS) t-rd_rfchdr.o rd_rfchdr.o \
	      $(LIBPP) $(LIBSYS) -o $@

xt-trace:     t-tracecheck.o $(LIBPP)
	      $(CC) $(LDFLAGS) t-tracecheck.o $(LIBPP) \
	      $(LIBSYS) -o $@

AUTHOBJS	= t-auth.o auth.o auth_logs.o auth_tb.o auth_ut.o ch_bind.o
xt-auth:	$(AUTHOBJS) $(LIBPP)
		$(CC) $(LDFLAGS) -o $@ $(AUTHOBJS) $(LIBPP) $(LIBSYS)

xsubmit: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBPP) $(LIBSYS) -o xsubmit

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lint: l-submit
l-submit: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean: tidy
	rm -f $(OBJS) *.o

tidy:
	rm -f zxsubmit xsubmit core a.out *.old *.BAK
	rm -f xtdrfc xt-trace t-rd_rfchdr.o t-tracecheck.o


install: inst-dir inst-submit
inst-submit: $(CMDDIR)/submit
$(CMDDIR)/submit: xsubmit
	-$(BACKUP) $@ zxsubmit
	rm -f $@
	$(INSTALL) xsubmit $@
	-$(CHOWN) $(PPUSER) $@
	-$(CHMOD) 4755 $@
	-@ls -ls $@
	-@echo "submit installed normally"; echo ""

inst-dir: $(CMDDIR)


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
ad_mgt.o: ad_mgt.c
ad_mgt.o: ../../h/util.h
ad_mgt.o: ../../h/config.h
ad_mgt.o: ../../h/ll_log.h
ad_mgt.o: ../../h/retcode.h
ad_mgt.o: ../../h/q.h
ad_mgt.o: ../../h/adr.h
ad_mgt.o: ../../h/list_rchan.h
ad_mgt.o: ../../h/chan.h
ad_mgt.o: ../../h/table.h
ad_mgt.o: ../../h/list_bpt.h
ad_mgt.o: ../../h/auth.h
ad_mgt.o: ../../h/list_bpt.h
ad_mgt.o: ../../h/extension.h
ad_mgt.o: ../../h/mta.h
ad_mgt.o: ../../h/adr.h
ad_mgt.o: ../../h/list_bpt.h
ad_mgt.o: ../../h/aparse.h
ad_mgt.o: ../../h/ap.h
ad_mgt.o: ../../h/util.h
ad_mgt.o: ../../h/or.h
ad_mgt.o: ../../h/chan.h
ad_mgt.o: ../../h/auth.h
ad_mgt.o: ../../h/list_rchan.h
ad_mgt.o: ../../h/mta.h
ad_mgt.o: ../../h/dr.h
ad_mgt.o: ../../h/extension.h
ad_mgt.o: ../../h/mta.h
ad_mgt.o: ../../h/ap.h
auth.o: auth.c
auth.o: ../../h/head.h
auth.o: ../../h/util.h
auth.o: ../../h/config.h
auth.o: ../../h/ll_log.h
auth.o: ../../h/retcode.h
auth.o: ../../h/q.h
auth.o: ../../h/adr.h
auth.o: ../../h/list_rchan.h
auth.o: ../../h/chan.h
auth.o: ../../h/table.h
auth.o: ../../h/list_bpt.h
auth.o: ../../h/auth.h
auth.o: ../../h/list_bpt.h
auth.o: ../../h/extension.h
auth.o: ../../h/mta.h
auth.o: ../../h/adr.h
auth.o: ../../h/list_bpt.h
auth.o: ../../h/aparse.h
auth.o: ../../h/ap.h
auth.o: ../../h/util.h
auth.o: ../../h/or.h
auth.o: ../../h/chan.h
auth.o: ../../h/auth.h
auth.o: ../../h/list_rchan.h
auth.o: ../../h/mta.h
auth.o: ../../h/dr.h
auth.o: ../../h/extension.h
auth.o: ../../h/mta.h
auth.o: ../../h/tb_auth.h
auth_logs.o: auth_logs.c
auth_logs.o: ../../h/util.h
auth_logs.o: ../../h/config.h
auth_logs.o: ../../h/ll_log.h
auth_logs.o: ../../h/q.h
auth_logs.o: ../../h/adr.h
auth_logs.o: ../../h/list_rchan.h
auth_logs.o: ../../h/chan.h
auth_logs.o: ../../h/table.h
auth_logs.o: ../../h/list_bpt.h
auth_logs.o: ../../h/auth.h
auth_logs.o: ../../h/list_bpt.h
auth_logs.o: ../../h/extension.h
auth_logs.o: ../../h/mta.h
auth_logs.o: ../../h/adr.h
auth_logs.o: ../../h/list_bpt.h
auth_logs.o: ../../h/aparse.h
auth_logs.o: ../../h/ap.h
auth_logs.o: ../../h/util.h
auth_logs.o: ../../h/or.h
auth_logs.o: ../../h/chan.h
auth_logs.o: ../../h/auth.h
auth_logs.o: ../../h/list_rchan.h
auth_logs.o: ../../h/mta.h
auth_tb.o: auth_tb.c
auth_tb.o: ../../h/util.h
auth_tb.o: ../../h/config.h
auth_tb.o: ../../h/ll_log.h
auth_tb.o: ../../h/q.h
auth_tb.o: ../../h/adr.h
auth_tb.o: ../../h/list_rchan.h
auth_tb.o: ../../h/chan.h
auth_tb.o: ../../h/table.h
auth_tb.o: ../../h/list_bpt.h
auth_tb.o: ../../h/auth.h
auth_tb.o: ../../h/list_bpt.h
auth_tb.o: ../../h/extension.h
auth_tb.o: ../../h/mta.h
auth_tb.o: ../../h/adr.h
auth_tb.o: ../../h/list_bpt.h
auth_tb.o: ../../h/aparse.h
auth_tb.o: ../../h/ap.h
auth_tb.o: ../../h/util.h
auth_tb.o: ../../h/or.h
auth_tb.o: ../../h/chan.h
auth_tb.o: ../../h/auth.h
auth_tb.o: ../../h/list_rchan.h
auth_tb.o: ../../h/mta.h
auth_tb.o: ../../h/tb_auth.h
auth_ut.o: auth_ut.c
auth_ut.o: ../../h/util.h
auth_ut.o: ../../h/config.h
auth_ut.o: ../../h/ll_log.h
auth_ut.o: ../../h/q.h
auth_ut.o: ../../h/adr.h
auth_ut.o: ../../h/list_rchan.h
auth_ut.o: ../../h/chan.h
auth_ut.o: ../../h/table.h
auth_ut.o: ../../h/list_bpt.h
auth_ut.o: ../../h/auth.h
auth_ut.o: ../../h/list_bpt.h
auth_ut.o: ../../h/extension.h
auth_ut.o: ../../h/mta.h
auth_ut.o: ../../h/adr.h
auth_ut.o: ../../h/list_bpt.h
auth_ut.o: ../../h/aparse.h
auth_ut.o: ../../h/ap.h
auth_ut.o: ../../h/util.h
auth_ut.o: ../../h/or.h
auth_ut.o: ../../h/chan.h
auth_ut.o: ../../h/auth.h
auth_ut.o: ../../h/list_rchan.h
auth_ut.o: ../../h/mta.h
ch_bind.o: ch_bind.c
ch_bind.o: ../../h/head.h
ch_bind.o: ../../h/util.h
ch_bind.o: ../../h/config.h
ch_bind.o: ../../h/ll_log.h
ch_bind.o: ../../h/retcode.h
ch_bind.o: ../../h/q.h
ch_bind.o: ../../h/adr.h
ch_bind.o: ../../h/list_rchan.h
ch_bind.o: ../../h/chan.h
ch_bind.o: ../../h/table.h
ch_bind.o: ../../h/list_bpt.h
ch_bind.o: ../../h/auth.h
ch_bind.o: ../../h/list_bpt.h
ch_bind.o: ../../h/extension.h
ch_bind.o: ../../h/mta.h
ch_bind.o: ../../h/adr.h
ch_bind.o: ../../h/list_bpt.h
ch_bind.o: ../../h/aparse.h
ch_bind.o: ../../h/ap.h
ch_bind.o: ../../h/util.h
ch_bind.o: ../../h/or.h
ch_bind.o: ../../h/chan.h
ch_bind.o: ../../h/auth.h
ch_bind.o: ../../h/list_rchan.h
ch_bind.o: ../../h/mta.h
ch_bind.o: ../../h/ch_bind.h
gen_io2dr.o: gen_io2dr.c
gen_io2dr.o: ../../h/head.h
gen_io2dr.o: ../../h/util.h
gen_io2dr.o: ../../h/config.h
gen_io2dr.o: ../../h/ll_log.h
gen_io2dr.o: ../../h/retcode.h
gen_io2dr.o: ../../h/q.h
gen_io2dr.o: ../../h/adr.h
gen_io2dr.o: ../../h/list_rchan.h
gen_io2dr.o: ../../h/chan.h
gen_io2dr.o: ../../h/table.h
gen_io2dr.o: ../../h/list_bpt.h
gen_io2dr.o: ../../h/auth.h
gen_io2dr.o: ../../h/list_bpt.h
gen_io2dr.o: ../../h/extension.h
gen_io2dr.o: ../../h/mta.h
gen_io2dr.o: ../../h/adr.h
gen_io2dr.o: ../../h/list_bpt.h
gen_io2dr.o: ../../h/aparse.h
gen_io2dr.o: ../../h/ap.h
gen_io2dr.o: ../../h/util.h
gen_io2dr.o: ../../h/or.h
gen_io2dr.o: ../../h/chan.h
gen_io2dr.o: ../../h/auth.h
gen_io2dr.o: ../../h/list_rchan.h
gen_io2dr.o: ../../h/mta.h
gen_io2dr.o: ../../h/dr.h
gen_io2dr.o: ../../h/extension.h
gen_io2dr.o: ../../h/mta.h
gen_ndr.o: gen_ndr.c
gen_ndr.o: ../../h/head.h
gen_ndr.o: ../../h/util.h
gen_ndr.o: ../../h/config.h
gen_ndr.o: ../../h/ll_log.h
gen_ndr.o: ../../h/retcode.h
gen_ndr.o: ../../h/q.h
gen_ndr.o: ../../h/adr.h
gen_ndr.o: ../../h/list_rchan.h
gen_ndr.o: ../../h/chan.h
gen_ndr.o: ../../h/table.h
gen_ndr.o: ../../h/list_bpt.h
gen_ndr.o: ../../h/auth.h
gen_ndr.o: ../../h/list_bpt.h
gen_ndr.o: ../../h/extension.h
gen_ndr.o: ../../h/mta.h
gen_ndr.o: ../../h/adr.h
gen_ndr.o: ../../h/list_bpt.h
gen_ndr.o: ../../h/aparse.h
gen_ndr.o: ../../h/ap.h
gen_ndr.o: ../../h/util.h
gen_ndr.o: ../../h/or.h
gen_ndr.o: ../../h/chan.h
gen_ndr.o: ../../h/auth.h
gen_ndr.o: ../../h/list_rchan.h
gen_ndr.o: ../../h/mta.h
gen_ndr.o: ../../h/dr.h
gen_ndr.o: ../../h/extension.h
gen_ndr.o: ../../h/mta.h
gen_probedr.o: gen_probedr.c
gen_probedr.o: ../../h/head.h
gen_probedr.o: ../../h/util.h
gen_probedr.o: ../../h/config.h
gen_probedr.o: ../../h/ll_log.h
gen_probedr.o: ../../h/retcode.h
gen_probedr.o: ../../h/q.h
gen_probedr.o: ../../h/adr.h
gen_probedr.o: ../../h/list_rchan.h
gen_probedr.o: ../../h/chan.h
gen_probedr.o: ../../h/table.h
gen_probedr.o: ../../h/list_bpt.h
gen_probedr.o: ../../h/auth.h
gen_probedr.o: ../../h/list_bpt.h
gen_probedr.o: ../../h/extension.h
gen_probedr.o: ../../h/mta.h
gen_probedr.o: ../../h/adr.h
gen_probedr.o: ../../h/list_bpt.h
gen_probedr.o: ../../h/aparse.h
gen_probedr.o: ../../h/ap.h
gen_probedr.o: ../../h/util.h
gen_probedr.o: ../../h/or.h
gen_probedr.o: ../../h/chan.h
gen_probedr.o: ../../h/auth.h
gen_probedr.o: ../../h/list_rchan.h
gen_probedr.o: ../../h/mta.h
gen_probedr.o: ../../h/dr.h
gen_probedr.o: ../../h/extension.h
gen_probedr.o: ../../h/mta.h
gen_warn.o: gen_warn.c
gen_warn.o: ../../h/util.h
gen_warn.o: ../../h/config.h
gen_warn.o: ../../h/ll_log.h
gen_warn.o: ../../h/q.h
gen_warn.o: ../../h/adr.h
gen_warn.o: ../../h/list_rchan.h
gen_warn.o: ../../h/chan.h
gen_warn.o: ../../h/table.h
gen_warn.o: ../../h/list_bpt.h
gen_warn.o: ../../h/auth.h
gen_warn.o: ../../h/list_bpt.h
gen_warn.o: ../../h/extension.h
gen_warn.o: ../../h/mta.h
gen_warn.o: ../../h/adr.h
gen_warn.o: ../../h/list_bpt.h
gen_warn.o: ../../h/aparse.h
gen_warn.o: ../../h/ap.h
gen_warn.o: ../../h/util.h
gen_warn.o: ../../h/or.h
gen_warn.o: ../../h/chan.h
gen_warn.o: ../../h/auth.h
gen_warn.o: ../../h/list_rchan.h
gen_warn.o: ../../h/mta.h
gen_warn.o: ../../h/auth.h
gen_warn.o: ../../h/prm.h
gen_warn.o: ../../h/retcode.h
rd_rfchdr.o: rd_rfchdr.c
rd_rfchdr.o: ../../h/head.h
rd_rfchdr.o: ../../h/util.h
rd_rfchdr.o: ../../h/config.h
rd_rfchdr.o: ../../h/ll_log.h
rd_rfchdr.o: ../../h/retcode.h
rd_rfchdr.o: ../../h/q.h
rd_rfchdr.o: ../../h/adr.h
rd_rfchdr.o: ../../h/list_rchan.h
rd_rfchdr.o: ../../h/chan.h
rd_rfchdr.o: ../../h/table.h
rd_rfchdr.o: ../../h/list_bpt.h
rd_rfchdr.o: ../../h/auth.h
rd_rfchdr.o: ../../h/list_bpt.h
rd_rfchdr.o: ../../h/extension.h
rd_rfchdr.o: ../../h/mta.h
rd_rfchdr.o: ../../h/adr.h
rd_rfchdr.o: ../../h/list_bpt.h
rd_rfchdr.o: ../../h/aparse.h
rd_rfchdr.o: ../../h/ap.h
rd_rfchdr.o: ../../h/util.h
rd_rfchdr.o: ../../h/or.h
rd_rfchdr.o: ../../h/chan.h
rd_rfchdr.o: ../../h/auth.h
rd_rfchdr.o: ../../h/list_rchan.h
rd_rfchdr.o: ../../h/mta.h
rd_rfchdr.o: ../../h/tb_q.h
rd_rfchdr.o: ../../h/or.h
rd_rfchdr.o: ../../h/ap.h
submit.o: submit.c
submit.o: ../../h/head.h
submit.o: ../../h/util.h
submit.o: ../../h/config.h
submit.o: ../../h/ll_log.h
submit.o: ../../h/retcode.h
submit.o: ../../h/prm.h
submit.o: ../../h/q.h
submit.o: ../../h/adr.h
submit.o: ../../h/list_rchan.h
submit.o: ../../h/chan.h
submit.o: ../../h/table.h
submit.o: ../../h/list_bpt.h
submit.o: ../../h/auth.h
submit.o: ../../h/list_bpt.h
submit.o: ../../h/extension.h
submit.o: ../../h/mta.h
submit.o: ../../h/adr.h
submit.o: ../../h/list_bpt.h
submit.o: ../../h/aparse.h
submit.o: ../../h/ap.h
submit.o: ../../h/util.h
submit.o: ../../h/or.h
submit.o: ../../h/chan.h
submit.o: ../../h/auth.h
submit.o: ../../h/list_rchan.h
submit.o: ../../h/mta.h
submit.o: ../../h/or.h
submit_chk.o: submit_chk.c
submit_chk.o: ../../h/head.h
submit_chk.o: ../../h/util.h
submit_chk.o: ../../h/config.h
submit_chk.o: ../../h/ll_log.h
submit_chk.o: ../../h/retcode.h
submit_chk.o: ../../h/q.h
submit_chk.o: ../../h/adr.h
submit_chk.o: ../../h/list_rchan.h
submit_chk.o: ../../h/chan.h
submit_chk.o: ../../h/table.h
submit_chk.o: ../../h/list_bpt.h
submit_chk.o: ../../h/auth.h
submit_chk.o: ../../h/list_bpt.h
submit_chk.o: ../../h/extension.h
submit_chk.o: ../../h/mta.h
submit_chk.o: ../../h/adr.h
submit_chk.o: ../../h/list_bpt.h
submit_chk.o: ../../h/aparse.h
submit_chk.o: ../../h/ap.h
submit_chk.o: ../../h/util.h
submit_chk.o: ../../h/or.h
submit_chk.o: ../../h/chan.h
submit_chk.o: ../../h/auth.h
submit_chk.o: ../../h/list_rchan.h
submit_chk.o: ../../h/mta.h
submit_chk.o: ../../h/dr.h
submit_chk.o: ../../h/extension.h
submit_chk.o: ../../h/mta.h
submit_mgt.o: submit_mgt.c
submit_mgt.o: ../../h/head.h
submit_mgt.o: ../../h/util.h
submit_mgt.o: ../../h/config.h
submit_mgt.o: ../../h/ll_log.h
submit_mgt.o: ../../h/retcode.h
submit_mgt.o: ../../h/prm.h
submit_mgt.o: ../../h/q.h
submit_mgt.o: ../../h/adr.h
submit_mgt.o: ../../h/list_rchan.h
submit_mgt.o: ../../h/chan.h
submit_mgt.o: ../../h/table.h
submit_mgt.o: ../../h/list_bpt.h
submit_mgt.o: ../../h/auth.h
submit_mgt.o: ../../h/list_bpt.h
submit_mgt.o: ../../h/extension.h
submit_mgt.o: ../../h/mta.h
submit_mgt.o: ../../h/adr.h
submit_mgt.o: ../../h/list_bpt.h
submit_mgt.o: ../../h/aparse.h
submit_mgt.o: ../../h/ap.h
submit_mgt.o: ../../h/util.h
submit_mgt.o: ../../h/or.h
submit_mgt.o: ../../h/chan.h
submit_mgt.o: ../../h/auth.h
submit_mgt.o: ../../h/list_rchan.h
submit_mgt.o: ../../h/mta.h
submit_mgt.o: ../../h/dr.h
submit_mgt.o: ../../h/extension.h
submit_mgt.o: ../../h/mta.h
submit_mgt.o: ../../h/ap.h
submit_prm.o: submit_prm.c
submit_prm.o: ../../h/head.h
submit_prm.o: ../../h/util.h
submit_prm.o: ../../h/config.h
submit_prm.o: ../../h/ll_log.h
submit_prm.o: ../../h/retcode.h
submit_prm.o: ../../h/prm.h
submit_prm.o: ../../h/q.h
submit_prm.o: ../../h/adr.h
submit_prm.o: ../../h/list_rchan.h
submit_prm.o: ../../h/chan.h
submit_prm.o: ../../h/table.h
submit_prm.o: ../../h/list_bpt.h
submit_prm.o: ../../h/auth.h
submit_prm.o: ../../h/list_bpt.h
submit_prm.o: ../../h/extension.h
submit_prm.o: ../../h/mta.h
submit_prm.o: ../../h/adr.h
submit_prm.o: ../../h/list_bpt.h
submit_prm.o: ../../h/aparse.h
submit_prm.o: ../../h/ap.h
submit_prm.o: ../../h/util.h
submit_prm.o: ../../h/or.h
submit_prm.o: ../../h/chan.h
submit_prm.o: ../../h/auth.h
submit_prm.o: ../../h/list_rchan.h
submit_prm.o: ../../h/mta.h
submit_que.o: submit_que.c
submit_que.o: ../../h/head.h
submit_que.o: ../../h/util.h
submit_que.o: ../../h/config.h
submit_que.o: ../../h/ll_log.h
submit_que.o: ../../h/retcode.h
submit_que.o: ../../h/prm.h
submit_que.o: ../../h/q.h
submit_que.o: ../../h/adr.h
submit_que.o: ../../h/list_rchan.h
submit_que.o: ../../h/chan.h
submit_que.o: ../../h/table.h
submit_que.o: ../../h/list_bpt.h
submit_que.o: ../../h/auth.h
submit_que.o: ../../h/list_bpt.h
submit_que.o: ../../h/extension.h
submit_que.o: ../../h/mta.h
submit_que.o: ../../h/adr.h
submit_que.o: ../../h/list_bpt.h
submit_que.o: ../../h/aparse.h
submit_que.o: ../../h/ap.h
submit_que.o: ../../h/util.h
submit_que.o: ../../h/or.h
submit_que.o: ../../h/chan.h
submit_que.o: ../../h/auth.h
submit_que.o: ../../h/list_rchan.h
submit_que.o: ../../h/mta.h
submit_que.o: ../../h/adr.h
submit_txt.o: submit_txt.c
submit_txt.o: ../../h/head.h
submit_txt.o: ../../h/util.h
submit_txt.o: ../../h/config.h
submit_txt.o: ../../h/ll_log.h
submit_txt.o: ../../h/retcode.h
submit_txt.o: ../../h/list_bpt.h
submit_txt.o: ../../h/q.h
submit_txt.o: ../../h/adr.h
submit_txt.o: ../../h/list_rchan.h
submit_txt.o: ../../h/chan.h
submit_txt.o: ../../h/table.h
submit_txt.o: ../../h/list_bpt.h
submit_txt.o: ../../h/auth.h
submit_txt.o: ../../h/list_bpt.h
submit_txt.o: ../../h/extension.h
submit_txt.o: ../../h/mta.h
submit_txt.o: ../../h/adr.h
submit_txt.o: ../../h/list_bpt.h
submit_txt.o: ../../h/aparse.h
submit_txt.o: ../../h/ap.h
submit_txt.o: ../../h/util.h
submit_txt.o: ../../h/or.h
submit_txt.o: ../../h/chan.h
submit_txt.o: ../../h/auth.h
submit_txt.o: ../../h/list_rchan.h
submit_txt.o: ../../h/mta.h
submit_txt.o: ../../h/chan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
