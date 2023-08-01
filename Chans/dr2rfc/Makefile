# PP Delivery Notification to RFC channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/dr2rfc/RCS/Makefile,v 6.0 1991/12/18 20:06:58 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:06:58  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  dr2rfc.c write_queue.c write_report.c
OBJS =  dr2rfc.o write_queue.o write_report.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a
CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS= ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xdr2rfc

default: $(PROGS)

dr2rfc: $(PROGS)

xdr2rfc: ${OBJS} $(LIBPP)
	${CC} ${LDFLAGS} ${OBJS} ${LIBPP} ${LIBSYS} -o $@

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

clean: tidy
	rm -f ${OBJS} $(PROGS) core a.out *.old *.BAK

tidy:
	rm -f $(PROGS) core a.out *old *.BAK zxdr2rfc

lint: l-dr2rfc
l-dr2rfc: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)


install: inst-dir inst-dr2rfc
inst-dr2rfc: $(CHANDIR)/dr2rfc
$(CHANDIR)/dr2rfc: xdr2rfc
	-$(BACKUP) $@ zxdr2rfc
	rm -f $@
	$(INSTALL) xdr2rfc $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "dr2rfc channel installed normally"; echo ""

inst-dir: $(CHANDIR)

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

############################################################
#
# End of Building Rules
#
############################################################


# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
dr2rfc.o: dr2rfc.c
dr2rfc.o: ../../h/head.h
dr2rfc.o: ../../h/util.h
dr2rfc.o: ../../h/config.h
dr2rfc.o: ../../h/ll_log.h
dr2rfc.o: ../../h/retcode.h
dr2rfc.o: ../../h/prm.h
dr2rfc.o: ../../h/qmgr.h
dr2rfc.o: ../../h/Qmgr-types.h
dr2rfc.o: ../../h/Qmgr-ops.h
dr2rfc.o: ../../h/q.h
dr2rfc.o: ../../h/adr.h
dr2rfc.o: ../../h/list_rchan.h
dr2rfc.o: ../../h/chan.h
dr2rfc.o: ../../h/table.h
dr2rfc.o: ../../h/list_bpt.h
dr2rfc.o: ../../h/auth.h
dr2rfc.o: ../../h/list_bpt.h
dr2rfc.o: ../../h/extension.h
dr2rfc.o: ../../h/mta.h
dr2rfc.o: ../../h/adr.h
dr2rfc.o: ../../h/list_bpt.h
dr2rfc.o: ../../h/aparse.h
dr2rfc.o: ../../h/ap.h
dr2rfc.o: ../../h/util.h
dr2rfc.o: ../../h/or.h
dr2rfc.o: ../../h/chan.h
dr2rfc.o: ../../h/auth.h
dr2rfc.o: ../../h/list_rchan.h
dr2rfc.o: ../../h/mta.h
dr2rfc.o: ../../h/tb_a.h
dr2rfc.o: ../../h/ap.h
write_queue.o: write_queue.c
write_queue.o: ../../h/head.h
write_queue.o: ../../h/util.h
write_queue.o: ../../h/config.h
write_queue.o: ../../h/ll_log.h
write_queue.o: ../../h/retcode.h
write_queue.o: ../../h/prm.h
write_queue.o: ../../h/q.h
write_queue.o: ../../h/adr.h
write_queue.o: ../../h/list_rchan.h
write_queue.o: ../../h/chan.h
write_queue.o: ../../h/table.h
write_queue.o: ../../h/list_bpt.h
write_queue.o: ../../h/auth.h
write_queue.o: ../../h/list_bpt.h
write_queue.o: ../../h/extension.h
write_queue.o: ../../h/mta.h
write_queue.o: ../../h/adr.h
write_queue.o: ../../h/list_bpt.h
write_queue.o: ../../h/aparse.h
write_queue.o: ../../h/ap.h
write_queue.o: ../../h/util.h
write_queue.o: ../../h/or.h
write_queue.o: ../../h/chan.h
write_queue.o: ../../h/auth.h
write_queue.o: ../../h/list_rchan.h
write_queue.o: ../../h/mta.h
write_queue.o: ../../h/tb_a.h
write_queue.o: ../../h/tb_q.h
write_queue.o: ../../h/tb_p1.h
write_report.o: write_report.c
write_report.o: ../../h/head.h
write_report.o: ../../h/util.h
write_report.o: ../../h/config.h
write_report.o: ../../h/ll_log.h
write_report.o: ../../h/retcode.h
write_report.o: ../../h/q.h
write_report.o: ../../h/adr.h
write_report.o: ../../h/list_rchan.h
write_report.o: ../../h/chan.h
write_report.o: ../../h/table.h
write_report.o: ../../h/list_bpt.h
write_report.o: ../../h/auth.h
write_report.o: ../../h/list_bpt.h
write_report.o: ../../h/extension.h
write_report.o: ../../h/mta.h
write_report.o: ../../h/adr.h
write_report.o: ../../h/list_bpt.h
write_report.o: ../../h/aparse.h
write_report.o: ../../h/ap.h
write_report.o: ../../h/util.h
write_report.o: ../../h/or.h
write_report.o: ../../h/chan.h
write_report.o: ../../h/auth.h
write_report.o: ../../h/list_rchan.h
write_report.o: ../../h/mta.h
write_report.o: ../../h/or.h
write_report.o: ../../h/dr.h
write_report.o: ../../h/extension.h
write_report.o: ../../h/mta.h
write_report.o: ../../h/tb_bpt84.h
write_report.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
