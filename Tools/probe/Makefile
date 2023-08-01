#  Build the probe utility
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/probe/RCS/Makefile,v 6.0 1991/12/18 20:32:41 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:32:41  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  p_stdin.c p_ut.c probe.c
OBJS =  p_stdin.o p_ut.o probe.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS		= $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xprobe

default: $(PROGS)

probe: xprobe
xprobe: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)


install: inst-dir inst-probe

inst-probe: $(TOOLDIR)/probe
$(TOOLDIR)/probe: xprobe
	-$(BACKUP) $@ zxprobe
	rm -f $@
	$(INSTALL) xprobe $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "probe installed normally"; echo ""

inst-dir: $(TOOLDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxprobe *.old *.BAK

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
p_stdin.o: p_stdin.c
p_stdin.o: ../../h/util.h
p_stdin.o: ../../h/config.h
p_stdin.o: ../../h/ll_log.h
p_stdin.o: ../../h/q.h
p_stdin.o: ../../h/adr.h
p_stdin.o: ../../h/list_rchan.h
p_stdin.o: ../../h/chan.h
p_stdin.o: ../../h/table.h
p_stdin.o: ../../h/list_bpt.h
p_stdin.o: ../../h/auth.h
p_stdin.o: ../../h/list_bpt.h
p_stdin.o: ../../h/extension.h
p_stdin.o: ../../h/mta.h
p_stdin.o: ../../h/adr.h
p_stdin.o: ../../h/list_bpt.h
p_stdin.o: ../../h/mta.h
p_stdin.o: ../../h/tb_q.h
p_ut.o: p_ut.c
p_ut.o: ../../h/util.h
p_ut.o: ../../h/config.h
p_ut.o: ../../h/ll_log.h
p_ut.o: ../../h/q.h
p_ut.o: ../../h/adr.h
p_ut.o: ../../h/list_rchan.h
p_ut.o: ../../h/chan.h
p_ut.o: ../../h/table.h
p_ut.o: ../../h/list_bpt.h
p_ut.o: ../../h/auth.h
p_ut.o: ../../h/list_bpt.h
p_ut.o: ../../h/extension.h
p_ut.o: ../../h/mta.h
p_ut.o: ../../h/adr.h
p_ut.o: ../../h/list_bpt.h
p_ut.o: ../../h/mta.h
p_ut.o: ../../h/tb_bpt84.h
p_ut.o: ../../h/tb_p1.h
p_ut.o: ../../h/retcode.h
probe.o: probe.c
probe.o: ../../h/head.h
probe.o: ../../h/util.h
probe.o: ../../h/config.h
probe.o: ../../h/ll_log.h
probe.o: ../../h/retcode.h
probe.o: ../../h/prm.h
probe.o: ../../h/q.h
probe.o: ../../h/adr.h
probe.o: ../../h/list_rchan.h
probe.o: ../../h/chan.h
probe.o: ../../h/table.h
probe.o: ../../h/list_bpt.h
probe.o: ../../h/auth.h
probe.o: ../../h/list_bpt.h
probe.o: ../../h/extension.h
probe.o: ../../h/mta.h
probe.o: ../../h/adr.h
probe.o: ../../h/list_bpt.h
probe.o: ../../h/mta.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
