# Makefile to build the 822 addressing stuff
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/addr/RCS/Makefile,v 6.0 1991/12/18 20:21:24 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:21:24  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ap_1adr.c ap_dmflip.c ap_file.c ap_lex.c ap_lex_tb.c ap_norm.c \
           ap_p2s.c ap_s2p.c ap_s2t.c ap_t2p.c ap_t2s.c ap_ut.c \
	   ap_val2str.c ap_s2s.c ap_equ.c
OBJS =  ap_1adr.o ap_dmflip.o ap_file.o ap_lex.o ap_lex_tb.o ap_norm.o \
           ap_p2s.o ap_s2p.o ap_s2t.o ap_t2p.o ap_t2s.o ap_ut.o \
	   ap_val2str.o ap_s2s.o ap_equ.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -DAP_DEBUG -I$(HEADERS) $(LCF)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = addr-made
LINTLIBS        = ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
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

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS) 
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lintlib: llib-laddr.ln
llib-laddr.ln: $(SRCS)
	$(LINT) $(LLFLAGS) -Caddr $(SRCS)

install:;

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

clean: tidy
	rm -f $(OBJS) llib-laddr.ln
tidy:
	rm -f core $(SHADOW) *.old  *.BAK

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
ap_1adr.o: ap_1adr.c
ap_1adr.o: ../../h/util.h
ap_1adr.o: ../../h/config.h
ap_1adr.o: ../../h/ll_log.h
ap_1adr.o: ../../h/ap.h
ap_1adr.o: ../../h/util.h
ap_1adr.o: ../../h/ap_lex.h
ap_dmflip.o: ap_dmflip.c
ap_dmflip.o: ../../h/util.h
ap_dmflip.o: ../../h/config.h
ap_dmflip.o: ../../h/ll_log.h
ap_file.o: ap_file.c
ap_file.o: ../../h/util.h
ap_file.o: ../../h/config.h
ap_file.o: ../../h/ll_log.h
ap_file.o: ../../h/ap.h
ap_file.o: ../../h/util.h
ap_lex.o: ap_lex.c
ap_lex.o: ../../h/util.h
ap_lex.o: ../../h/config.h
ap_lex.o: ../../h/ll_log.h
ap_lex.o: ../../h/ap.h
ap_lex.o: ../../h/util.h
ap_lex.o: ../../h/ap_lex.h
ap_lex_tb.o: ap_lex_tb.c
ap_lex_tb.o: ../../h/util.h
ap_lex_tb.o: ../../h/config.h
ap_lex_tb.o: ../../h/ll_log.h
ap_lex_tb.o: ../../h/ap_lex.h
ap_lex_tb.o: ../../h/ap.h
ap_lex_tb.o: ../../h/util.h
ap_norm.o: ap_norm.c
ap_norm.o: ../../h/head.h
ap_norm.o: ../../h/util.h
ap_norm.o: ../../h/config.h
ap_norm.o: ../../h/ll_log.h
ap_norm.o: ../../h/retcode.h
ap_norm.o: ../../h/chan.h
ap_norm.o: ../../h/table.h
ap_norm.o: ../../h/list_bpt.h
ap_norm.o: ../../h/ap.h
ap_norm.o: ../../h/util.h
ap_p2s.o: ap_p2s.c
ap_p2s.o: ../../h/util.h
ap_p2s.o: ../../h/config.h
ap_p2s.o: ../../h/ll_log.h
ap_p2s.o: ../../h/chan.h
ap_p2s.o: ../../h/table.h
ap_p2s.o: ../../h/list_bpt.h
ap_p2s.o: ../../h/ap.h
ap_p2s.o: ../../h/util.h
ap_s2p.o: ap_s2p.c
ap_s2p.o: ../../h/util.h
ap_s2p.o: ../../h/config.h
ap_s2p.o: ../../h/ll_log.h
ap_s2p.o: ../../h/ap_lex.h
ap_s2p.o: ../../h/ap.h
ap_s2p.o: ../../h/util.h
ap_s2t.o: ap_s2t.c
ap_s2t.o: ../../h/util.h
ap_s2t.o: ../../h/config.h
ap_s2t.o: ../../h/ll_log.h
ap_s2t.o: ../../h/ap.h
ap_s2t.o: ../../h/util.h
ap_t2p.o: ap_t2p.c
ap_t2p.o: ../../h/util.h
ap_t2p.o: ../../h/config.h
ap_t2p.o: ../../h/ll_log.h
ap_t2p.o: ../../h/ap.h
ap_t2p.o: ../../h/util.h
ap_t2s.o: ap_t2s.c
ap_t2s.o: ../../h/util.h
ap_t2s.o: ../../h/config.h
ap_t2s.o: ../../h/ll_log.h
ap_t2s.o: ../../h/ap.h
ap_t2s.o: ../../h/util.h
ap_ut.o: ap_ut.c
ap_ut.o: ../../h/util.h
ap_ut.o: ../../h/config.h
ap_ut.o: ../../h/ll_log.h
ap_ut.o: ../../h/ap.h
ap_ut.o: ../../h/util.h
ap_val2str.o: ap_val2str.c
ap_val2str.o: ../../h/util.h
ap_val2str.o: ../../h/config.h
ap_val2str.o: ../../h/ll_log.h
ap_val2str.o: ../../h/ap.h
ap_val2str.o: ../../h/util.h
ap_s2s.o: ap_s2s.c
ap_s2s.o: ../../h/util.h
ap_s2s.o: ../../h/config.h
ap_s2s.o: ../../h/ll_log.h
ap_s2s.o: ../../h/ap.h
ap_s2s.o: ../../h/util.h
ap_equ.o: ap_equ.c
ap_equ.o: ../../h/util.h
ap_equ.o: ../../h/config.h
ap_equ.o: ../../h/ll_log.h
ap_equ.o: ../../h/ap.h
ap_equ.o: ../../h/util.h
ap_equ.o: ../../h/adr.h
ap_equ.o: ../../h/list_rchan.h
ap_equ.o: ../../h/chan.h
ap_equ.o: ../../h/table.h
ap_equ.o: ../../h/list_bpt.h
ap_equ.o: ../../h/auth.h
ap_equ.o: ../../h/list_bpt.h
ap_equ.o: ../../h/extension.h
ap_equ.o: ../../h/mta.h
ap_equ.o: ../../h/adr.h
ap_equ.o: ../../h/list_bpt.h
ap_equ.o: ../../h/aparse.h
ap_equ.o: ../../h/ap.h
ap_equ.o: ../../h/or.h
ap_equ.o: ../../h/chan.h
ap_equ.o: ../../h/auth.h
ap_equ.o: ../../h/list_rchan.h
ap_equ.o: ../../h/chan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
