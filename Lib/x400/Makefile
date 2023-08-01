# PP X400 1988 asn1 stuff
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/x400/RCS/Makefile,v 6.0 1991/12/18 20:25:37 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:25:37  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################


SRCS	= enctypes2mem.c Toks_tables.c MTA_tables.c MTA-ops.c IOB_tables.c \
	Auth_tables.c Ext_tables.c Trans_tables.c\
	RTS84_tables.c fixorig.c rasn.c

OBJS	= enctypes2mem.o Toks_tables.o MTA_tables.o MTA-ops.o IOB_tables.o \
	Auth_tables.o Ext_tables.o Trans_tables.o\
	RTS84_tables.o fixorig.o rasn.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

PYFLAGS = -P
HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(LIBCCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)
SHADOW          = x400-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../tai/llib-ltai.ln \
		  ../util/llib-lutil.ln

############################################################
#
# Building Rules
#
############################################################

default: ${SHADOW}


${SHADOW}: $(OBJS)
	$(AR) r$(ARFLAGS) ../libpp.a $?
	touch $@

lintlib: llib-lx400.ln
llib-lx400.ln: $(SRCS)
	$(LINT) $(LLFLAGS) -Cx400 $(SRCS)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

############################################################
#
# token modules
#
############################################################

Toks_tables.o: Auth-types.h

Toks_tables.c Toks-types.h: tokens.py
	$(PEPSY) $(POFLAGS) -A -h -f tokens.py
	-cmp -s Toks-types.h $(HEADERS)/Toks-types.h || \
		cp Toks-types.h $(HEADERS)/Toks-types.h
	-if [ -f Toks_defs.h ]; then \
		cmp -s Toks_defs.h $(HEADERS)/Toks_defs.h || \
		cp Toks_defs.h $(HEADERS)/Toks_defs.h; \
	fi
	-if [ -f Toks_pre_defs.h ]; then \
		cmp -s Toks_pre_defs.h $(HEADERS)/Toks_pre_defs.h || \
		cp Toks_pre_defs.h $(HEADERS)/Toks_pre_defs.h; \
	fi

############################################################
#
# Auth modules
#
############################################################

Auth_tables.o: Auth-types.h

Auth_tables.c Auth-types.h: authen.py
	$(PEPSY) $(POFLAGS) -A -h -f authen.py
	-cmp -s Auth-types.h $(HEADERS)/Auth-types.h || \
	cp Auth-types.h $(HEADERS)/Auth-types.h
	-if [ -f Auth_defs.h ]; then \
		cmp -s Auth_defs.h $(HEADERS)/Auth_defs.h || \
		cp Auth_defs.h $(HEADERS)/Auth_defs.h; \
		cmp -s Auth_pre_defs.h $(HEADERS)/Auth_pre_defs.h || \
		cp Auth_pre_defs.h $(HEADERS)/Auth_pre_defs.h; \
	fi

############################################################
#
# MTA modules
#
############################################################

MTA_tables.o: MTA-types.h

MTA-asn.py:mta.ry
	$(ROSY) $(RYFLAGS) -pepsy -o $@ mta.ry
	-cmp -s MTA-ops.h $(HEADERS)/MTA-ops.h ||\
		cp MTA-ops.h $(HEADERS)/MTA-ops.h

MTA-types.h MTA_tables.c: MTA-asn.py
	$(PEPSY) $(POFLAGS) -A -h -f MTA-asn.py
	-cmp -s MTA-types.h $(HEADERS)/MTA-types.h || \
		cp MTA-types.h $(HEADERS)/MTA-types.h
	-if [ -f MTA_defs.h ]; then \
		cmp -s MTA_defs.h $(HEADERS)/MTA_defs.h || \
		cp MTA_defs.h $(HEADERS)/MTA_defs.h; \
		cmp -s MTA_pre_defs.h $(HEADERS)/MTA_pre_defs.h || \
		cp MTA_pre_defs.h $(HEADERS)/MTA_pre_defs.h; \
	fi

############################################################
#
# Extension stuff
#
############################################################

Ext_tables.o: Ext-types.h

Ext-types.h Ext_tables.c: extent.py
	$(PEPSY) $(POFLAGS) -A -h -f extent.py
	-cmp -s Ext-types.h $(HEADERS)/Ext-types.h || \
		cp Ext-types.h $(HEADERS)/Ext-types.h
	-if [ -f Ext_defs.h ]; then \
		cmp -s Ext_defs.h $(HEADERS)/Ext_defs.h || \
		cp Ext_defs.h $(HEADERS)/Ext_defs.h; \
		cmp -s Ext_pre_defs.h $(HEADERS)/Ext_pre_defs.h || \
		cp Ext_pre_defs.h $(HEADERS)/Ext_pre_defs.h; \
	fi

############################################################
#
# Trans stuff
#
############################################################

Trans_tables.o: Trans-types.h

Trans-types.h Trans_tables.c: transfer.py
	$(PEPSY) $(POFLAGS) -A -h -f transfer.py
	-cmp -s Trans-types.h $(HEADERS)/Trans-types.h ||\
		cp Trans-types.h $(HEADERS)/Trans-types.h
	-if [ -f Trans_defs.h ]; then\
		cmp -s Trans_defs.h $(HEADERS)/Trans_defs.h ||\
		cp Trans_defs.h $(HEADERS)/Trans_defs.h; \
		cmp -s Trans_pre_defs.h $(HEADERS)/Trans_pre_defs.h ||\
		cp Trans_pre_defs.h $(HEADERS)/Trans_pre_defs.h; \
	fi

############################################################
#
# Information Object stuff
#
############################################################

IOB_tables.o: IOB-types.h

IOB-types.h IOB_tables.c: iob.py
	$(PEPSY) $(POFLAGS) -A -h -f iob.py
	-cmp -s IOB-types.h $(HEADERS)/IOB-types.h ||\
		cp IOB-types.h $(HEADERS)/IOB-types.h
	-if [ -f IOB_defs.h ]; then \
		cmp -s IOB_defs.h $(HEADERS)/IOB_defs.h ||\
		cp IOB_defs.h $(HEADERS)/IOB_defs.h; \
		cmp -s IOB_pre_defs.h $(HEADERS)/IOB_pre_defs.h ||\
		cp IOB_pre_defs.h $(HEADERS)/IOB_pre_defs.h; \
	fi

############################################################
#
# RTS 84 stuff
#
############################################################

RTS84_tables.o: RTS84-types.h

RTS84-types.h RTS84_tables.c: Rts_posy.py
	$(PEPSY) $(POFLAGS) -A -h -f Rts_posy.py
	-cmp -s RTS84-types.h $(HEADERS)/RTS84-types.h ||\
		cp RTS84-types.h $(HEADERS)/RTS84-types.h
	-if [ -f RTS84_defs.h ]; then \
		cmp -s RTS84_defs.h $(HEADERS)/RTS84_defs.h ||\
		cp RTS84_defs.h $(HEADERS)/RTS84_defs.h; \
		cmp -s RTS84_pre_defs.h $(HEADERS)/RTS84_pre_defs.h ||\
		cp RTS84_pre_defs.h $(HEADERS)/RTS84_pre_defs.h; \
	fi

############################################################
#
# Other stuff
#
############################################################

install:;

clean: tidy
	rm -f *.h ${OBJS} llib-lx400.ln \
	Toks* Auth* MTA* Ext* \
	Trans* IOB* RTS84* \
	$(HEADERS)/Toks-types.h $(HEADERS)/Toks_defs.h \
	$(HEADERS)/Toks_pre_defs.h \
	$(HEADERS)/Auth-types.h $(HEADERS)/Auth_defs.h \
	$(HEADERS)/Auth_pre_defs.h \
	$(HEADERS)/MTA-ops.h $(HEADERS)/MTA-types.h \
	$(HEADERS)/MTA_defs.h $(HEADERS)/MTA_pre_defs.h \
	$(HEADERS)/Ext-types.h $(HEADERS)/Ext_defs.h \
	$(HEADERS)/Ext_pre_defs.h \
	$(HEADERS)/Trans-types.h $(HEADERS)/Trans_defs.h \
	$(HEADERS)/Trans_pre_defs.h \
	$(HEADERS)/IOB-types.h $(HEADERS)/IOB_defs.h \
	$(HEADERS)/IOB_pre_defs.h \
	$(HEADERS)/RTS84-types.h $(HEADERS)/RTS84_defs.h \
	$(HEADERS)/RTS84_pre_defs.h 
tidy:
	rm -f core a.out Makefile.old  $(SHADOW)
	

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
enctypes2mem.o: enctypes2mem.c
enctypes2mem.o: ../../h/util.h
enctypes2mem.o: ../../h/config.h
enctypes2mem.o: ../../h/ll_log.h
enctypes2mem.o: ../../h/tb_bpt84.h
enctypes2mem.o: ../../h/list_bpt.h
enctypes2mem.o: ../../h/tb_p1.h
Toks_tables.o: Toks_tables.c
Toks_tables.o: Toks-types.h
Toks_tables.o: Auth-types.h
MTA_tables.o: MTA_tables.c
MTA_tables.o: MTA-types.h
MTA_tables.o: Toks-types.h
MTA_tables.o: Auth-types.h
MTA-ops.o: MTA-ops.c
MTA-ops.o: MTA-types.h
MTA-ops.o: Toks-types.h
MTA-ops.o: Auth-types.h
MTA-ops.o: MTA-ops.h
IOB_tables.o: IOB_tables.c
IOB_tables.o: IOB-types.h
IOB_tables.o: MTA-types.h
IOB_tables.o: Toks-types.h
IOB_tables.o: Auth-types.h
Auth_tables.o: Auth_tables.c
Auth_tables.o: Auth-types.h
Ext_tables.o: Ext_tables.c
Ext_tables.o: Ext-types.h
Ext_tables.o: MTA-types.h
Ext_tables.o: Toks-types.h
Ext_tables.o: Auth-types.h
Ext_tables.o: Toks-types.h
Ext_tables.o: Auth-types.h
Trans_tables.o: Trans_tables.c
Trans_tables.o: Trans-types.h
Trans_tables.o: MTA-types.h
Trans_tables.o: Toks-types.h
Trans_tables.o: Auth-types.h
RTS84_tables.o: RTS84_tables.c
RTS84_tables.o: RTS84-types.h
fixorig.o: fixorig.c
fixorig.o: ../../h/util.h
fixorig.o: ../../h/config.h
fixorig.o: ../../h/ll_log.h
fixorig.o: ../../h/adr.h
fixorig.o: ../../h/list_rchan.h
fixorig.o: ../../h/chan.h
fixorig.o: ../../h/table.h
fixorig.o: ../../h/list_bpt.h
fixorig.o: ../../h/auth.h
fixorig.o: ../../h/list_bpt.h
fixorig.o: ../../h/extension.h
fixorig.o: ../../h/mta.h
fixorig.o: ../../h/adr.h
fixorig.o: ../../h/list_bpt.h
fixorig.o: ../../h/aparse.h
fixorig.o: ../../h/ap.h
fixorig.o: ../../h/util.h
fixorig.o: ../../h/or.h
fixorig.o: ../../h/chan.h
fixorig.o: ../../h/auth.h
fixorig.o: ../../h/list_rchan.h
fixorig.o: ../../h/or.h
rasn.o: rasn.c
rasn.o: ../../h/util.h
rasn.o: ../../h/config.h
rasn.o: ../../h/ll_log.h
rasn.o: ../../h/q.h
rasn.o: ../../h/adr.h
rasn.o: ../../h/list_rchan.h
rasn.o: ../../h/chan.h
rasn.o: ../../h/table.h
rasn.o: ../../h/list_bpt.h
rasn.o: ../../h/auth.h
rasn.o: ../../h/list_bpt.h
rasn.o: ../../h/extension.h
rasn.o: ../../h/mta.h
rasn.o: ../../h/adr.h
rasn.o: ../../h/list_bpt.h
rasn.o: ../../h/aparse.h
rasn.o: ../../h/ap.h
rasn.o: ../../h/util.h
rasn.o: ../../h/or.h
rasn.o: ../../h/chan.h
rasn.o: ../../h/auth.h
rasn.o: ../../h/list_rchan.h
rasn.o: ../../h/mta.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
