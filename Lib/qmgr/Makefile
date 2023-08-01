# Makefile to build the 822 addressing stuff
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/Makefile,v 6.0 1991/12/18 20:23:58 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:23:58  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =	Qmgr-ops.c Qmgr_tables.c chan_control.c submit2qmgr.c \
	chan_debug.c qstruct2qmgr.c recipstate.c \
	comms.c consblk.c cons-misc.c op_chan.c op_msg.c \
	op_mta.c op_que.c order.c op_filter.c
OBJS =	Qmgr-ops.o chan_control.o \
	   chan_debug.o qstruct2qmgr.o recipstate.o \
	   submit2qmgr.o Qmgr_tables.o \
	comms.o consblk.o cons-misc.o op_chan.o op_msg.o \
	op_mta.o op_que.o order.o op_filter.o

HEADERS		= ../../h

CFLAGS		= $(LIBCCOPTIONS) -I$(HEADERS) $(LCF) 
LLFLAGS		= $(LINTFLAGS) -I$(HEADERS) -u
SHADOW		= qmgr-made

LINTLIBS	= ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
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

Qmgr-ops.c Qmgr-ops.h Qmgr-asn.py: qmgr.ry
	$(ROSY) -pepsy $(RYFLAGS) -o $@ qmgr.ry
	-cmp -s Qmgr-ops.h $(HEADERS)/Qmgr-ops.h || cp Qmgr-ops.h $(HEADERS)

Qmgr-types.h: Qmgr_tables.c

Qmgr_tables.o: Qmgr_tables.c

Qmgr_tables.c Qmgr-types.h: Qmgr-asn.py 
	$(PEPSY) $(POFLAGS) -A -h -f Qmgr-asn.py
	-cmp -s Qmgr-types.h $(HEADERS)/Qmgr-types.h || \
		cp Qmgr-types.h $(HEADERS)/Qmgr-types.h
	-if [ -f Qmgr_defs.h ]; \
	then cmp -s Qmgr_defs.h $(HEADERS)/Qmgr_defs.h || \
		cp Qmgr_defs.h $(HEADERS)/Qmgr_defs.h; \
	fi
	-if [ -f Qmgr_pre_defs.h ]; \
	then cmp -s Qmgr_pre_defs.h $(HEADERS)/Qmgr_pre_defs.h || \
		cp Qmgr_pre_defs.h $(HEADERS)/Qmgr_pre_defs.h; \
	fi

Qmgr-ops.o: Qmgr-ops.c
	$(CC) $(CFLAGS) -DPERFORMER -DINVOKER -c Qmgr-ops.c

lintlib: llib-lqmgr.ln
llib-lqmgr.ln: $(SRCS)
	$(LINT) -Cqmgr $(LLFLAGS) $(SRCS)

install:;

lint: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

clean: tidy
	rm -f $(OBJS) QMGR-*  llib-lqmgr.ln
	rm -f Qmgr-asn.py Qmgr-ops.c Qmgr-ops.h Qmgr-ops.o
	rm -f Qmgr-stubs.c Qmgr-stubs.o Qmgr-types.h Qmgr-types.py
	rm -f Qmgr_*
	rm -f $(HEADERS)/Qmgr-ops.h $(HEADERS)/Qmgr-types.h \
		$(HEADERS)/Qmgr_defs.h $(HEADERS)/Qmgr_pre_defs.h 

tidy:
	rm -f core $(SHADOW) *.old *.BAK *.ph

define:
	$(DEFINE) Makefile

depend: $(SRCS)
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of building rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
Qmgr-ops.o: Qmgr-ops.c
Qmgr-ops.o: Qmgr-types.h
Qmgr-ops.o: Qmgr-ops.h
Qmgr_tables.o: Qmgr_tables.c
Qmgr_tables.o: Qmgr-types.h
chan_control.o: chan_control.c
chan_control.o: ../../h/qmgr.h
chan_control.o: ../../h/Qmgr-types.h
chan_control.o: ../../h/Qmgr-ops.h
chan_control.o: ../../h/retcode.h
chan_control.o: ../../h/ll_log.h
submit2qmgr.o: submit2qmgr.c
submit2qmgr.o: ../../h/head.h
submit2qmgr.o: ../../h/util.h
submit2qmgr.o: ../../h/config.h
submit2qmgr.o: ../../h/ll_log.h
submit2qmgr.o: ../../h/retcode.h
submit2qmgr.o: ../../h/qmgr.h
submit2qmgr.o: ../../h/Qmgr-types.h
submit2qmgr.o: ../../h/Qmgr-ops.h
submit2qmgr.o: ../../h/q.h
submit2qmgr.o: ../../h/adr.h
submit2qmgr.o: ../../h/list_rchan.h
submit2qmgr.o: ../../h/chan.h
submit2qmgr.o: ../../h/table.h
submit2qmgr.o: ../../h/list_bpt.h
submit2qmgr.o: ../../h/auth.h
submit2qmgr.o: ../../h/list_bpt.h
submit2qmgr.o: ../../h/extension.h
submit2qmgr.o: ../../h/mta.h
submit2qmgr.o: ../../h/adr.h
submit2qmgr.o: ../../h/list_bpt.h
submit2qmgr.o: ../../h/aparse.h
submit2qmgr.o: ../../h/ap.h
submit2qmgr.o: ../../h/util.h
submit2qmgr.o: ../../h/or.h
submit2qmgr.o: ../../h/chan.h
submit2qmgr.o: ../../h/auth.h
submit2qmgr.o: ../../h/list_rchan.h
submit2qmgr.o: ../../h/mta.h
submit2qmgr.o: ../../h/prm.h
chan_debug.o: chan_debug.c
chan_debug.o: ../../h/util.h
chan_debug.o: ../../h/config.h
chan_debug.o: ../../h/ll_log.h
chan_debug.o: ../../h/qmgr.h
chan_debug.o: ../../h/Qmgr-types.h
chan_debug.o: ../../h/Qmgr-ops.h
qstruct2qmgr.o: qstruct2qmgr.c
qstruct2qmgr.o: ../../h/util.h
qstruct2qmgr.o: ../../h/config.h
qstruct2qmgr.o: ../../h/ll_log.h
qstruct2qmgr.o: Qmgr-types.h
qstruct2qmgr.o: ../../h/q.h
qstruct2qmgr.o: ../../h/adr.h
qstruct2qmgr.o: ../../h/list_rchan.h
qstruct2qmgr.o: ../../h/chan.h
qstruct2qmgr.o: ../../h/table.h
qstruct2qmgr.o: ../../h/list_bpt.h
qstruct2qmgr.o: ../../h/auth.h
qstruct2qmgr.o: ../../h/list_bpt.h
qstruct2qmgr.o: ../../h/extension.h
qstruct2qmgr.o: ../../h/mta.h
qstruct2qmgr.o: ../../h/adr.h
qstruct2qmgr.o: ../../h/list_bpt.h
qstruct2qmgr.o: ../../h/aparse.h
qstruct2qmgr.o: ../../h/ap.h
qstruct2qmgr.o: ../../h/util.h
qstruct2qmgr.o: ../../h/or.h
qstruct2qmgr.o: ../../h/chan.h
qstruct2qmgr.o: ../../h/auth.h
qstruct2qmgr.o: ../../h/list_rchan.h
qstruct2qmgr.o: ../../h/mta.h
qstruct2qmgr.o: ../../h/prm.h
recipstate.o: recipstate.c
recipstate.o: ../../h/util.h
recipstate.o: ../../h/config.h
recipstate.o: ../../h/ll_log.h
recipstate.o: ../../h/qmgr.h
recipstate.o: ../../h/Qmgr-types.h
recipstate.o: ../../h/Qmgr-ops.h
comms.o: comms.c
comms.o: ../../h/util.h
comms.o: ../../h/config.h
comms.o: ../../h/ll_log.h
comms.o: ../../h/qmgr-int.h
comms.o: Qmgr-types.h
comms.o: Qmgr-ops.h
comms.o: ../../h/qmgr.h
comms.o: ../../h/Qmgr-types.h
comms.o: ../../h/Qmgr-ops.h
comms.o: consblk.h
consblk.o: consblk.c
consblk.o: ../../h/util.h
consblk.o: ../../h/config.h
consblk.o: ../../h/ll_log.h
consblk.o: consblk.h
cons-misc.o: cons-misc.c
cons-misc.o: ../../h/util.h
cons-misc.o: ../../h/config.h
cons-misc.o: ../../h/ll_log.h
cons-misc.o: ../../h/qmgr-int.h
cons-misc.o: Qmgr-types.h
op_chan.o: op_chan.c
op_chan.o: ../../h/util.h
op_chan.o: ../../h/config.h
op_chan.o: ../../h/ll_log.h
op_chan.o: ../../h/qmgr-int.h
op_chan.o: consblk.h
op_chan.o: Qmgr-types.h
op_msg.o: op_msg.c
op_msg.o: ../../h/util.h
op_msg.o: ../../h/config.h
op_msg.o: ../../h/ll_log.h
op_msg.o: ../../h/qmgr-int.h
op_msg.o: consblk.h
op_msg.o: Qmgr-types.h
op_mta.o: op_mta.c
op_mta.o: ../../h/util.h
op_mta.o: ../../h/config.h
op_mta.o: ../../h/ll_log.h
op_mta.o: ../../h/qmgr-int.h
op_mta.o: consblk.h
op_mta.o: Qmgr-types.h
op_que.o: op_que.c
op_que.o: ../../h/util.h
op_que.o: ../../h/config.h
op_que.o: ../../h/ll_log.h
op_que.o: consblk.h
op_que.o: ../../h/qmgr-int.h
op_que.o: Qmgr-types.h
order.o: order.c
order.o: ../../h/util.h
order.o: ../../h/config.h
order.o: ../../h/ll_log.h
order.o: ../../h/qmgr-int.h
op_filter.o: op_filter.c
op_filter.o: ../../h/util.h
op_filter.o: ../../h/config.h
op_filter.o: ../../h/ll_log.h
op_filter.o: ../../h/qmgr-int.h
op_filter.o: consblk.h
op_filter.o: Qmgr-types.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
