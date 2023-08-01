# asn - takes a ASN.1 Body Part and converts it into another 
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/asn/RCS/Makefile,v 6.0 1991/12/18 20:15:43 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:15:43  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  asn.c asn_charset.c asn_dec.c asn_enc.c asn_proc.c asn_rd.c f_6937.c \
           f_gentxt.c f_teletex.c opt_cmdln.c opt_functs.c ut_asn.c ut_ext.c \
           ut_f.c
OBJS =  asn.o asn_charset.o asn_dec.o asn_enc.o asn_proc.o asn_rd.o f_6937.o \
           f_gentxt.o f_teletex.o opt_cmdln.o opt_functs.o ut_asn.o ut_ext.o \
           ut_f.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT


HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
LIBASN1		= asn1/libasn1.a
LIBASN1-MADE	= asn1/asn1-made

CFLAGS          = $(CCOPTIONS) -g $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) -g $(LLF)

LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)




############################################################
#
# Building Rules
#
############################################################

PROGS  = xasn

default: $(PROGS)

xasn:    $(LIBASN1) $(LIBASN1-MADE) $(LIBPP) $(OBJS) 
	(echo "cd asn1; $(MAKE)"; cd asn1; $(MAKE))
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBASN1) $(LIBPP) $(LIBSYS)

$(LIBASN1) $(LIBASN1-MADE): 
	(echo "cd asn1; $(MAKE)"; cd asn1; $(MAKE))



install: inst-dir inst-asn
inst-dir: $(CHANDIR)
inst-asn: $(FORMDIR)/asn

$(FORMDIR)/asn: xasn
		     -$(BACKUP) $@ zxasn
		     rm -f $@           
		     $(INSTALL) xasn $@
		     -$(CHMOD) $(PGMPROT) $@
		     -$(CHOWN) $(PPUSER) $@
		     -@ls -ls $@
		     -@echo "asn filter installed normally"; echo ""




clean: tidy
	(echo "cd asn1; $(MAKE) $@"; cd asn1; $(MAKE) $@)
	rm -f $(OBJS)

tidy:
	(echo "cd asn1; $(MAKE) $@"; cd asn1; $(MAKE) $@)
	rm -f $(PROGS) core a.out *.old *.BAK zxasn


lint: l-asn

l-asn: $(SRCS)
	      $(LINT) $(LLFLAGS) $(SRCS) asn1/l-asn1 $(LINTLIBS)

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
asn.o: asn.c
asn.o: ../../h/head.h
asn.o: ../../h/util.h
asn.o: ../../h/config.h
asn.o: ../../h/ll_log.h
asn.o: ../../h/retcode.h
asn.o: ./asn.h
asn_charset.o: asn_charset.c
asn_charset.o: ../../h/head.h
asn_charset.o: ../../h/util.h
asn_charset.o: ../../h/config.h
asn_charset.o: ../../h/ll_log.h
asn_charset.o: ../../h/retcode.h
asn_charset.o: ../../h/charset.h
asn_charset.o: ./asn.h
asn_dec.o: asn_dec.c
asn_dec.o: ../../h/head.h
asn_dec.o: ../../h/util.h
asn_dec.o: ../../h/config.h
asn_dec.o: ../../h/ll_log.h
asn_dec.o: ../../h/retcode.h
asn_dec.o: ./asn.h
asn_enc.o: asn_enc.c
asn_enc.o: ../../h/head.h
asn_enc.o: ../../h/util.h
asn_enc.o: ../../h/config.h
asn_enc.o: ../../h/ll_log.h
asn_enc.o: ../../h/retcode.h
asn_enc.o: ./asn.h
asn_proc.o: asn_proc.c
asn_proc.o: ../../h/head.h
asn_proc.o: ../../h/util.h
asn_proc.o: ../../h/config.h
asn_proc.o: ../../h/ll_log.h
asn_proc.o: ../../h/retcode.h
asn_proc.o: ./asn.h
asn_rd.o: asn_rd.c
asn_rd.o: ../../h/head.h
asn_rd.o: ../../h/util.h
asn_rd.o: ../../h/config.h
asn_rd.o: ../../h/ll_log.h
asn_rd.o: ../../h/retcode.h
asn_rd.o: ./asn.h
f_6937.o: f_6937.c
f_6937.o: ../../h/head.h
f_6937.o: ../../h/util.h
f_6937.o: ../../h/config.h
f_6937.o: ../../h/ll_log.h
f_6937.o: ../../h/retcode.h
f_6937.o: ../../h/IOB-types.h
f_6937.o: ../../h/MTA-types.h
f_6937.o: ../../h/Toks-types.h
f_6937.o: ../../h/Auth-types.h
f_6937.o: ./asn.h
f_gentxt.o: f_gentxt.c
f_gentxt.o: ../../h/head.h
f_gentxt.o: ../../h/util.h
f_gentxt.o: ../../h/config.h
f_gentxt.o: ../../h/ll_log.h
f_gentxt.o: ../../h/retcode.h
f_gentxt.o: ./asn.h
f_teletex.o: f_teletex.c
f_teletex.o: ../../h/head.h
f_teletex.o: ../../h/util.h
f_teletex.o: ../../h/config.h
f_teletex.o: ../../h/ll_log.h
f_teletex.o: ../../h/retcode.h
f_teletex.o: ../../h/IOB-types.h
f_teletex.o: ../../h/MTA-types.h
f_teletex.o: ../../h/Toks-types.h
f_teletex.o: ../../h/Auth-types.h
f_teletex.o: ./asn.h
opt_cmdln.o: opt_cmdln.c
opt_cmdln.o: ../../h/head.h
opt_cmdln.o: ../../h/util.h
opt_cmdln.o: ../../h/config.h
opt_cmdln.o: ../../h/ll_log.h
opt_cmdln.o: ../../h/retcode.h
opt_cmdln.o: ./asn.h
opt_functs.o: opt_functs.c
opt_functs.o: ../../h/head.h
opt_functs.o: ../../h/util.h
opt_functs.o: ../../h/config.h
opt_functs.o: ../../h/ll_log.h
opt_functs.o: ../../h/retcode.h
opt_functs.o: ./asn.h
ut_asn.o: ut_asn.c
ut_asn.o: ../../h/head.h
ut_asn.o: ../../h/util.h
ut_asn.o: ../../h/config.h
ut_asn.o: ../../h/ll_log.h
ut_asn.o: ../../h/retcode.h
ut_asn.o: ./asn.h
ut_ext.o: ut_ext.c
ut_ext.o: ../../h/head.h
ut_ext.o: ../../h/util.h
ut_ext.o: ../../h/config.h
ut_ext.o: ../../h/ll_log.h
ut_ext.o: ../../h/retcode.h
ut_ext.o: ./asn.h
ut_ext.o: ./oid.h
ut_f.o: ut_f.c
ut_f.o: ../../h/head.h
ut_f.o: ../../h/util.h
ut_f.o: ../../h/config.h
ut_f.o: ../../h/ll_log.h
ut_f.o: ../../h/retcode.h
ut_f.o: ./asn.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
