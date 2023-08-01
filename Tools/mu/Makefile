# MTS user interface for PP     ---->  mu  <----
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/mu/RCS/Makefile,v 6.0 1991/12/18 20:31:54 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:31:54  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  argh.c mu.c
OBJS =  argh.o mu.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS		= $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIB		= ../../Lib/llib-lpp.ln

############################################################
#
# Building Rules
#
############################################################

PROGS = xmu

default: $(PROGS)

mu:     xmu

xmu:    $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o xmu $(OBJS) \
	$(LIBPP) $(LIBSYS) 


install: inst-dir inst-mu
inst-mu: $(TOOLDIR)/mu
$(TOOLDIR)/mu: xmu
	-$(BACKUP) $@ zxmu
	rm -f $@
	$(INSTALL) xmu $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Mu installed normally"; echo ""


inst-dir: $(TOOLDIR)


lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f core $(PROGS) zxmu *.old *.BAK

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
argh.o: argh.c
argh.o: ../../h/argh.h
argh.o: ../../h/util.h
argh.o: ../../h/config.h
argh.o: ../../h/ll_log.h
mu.o: mu.c
mu.o: ../../h/head.h
mu.o: ../../h/util.h
mu.o: ../../h/config.h
mu.o: ../../h/ll_log.h
mu.o: ../../h/retcode.h
mu.o: ../../h/util.h
mu.o: ../../h/q.h
mu.o: ../../h/adr.h
mu.o: ../../h/list_rchan.h
mu.o: ../../h/chan.h
mu.o: ../../h/table.h
mu.o: ../../h/list_bpt.h
mu.o: ../../h/auth.h
mu.o: ../../h/list_bpt.h
mu.o: ../../h/extension.h
mu.o: ../../h/mta.h
mu.o: ../../h/adr.h
mu.o: ../../h/list_bpt.h
mu.o: ../../h/aparse.h
mu.o: ../../h/ap.h
mu.o: ../../h/util.h
mu.o: ../../h/or.h
mu.o: ../../h/chan.h
mu.o: ../../h/auth.h
mu.o: ../../h/list_rchan.h
mu.o: ../../h/mta.h
mu.o: ../../h/prm.h
mu.o: ../../h/tb_prm.h
mu.o: ../../h/tb_q.h
mu.o: ../../h/tb_a.h
mu.o: ../../h/tb_dr.h
mu.o: ../../h/argh.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
