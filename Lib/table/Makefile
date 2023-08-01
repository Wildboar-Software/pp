# Table & database lookup routines.
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/table/RCS/Makefile,v 6.0 1991/12/18 20:24:28 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:24:28  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ch_nm2struct.c tb_1148gate.c tb_checkauth.c tb_dbm.c tb_getalias.c \
           tb_getauth.c tb_getchan.c tb_getdl.c tb_getdomain.c tb_getlocal.c \
           tb_getuser.c tb_k2val.c tb_nm2struct.c tb_rtsparams.c
OBJS =  ch_nm2struct.o tb_1148gate.o tb_checkauth.o tb_dbm.o tb_getalias.o \
           tb_getauth.o tb_getchan.o tb_getdl.o tb_getdomain.o tb_getlocal.o \
           tb_getuser.o tb_k2val.o tb_nm2struct.o tb_rtsparams.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = table-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../tai/llib-ltai.ln \
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

lintlib: llib-ltable.ln
llib-ltable.ln: ${SRCS}
	$(LINT) -Ctable $(LLFLAGS) ${SRCS}

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

clean: tidy
	rm -f ${OBJS} llib-ltable.ln

tidy:
	rm -f core ${SHADOW} *.old a.out *.BAK

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

tb_dbm.o: tb_dbm.c
	$(OLDCC) -c $(CFLAGS) tb_dbm.c

############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
ch_nm2struct.o: ch_nm2struct.c
ch_nm2struct.o: ../../h/util.h
ch_nm2struct.o: ../../h/config.h
ch_nm2struct.o: ../../h/ll_log.h
ch_nm2struct.o: ../../h/chan.h
ch_nm2struct.o: ../../h/table.h
ch_nm2struct.o: ../../h/list_bpt.h
ch_nm2struct.o: ../../h/list_bpt.h
tb_1148gate.o: tb_1148gate.c
tb_1148gate.o: ../../h/or.h
tb_1148gate.o: ../../h/util.h
tb_1148gate.o: ../../h/config.h
tb_1148gate.o: ../../h/ll_log.h
tb_1148gate.o: ../../h/table.h
tb_checkauth.o: tb_checkauth.c
tb_checkauth.o: ../../h/util.h
tb_checkauth.o: ../../h/config.h
tb_checkauth.o: ../../h/ll_log.h
tb_checkauth.o: ../../h/chan.h
tb_checkauth.o: ../../h/table.h
tb_checkauth.o: ../../h/list_bpt.h
tb_dbm.o: tb_dbm.c
tb_dbm.o: ../../h/head.h
tb_dbm.o: ../../h/util.h
tb_dbm.o: ../../h/config.h
tb_dbm.o: ../../h/ll_log.h
tb_dbm.o: ../../h/retcode.h
tb_dbm.o: ../../h/dbase.h
tb_dbm.o: ../../h/config.h
tb_dbm.o: ../../h/table.h
tb_dbm.o: ../../h/sys.file.h
tb_dbm.o: ../../h/util.h
tb_getalias.o: tb_getalias.c
tb_getalias.o: ../../h/util.h
tb_getalias.o: ../../h/config.h
tb_getalias.o: ../../h/ll_log.h
tb_getalias.o: ../../h/table.h
tb_getalias.o: ../../h/adr.h
tb_getalias.o: ../../h/list_rchan.h
tb_getalias.o: ../../h/chan.h
tb_getalias.o: ../../h/table.h
tb_getalias.o: ../../h/list_bpt.h
tb_getalias.o: ../../h/auth.h
tb_getalias.o: ../../h/list_bpt.h
tb_getalias.o: ../../h/extension.h
tb_getalias.o: ../../h/mta.h
tb_getalias.o: ../../h/adr.h
tb_getalias.o: ../../h/list_bpt.h
tb_getalias.o: ../../h/aparse.h
tb_getalias.o: ../../h/ap.h
tb_getalias.o: ../../h/util.h
tb_getalias.o: ../../h/or.h
tb_getalias.o: ../../h/chan.h
tb_getalias.o: ../../h/auth.h
tb_getalias.o: ../../h/list_rchan.h
tb_getalias.o: ../../h/alias.h
tb_getauth.o: tb_getauth.c
tb_getauth.o: ../../h/head.h
tb_getauth.o: ../../h/util.h
tb_getauth.o: ../../h/config.h
tb_getauth.o: ../../h/ll_log.h
tb_getauth.o: ../../h/retcode.h
tb_getauth.o: ../../h/util.h
tb_getauth.o: ../../h/chan.h
tb_getauth.o: ../../h/table.h
tb_getauth.o: ../../h/list_bpt.h
tb_getauth.o: ../../h/auth.h
tb_getauth.o: ../../h/list_bpt.h
tb_getauth.o: ../../h/list_rchan.h
tb_getauth.o: ../../h/chan.h
tb_getauth.o: ../../h/auth.h
tb_getauth.o: ../../h/adr.h
tb_getauth.o: ../../h/list_rchan.h
tb_getauth.o: ../../h/extension.h
tb_getauth.o: ../../h/mta.h
tb_getauth.o: ../../h/adr.h
tb_getauth.o: ../../h/list_bpt.h
tb_getauth.o: ../../h/aparse.h
tb_getauth.o: ../../h/ap.h
tb_getauth.o: ../../h/util.h
tb_getauth.o: ../../h/or.h
tb_getauth.o: ../../h/chan.h
tb_getauth.o: ../../h/auth.h
tb_getauth.o: ../../h/list_rchan.h
tb_getchan.o: tb_getchan.c
tb_getchan.o: ../../h/util.h
tb_getchan.o: ../../h/config.h
tb_getchan.o: ../../h/ll_log.h
tb_getchan.o: ../../h/list_rchan.h
tb_getchan.o: ../../h/chan.h
tb_getchan.o: ../../h/table.h
tb_getchan.o: ../../h/list_bpt.h
tb_getchan.o: ../../h/auth.h
tb_getchan.o: ../../h/list_bpt.h
tb_getdl.o: tb_getdl.c
tb_getdl.o: ../../h/util.h
tb_getdl.o: ../../h/config.h
tb_getdl.o: ../../h/ll_log.h
tb_getdl.o: ../../h/chan.h
tb_getdl.o: ../../h/table.h
tb_getdl.o: ../../h/list_bpt.h
tb_getdl.o: ../../h/table.h
tb_getdl.o: ../../h/dl.h
tb_getdl.o: ../../h/adr.h
tb_getdl.o: ../../h/list_rchan.h
tb_getdl.o: ../../h/chan.h
tb_getdl.o: ../../h/auth.h
tb_getdl.o: ../../h/list_bpt.h
tb_getdl.o: ../../h/extension.h
tb_getdl.o: ../../h/mta.h
tb_getdl.o: ../../h/adr.h
tb_getdl.o: ../../h/list_bpt.h
tb_getdl.o: ../../h/aparse.h
tb_getdl.o: ../../h/ap.h
tb_getdl.o: ../../h/util.h
tb_getdl.o: ../../h/or.h
tb_getdl.o: ../../h/chan.h
tb_getdl.o: ../../h/auth.h
tb_getdl.o: ../../h/list_rchan.h
tb_getdl.o: ../../h/retcode.h
tb_getdomain.o: tb_getdomain.c
tb_getdomain.o: ../../h/util.h
tb_getdomain.o: ../../h/config.h
tb_getdomain.o: ../../h/ll_log.h
tb_getdomain.o: ../../h/list_rchan.h
tb_getdomain.o: ../../h/chan.h
tb_getdomain.o: ../../h/table.h
tb_getdomain.o: ../../h/list_bpt.h
tb_getdomain.o: ../../h/auth.h
tb_getdomain.o: ../../h/list_bpt.h
tb_getlocal.o: tb_getlocal.c
tb_getlocal.o: ../../h/util.h
tb_getlocal.o: ../../h/config.h
tb_getlocal.o: ../../h/ll_log.h
tb_getlocal.o: ../../h/chan.h
tb_getlocal.o: ../../h/table.h
tb_getlocal.o: ../../h/list_bpt.h
tb_getlocal.o: ../../h/loc_user.h
tb_getuser.o: tb_getuser.c
tb_getuser.o: ../../h/util.h
tb_getuser.o: ../../h/config.h
tb_getuser.o: ../../h/ll_log.h
tb_getuser.o: ../../h/chan.h
tb_getuser.o: ../../h/table.h
tb_getuser.o: ../../h/list_bpt.h
tb_getuser.o: ../../h/list_rchan.h
tb_getuser.o: ../../h/chan.h
tb_getuser.o: ../../h/auth.h
tb_getuser.o: ../../h/list_bpt.h
tb_k2val.o: tb_k2val.c
tb_k2val.o: ../../h/util.h
tb_k2val.o: ../../h/config.h
tb_k2val.o: ../../h/ll_log.h
tb_k2val.o: ../../h/table.h
tb_nm2struct.o: tb_nm2struct.c
tb_nm2struct.o: ../../h/util.h
tb_nm2struct.o: ../../h/config.h
tb_nm2struct.o: ../../h/ll_log.h
tb_nm2struct.o: ../../h/table.h
tb_rtsparams.o: tb_rtsparams.c
tb_rtsparams.o: ../../h/util.h
tb_rtsparams.o: ../../h/config.h
tb_rtsparams.o: ../../h/ll_log.h
tb_rtsparams.o: ../../h/chan.h
tb_rtsparams.o: ../../h/table.h
tb_rtsparams.o: ../../h/list_bpt.h
tb_rtsparams.o: ../../h/rtsparams.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
