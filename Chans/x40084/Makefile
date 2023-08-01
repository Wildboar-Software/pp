# X400 inbound & outbound channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/x40084/RCS/Makefile,v 6.0 1991/12/18 20:13:50 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:13:50  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  P1_in.c P1_out.c P1_print.c Rts84-print.c Rts84-types.c \
           ut_logs.c \
           ut_misc.c ut_rts.c x400in84.c x400out84.c x400topp.c
OBJS =  P1_in.o P1_out.o P1_print.o Rts84-print.o Rts84-types.o  \
           ut_logs.o \
           ut_misc.o ut_rts.o x400in84.o x400out84.o x400topp.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
INCLUDE         = -I$(HEADERS)

CFLAGS          = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) $(INCLUDE)

LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS           = xx400in84 xx400out84 


############################################################
#
# Suffix rules
#
############################################################


.SUFFIXES: .py

.py.c:
	$(PEPY) $(PYFLAGS) -a PY_advise $<

.py.o:
	$(PEPY) $(PYFLAGS) -a PY_advise $<
	$(CC) $(CFLAGS) -c $*.c
	rm $*.c


############################################################
#
# Building Rules
#
############################################################


default: $(PROGS)


lint:  l-x400out84 l-x400in84

install: inst-dir inst-x400out84 inst-x400in84


############################################################
#
# X400out84
#
############################################################

OUTSRCS         = x400out84.c P1_out.c
OUTOBJS         = x400out84.o P1_out.o 

x400out84: xx400out84
xx400out84: $(OUTOBJS) libx400.a $(LIBPP)
	$(CC) $(LDFLAGS) -o xx400out84 $(OUTOBJS)  libx400.a \
		$(LIBPP) $(LIBSYS)

saber_x400out: $(OUTSRCS) $(LIBSRC)
	#load -C $(CFLAGS) $(OUTSRCS) $(LIBSRC) $(LIBPP) $(LIBSYS)

l-x400out84: $(OUTSRCS)
	$(LINT) $(LLFLAGS) $(OUTSRCS) $(LINTLIBS)

inst-x400out84: $(CHANDIR)/x400out84
$(CHANDIR)/x400out84: xx400out84
	-$(BACKUP) $@ zxx400out84
	rm -f $@
	$(INSTALL) xx400out84 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X400out84 channel installed normally"; echo ""


inst-dir: $(CHANDIR) $(TOOLDIR)

############################################################
#
# X400in84
#
############################################################

INSRCS          =  x400in84.c P1_in.c x400topp.c
INOBJS          =  x400in84.o P1_in.o x400topp.o

x400in84: xx400in84
xx400in84: $(INOBJS) libx400.a $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(INOBJS) libx400.a $(LIBPP) $(LIBSYS)


inst-x400in84: $(CHANDIR)/x400in84
$(CHANDIR)/x400in84: xx400in84
	-$(BACKUP) $@ zxx400in84
	rm -f $@
	$(INSTALL) xx400in84 $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "X400in84 channel installed normally"; echo ""


lint: l-x400in84
l-x400in84: $(INSRCS)
	$(LINT) $(LLFLAGS) $(INSRCS) $(LINTLIBS)


############################################################
#
# X400 library
#
############################################################

LIBSRC = Rts84-print.c Rts84-types.c ut_logs.c ut_rts.c ut_misc.c P1_print.c
LIBOBJS = Rts84-print.o Rts84-types.o ut_logs.o ut_rts.o ut_misc.o P1_print.o

libx400.a: $(LIBOBJS)
	$(AR) r$(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@
	touch $@

Rts84-print.c: Rts_posy.py
	$(PEPY) -a PY_advise -S PRINT -o $@ Rts_posy.py

Rts84-types.h Rts84-types.c: Rts84-types.py
	$(PEPY) -a PY_advise Rts84-types.py
#	-cmp -s Rts84-types.h $(HEADERS)/Rts84-types.h || \
#		cp Rts84-types.h $(HEADERS)/Rts84-types.h

Rts84-types.py:
	$(POSY) $(POFLAGS) -f -h -o Rts84-types.py Rts_posy.py

############################################################
#
# Other things
#
############################################################

clean: tidy
	rm -f $(OBJS) $(OUTOBJS) libx400.a
	rm -f P1_out.c P1_in.c P1_print.c Rts84-* acsold.c Rts84-types.h

tidy:
	rm -f $(PROGS) core a.out *.ph *old *.BAK zxx* zxrts*

depend: $(SRCS)
	$(DEPEND) $(INCLUDE) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
P1_in.o: P1_in.c
P1_in.o: ../../h/head.h
P1_in.o: ../../h/util.h
P1_in.o: ../../h/config.h
P1_in.o: ../../h/ll_log.h
P1_in.o: ../../h/retcode.h
P1_in.o: ../../h/prm.h
P1_in.o: ../../h/q.h
P1_in.o: ../../h/adr.h
P1_in.o: ../../h/list_rchan.h
P1_in.o: ../../h/chan.h
P1_in.o: ../../h/table.h
P1_in.o: ../../h/list_bpt.h
P1_in.o: ../../h/auth.h
P1_in.o: ../../h/list_bpt.h
P1_in.o: ../../h/extension.h
P1_in.o: ../../h/mta.h
P1_in.o: ../../h/adr.h
P1_in.o: ../../h/list_bpt.h
P1_in.o: ../../h/aparse.h
P1_in.o: ../../h/ap.h
P1_in.o: ../../h/util.h
P1_in.o: ../../h/or.h
P1_in.o: ../../h/chan.h
P1_in.o: ../../h/auth.h
P1_in.o: ../../h/list_rchan.h
P1_in.o: ../../h/mta.h
P1_in.o: ../../h/or.h
P1_in.o: ../../h/dr.h
P1_in.o: ../../h/extension.h
P1_in.o: ../../h/mta.h
P1_out.o: P1_out.c
P1_out.o: ../../h/head.h
P1_out.o: ../../h/util.h
P1_out.o: ../../h/config.h
P1_out.o: ../../h/ll_log.h
P1_out.o: ../../h/retcode.h
P1_out.o: ../../h/q.h
P1_out.o: ../../h/adr.h
P1_out.o: ../../h/list_rchan.h
P1_out.o: ../../h/chan.h
P1_out.o: ../../h/table.h
P1_out.o: ../../h/list_bpt.h
P1_out.o: ../../h/auth.h
P1_out.o: ../../h/list_bpt.h
P1_out.o: ../../h/extension.h
P1_out.o: ../../h/mta.h
P1_out.o: ../../h/adr.h
P1_out.o: ../../h/list_bpt.h
P1_out.o: ../../h/aparse.h
P1_out.o: ../../h/ap.h
P1_out.o: ../../h/util.h
P1_out.o: ../../h/or.h
P1_out.o: ../../h/chan.h
P1_out.o: ../../h/auth.h
P1_out.o: ../../h/list_rchan.h
P1_out.o: ../../h/mta.h
P1_out.o: ../../h/or.h
P1_out.o: ../../h/dr.h
P1_out.o: ../../h/extension.h
P1_out.o: ../../h/mta.h
P1_out.o: ../../h/rtsparams.h
P1_print.o: P1_print.c
Rts84-print.o: Rts84-print.c
Rts84-types.o: Rts84-types.c
Rts84-types.o: Rts84-types.h
ut_logs.o: ut_logs.c
ut_logs.o: ../../h/util.h
ut_logs.o: ../../h/config.h
ut_logs.o: ../../h/ll_log.h
ut_misc.o: ut_misc.c
ut_misc.o: ../../h/util.h
ut_misc.o: ../../h/config.h
ut_misc.o: ../../h/ll_log.h
ut_misc.o: ../../h/tb_bpt84.h
ut_misc.o: ../../h/list_bpt.h
ut_rts.o: ut_rts.c
ut_rts.o: ../../h/util.h
ut_rts.o: ../../h/config.h
ut_rts.o: ../../h/ll_log.h
ut_rts.o: ../../h/rtsparams.h
ut_rts.o: ../../h/chan.h
ut_rts.o: ../../h/table.h
ut_rts.o: ../../h/list_bpt.h
ut_rts.o: Rts84-types.h
x400in84.o: x400in84.c
x400in84.o: ../../h/head.h
x400in84.o: ../../h/util.h
x400in84.o: ../../h/config.h
x400in84.o: ../../h/ll_log.h
x400in84.o: ../../h/retcode.h
x400in84.o: ../../h/prm.h
x400in84.o: ../../h/dr.h
x400in84.o: ../../h/extension.h
x400in84.o: ../../h/mta.h
x400in84.o: ../../h/adr.h
x400in84.o: ../../h/list_rchan.h
x400in84.o: ../../h/chan.h
x400in84.o: ../../h/table.h
x400in84.o: ../../h/list_bpt.h
x400in84.o: ../../h/auth.h
x400in84.o: ../../h/list_bpt.h
x400in84.o: ../../h/extension.h
x400in84.o: ../../h/mta.h
x400in84.o: ../../h/aparse.h
x400in84.o: ../../h/ap.h
x400in84.o: ../../h/util.h
x400in84.o: ../../h/or.h
x400in84.o: ../../h/chan.h
x400in84.o: ../../h/auth.h
x400in84.o: ../../h/list_rchan.h
x400in84.o: ../../h/list_bpt.h
x400in84.o: ../../h/q.h
x400in84.o: ../../h/adr.h
x400in84.o: ../../h/mta.h
x400in84.o: Rts84-types.h
x400out84.o: x400out84.c
x400out84.o: ../../h/head.h
x400out84.o: ../../h/util.h
x400out84.o: ../../h/config.h
x400out84.o: ../../h/ll_log.h
x400out84.o: ../../h/retcode.h
x400out84.o: ../../h/sys.file.h
x400out84.o: ../../h/prm.h
x400out84.o: ../../h/q.h
x400out84.o: ../../h/adr.h
x400out84.o: ../../h/list_rchan.h
x400out84.o: ../../h/chan.h
x400out84.o: ../../h/table.h
x400out84.o: ../../h/list_bpt.h
x400out84.o: ../../h/auth.h
x400out84.o: ../../h/list_bpt.h
x400out84.o: ../../h/extension.h
x400out84.o: ../../h/mta.h
x400out84.o: ../../h/adr.h
x400out84.o: ../../h/list_bpt.h
x400out84.o: ../../h/aparse.h
x400out84.o: ../../h/ap.h
x400out84.o: ../../h/util.h
x400out84.o: ../../h/or.h
x400out84.o: ../../h/chan.h
x400out84.o: ../../h/auth.h
x400out84.o: ../../h/list_rchan.h
x400out84.o: ../../h/mta.h
x400out84.o: ../../h/qmgr.h
x400out84.o: ../../h/Qmgr-types.h
x400out84.o: ../../h/Qmgr-ops.h
x400out84.o: ../../h/dr.h
x400out84.o: ../../h/extension.h
x400out84.o: ../../h/mta.h
x400out84.o: Rts84-types.h
x400out84.o: ../../h/rtsparams.h
x400topp.o: x400topp.c
x400topp.o: ../../h/util.h
x400topp.o: ../../h/config.h
x400topp.o: ../../h/ll_log.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/table.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/q.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/list_rchan.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/auth.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/extension.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/list_bpt.h
x400topp.o: ../../h/aparse.h
x400topp.o: ../../h/ap.h
x400topp.o: ../../h/util.h
x400topp.o: ../../h/or.h
x400topp.o: ../../h/chan.h
x400topp.o: ../../h/auth.h
x400topp.o: ../../h/list_rchan.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/adr.h
x400topp.o: ../../h/or.h
x400topp.o: ../../h/prm.h
x400topp.o: ../../h/dr.h
x400topp.o: ../../h/extension.h
x400topp.o: ../../h/mta.h
x400topp.o: ../../h/retcode.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
