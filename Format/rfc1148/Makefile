# 1148 stuff
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/rfc1148/RCS/Makefile,v 6.0 1991/12/18 20:20:34 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:20:34  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= P2toRFC.c c-P2toRFC.c t-P2toRFC.c \
	RFCtoP2.c c-RFCtoP2.c t-RFCtoP2.c 
OBJS	= P2toRFC.o c-P2toRFC.o t-P2toRFC.o \
	RFCtoP2.o c-RFCtoP2.o t-RFCtoP2.o 
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Suffix rules
#
############################################################

.SUFFIXES: .py

.py.c:
	$(PEPY) -P -a PY_advise $(PYFLAGS) $<
.py.o:
	$(PEPY) -P $(PYFLAGS) -a PY_advise $<
	$(CC) $(CFLAGS) -c $*.c
	rm $*.c

############################################################
#
# Building Rules
#
############################################################

PROGS = xc-P2toRFC xc-RFCtoP2 

default:	   $(SRCS) $(PROGS)

xt-P2toRFC: t-P2toRFC.o P2toRFC.o $(LIBPP)
	$(CC) $(LDFLAGS) P2toRFC.o \
	$(LIBPP) $(LIBSYS) -o $@

xc-P2toRFC: c-P2toRFC.o P2toRFC.o $(LIBPP)
	$(CC) $(LDFLAGS) c-P2toRFC.o P2toRFC.o \
	$(LIBPP) $(LIBSYS) -o $@

xt-RFCtoP2: t-RFCtoP2.o RFCtoP2.o rfc-hdr.h $(LIBPP)
	$(CC) $(LDFLAGS) t-RFCtoP2.o RFCtoP2.o \
	$(LIBPP) $(LIBSYS) -o $@

xc-RFCtoP2: c-RFCtoP2.o RFCtoP2.o rfc-hdr.h $(LIBPP)
	$(CC) $(LDFLAGS) c-RFCtoP2.o RFCtoP2.o \
	$(LIBPP) $(LIBSYS) -o $@

install: inst-c-RFCtoP2 inst-c-P2toRFC

inst-c-P2toRFC: $(CHANDIR)/P2toRFC
$(CHANDIR)/P2toRFC: xc-P2toRFC
	-$(BACKUP) $@ zc-P2toRFC
	rm -f $@
	$(INSTALL) xc-P2toRFC $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "P2 to RFC (1138) channel installed normally"; echo ""

inst-c-RFCtoP2: $(CHANDIR)/RFCtoP2
$(CHANDIR)/RFCtoP2: xc-RFCtoP2
	-$(BACKUP) $@ zc-RFCtoP2
	rm -f $@
	$(INSTALL) xc-RFCtoP2 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "RFC to P2 (1138) channel installed normally"; echo ""

clean:	tidy
	rm -f $(OBJS) p2.c

tidy:
	rm -f $(PROGS) zc-* P2-types* P2.ph core a.out \
		Makefile.old t-RFCtoP2 t-P2toRFC xt-*

lint: l-P2toRFC l-RFCtoP2
l-P2toRFC: P2toRFC.c c-P2toRFC.c t-P2toRFC.c
	   $(LINT) $(LLFLAGS) P2toRFC.c c-P2toRFC.c t-P2toRFC.c \
	   $(LINTLIBS)
l-RFCtoP2: RFCtoP2.c c-RFCtoP2.c t-RFCtoP2.c rfc-hdr.h
	   $(LINT) $(LLFLAGS) RFCtoP2.c c-RFCtoP2.c t-RFCtoP2.c \
	   $(LINTLIBS) 

saber_src_p2torfc: c-P2toRFC.c P2toRFC.c
	#load -C $(CFLAGS) c-P2toRFC.c P2toRFC.c $(LIBPP) $(LIBSYS)

saber_obj_p2torfc: c-P2toRFC.o P2toRFC.o
	#load -C $(CFLAGS) c-P2toRFC.o P2toRFC.o $(LIBPP) $(LIBSYS)

saber_src_rfctop2: c-RFCtoP2.c RFCtoP2.c
	#load -C $(CFLAGS) c-RFCtoP2.c RFCtoP2.c $(LIBPP) $(LIBSYS)

saber_obj_rfctop2: c-RFCtoP2.o RFCtoP2.o
	#load -C $(CFLAGS) c-RFCtoP2.o RFCtoP2.o $(LIBPP) $(LIBSYS)

define:
	$(DEFINE) Makefile 
depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
P2toRFC.o: P2toRFC.c
P2toRFC.o: ../../h/head.h
P2toRFC.o: ../../h/util.h
P2toRFC.o: ../../h/config.h
P2toRFC.o: ../../h/ll_log.h
P2toRFC.o: ../../h/retcode.h
P2toRFC.o: ../../h/util.h
P2toRFC.o: ../../h/IOB-types.h
P2toRFC.o: ../../h/MTA-types.h
P2toRFC.o: ../../h/Toks-types.h
P2toRFC.o: ../../h/Auth-types.h
P2toRFC.o: ../../h/Ext-types.h
P2toRFC.o: ../../h/MTA-types.h
P2toRFC.o: ../../h/Toks-types.h
P2toRFC.o: ../../h/Auth-types.h
P2toRFC.o: ../../h/q.h
P2toRFC.o: ../../h/adr.h
P2toRFC.o: ../../h/list_rchan.h
P2toRFC.o: ../../h/chan.h
P2toRFC.o: ../../h/table.h
P2toRFC.o: ../../h/list_bpt.h
P2toRFC.o: ../../h/auth.h
P2toRFC.o: ../../h/list_bpt.h
P2toRFC.o: ../../h/extension.h
P2toRFC.o: ../../h/mta.h
P2toRFC.o: ../../h/adr.h
P2toRFC.o: ../../h/list_bpt.h
P2toRFC.o: ../../h/aparse.h
P2toRFC.o: ../../h/ap.h
P2toRFC.o: ../../h/util.h
P2toRFC.o: ../../h/or.h
P2toRFC.o: ../../h/chan.h
P2toRFC.o: ../../h/auth.h
P2toRFC.o: ../../h/list_rchan.h
P2toRFC.o: ../../h/mta.h
P2toRFC.o: ../../h/tb_bpt88.h
P2toRFC.o: ../../h/or.h
P2toRFC.o: ../../h/ap.h
P2toRFC.o: oids.h
P2toRFC.o: ../../h/charset.h
c-P2toRFC.o: c-P2toRFC.c
c-P2toRFC.o: ../../h/util.h
c-P2toRFC.o: ../../h/config.h
c-P2toRFC.o: ../../h/ll_log.h
c-P2toRFC.o: ../../h/retcode.h
c-P2toRFC.o: ../../h/qmgr.h
c-P2toRFC.o: ../../h/Qmgr-types.h
c-P2toRFC.o: ../../h/Qmgr-ops.h
c-P2toRFC.o: ../../h/q.h
c-P2toRFC.o: ../../h/adr.h
c-P2toRFC.o: ../../h/list_rchan.h
c-P2toRFC.o: ../../h/chan.h
c-P2toRFC.o: ../../h/table.h
c-P2toRFC.o: ../../h/list_bpt.h
c-P2toRFC.o: ../../h/auth.h
c-P2toRFC.o: ../../h/list_bpt.h
c-P2toRFC.o: ../../h/extension.h
c-P2toRFC.o: ../../h/mta.h
c-P2toRFC.o: ../../h/adr.h
c-P2toRFC.o: ../../h/list_bpt.h
c-P2toRFC.o: ../../h/aparse.h
c-P2toRFC.o: ../../h/ap.h
c-P2toRFC.o: ../../h/util.h
c-P2toRFC.o: ../../h/or.h
c-P2toRFC.o: ../../h/chan.h
c-P2toRFC.o: ../../h/auth.h
c-P2toRFC.o: ../../h/list_rchan.h
c-P2toRFC.o: ../../h/mta.h
c-P2toRFC.o: ../../h/prm.h
c-P2toRFC.o: ../../h/chan.h
c-P2toRFC.o: ../../h/ap.h
c-P2toRFC.o: ../../h/dr.h
c-P2toRFC.o: ../../h/extension.h
c-P2toRFC.o: ../../h/mta.h
c-P2toRFC.o: ../../h/sys.file.h
c-P2toRFC.o: ../../h/tb_bpt88.h
t-P2toRFC.o: t-P2toRFC.c
t-P2toRFC.o: ../../h/head.h
t-P2toRFC.o: ../../h/util.h
t-P2toRFC.o: ../../h/config.h
t-P2toRFC.o: ../../h/ll_log.h
t-P2toRFC.o: ../../h/retcode.h
t-P2toRFC.o: ../../h/util.h
t-P2toRFC.o: ../../h/q.h
t-P2toRFC.o: ../../h/adr.h
t-P2toRFC.o: ../../h/list_rchan.h
t-P2toRFC.o: ../../h/chan.h
t-P2toRFC.o: ../../h/table.h
t-P2toRFC.o: ../../h/list_bpt.h
t-P2toRFC.o: ../../h/auth.h
t-P2toRFC.o: ../../h/list_bpt.h
t-P2toRFC.o: ../../h/extension.h
t-P2toRFC.o: ../../h/mta.h
t-P2toRFC.o: ../../h/adr.h
t-P2toRFC.o: ../../h/list_bpt.h
t-P2toRFC.o: ../../h/aparse.h
t-P2toRFC.o: ../../h/ap.h
t-P2toRFC.o: ../../h/util.h
t-P2toRFC.o: ../../h/or.h
t-P2toRFC.o: ../../h/chan.h
t-P2toRFC.o: ../../h/auth.h
t-P2toRFC.o: ../../h/list_rchan.h
t-P2toRFC.o: ../../h/mta.h
RFCtoP2.o: RFCtoP2.c
RFCtoP2.o: ../../h/head.h
RFCtoP2.o: ../../h/util.h
RFCtoP2.o: ../../h/config.h
RFCtoP2.o: ../../h/ll_log.h
RFCtoP2.o: ../../h/retcode.h
RFCtoP2.o: ../../h/util.h
RFCtoP2.o: ../../h/IOB-types.h
RFCtoP2.o: ../../h/MTA-types.h
RFCtoP2.o: ../../h/Toks-types.h
RFCtoP2.o: ../../h/Auth-types.h
RFCtoP2.o: rfc-hdr.h
RFCtoP2.o: ../../h/ap.h
RFCtoP2.o: ../../h/util.h
RFCtoP2.o: ../../h/or.h
RFCtoP2.o: oids.h
RFCtoP2.o: ../../h/charset.h
RFCtoP2.o: ../../h/chan.h
RFCtoP2.o: ../../h/table.h
RFCtoP2.o: ../../h/list_bpt.h
c-RFCtoP2.o: c-RFCtoP2.c
c-RFCtoP2.o: ../../h/util.h
c-RFCtoP2.o: ../../h/config.h
c-RFCtoP2.o: ../../h/ll_log.h
c-RFCtoP2.o: ../../h/head.h
c-RFCtoP2.o: ../../h/util.h
c-RFCtoP2.o: ../../h/retcode.h
c-RFCtoP2.o: ../../h/qmgr.h
c-RFCtoP2.o: ../../h/Qmgr-types.h
c-RFCtoP2.o: ../../h/Qmgr-ops.h
c-RFCtoP2.o: ../../h/q.h
c-RFCtoP2.o: ../../h/adr.h
c-RFCtoP2.o: ../../h/list_rchan.h
c-RFCtoP2.o: ../../h/chan.h
c-RFCtoP2.o: ../../h/table.h
c-RFCtoP2.o: ../../h/list_bpt.h
c-RFCtoP2.o: ../../h/auth.h
c-RFCtoP2.o: ../../h/list_bpt.h
c-RFCtoP2.o: ../../h/extension.h
c-RFCtoP2.o: ../../h/mta.h
c-RFCtoP2.o: ../../h/adr.h
c-RFCtoP2.o: ../../h/list_bpt.h
c-RFCtoP2.o: ../../h/aparse.h
c-RFCtoP2.o: ../../h/ap.h
c-RFCtoP2.o: ../../h/util.h
c-RFCtoP2.o: ../../h/or.h
c-RFCtoP2.o: ../../h/chan.h
c-RFCtoP2.o: ../../h/auth.h
c-RFCtoP2.o: ../../h/list_rchan.h
c-RFCtoP2.o: ../../h/mta.h
c-RFCtoP2.o: ../../h/prm.h
c-RFCtoP2.o: ../../h/chan.h
c-RFCtoP2.o: ../../h/sys.file.h
t-RFCtoP2.o: t-RFCtoP2.c
t-RFCtoP2.o: ../../h/head.h
t-RFCtoP2.o: ../../h/util.h
t-RFCtoP2.o: ../../h/config.h
t-RFCtoP2.o: ../../h/ll_log.h
t-RFCtoP2.o: ../../h/retcode.h
t-RFCtoP2.o: ../../h/util.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
