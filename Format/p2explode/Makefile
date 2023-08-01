# P2explode -takes P2 and splits into directory structure
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/p2explode/RCS/Makefile,v 6.0 1991/12/18 20:20:02 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:20:02  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  p2explode.c explode_only.c explode_chan.c
OBJS =  p2explode.o explode_only.o expode_chan.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

CHAN_SRCS       = p2explode.c explode_chan.c
CHAN_OBJS       = p2explode.o explode_chan.o
ALONE_SRCS      = p2explode.c explode_only.c
ALONE_OBJS      = p2explode.o explode_only.o
HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: $(SRCS) xexplode_chan

clean: tidy
	rm -f $(OBJS) *.o 

tidy:
	rm -f zx* xexplode_chan core a.out *.old *.BAK *.ph *.log

lint: l-p2explode l-explode_only
l-p2explode: $(CHAN_SRCS)
	$(LINT) $(LLFLAGS) $(CHAN_SRCS) $(LINTLIBS)

l-explode_only: $(ALONE_SRCS)
	$(LINT) $(LLFLAGS) $(ALONE_SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile

depend: $(SRCS)
	$(DEPEND) -I$(HEADERS) -I$(ISODEHDRS) $(SRCS)


xexplode_chan: $(CHAN_OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(CHAN_OBJS)  -o $@ $(LIBPP) $(LIBSYS)

alone: $(ALONE_OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(ALONE_OBJS)  -o xexplode_only \
		$(LIBPP) $(LIBSYS)


install: inst-dir inst-p2explode

inst-p2explode: $(CHANDIR)/p2explode
$(CHANDIR)/p2explode: xexplode_chan
	-$(BACKUP) $@ zxexplode_chan
	rm -f $@
	$(INSTALL) xexplode_chan $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "The P2 explode channel installed normally"; echo ""

inst-dir: $(CHANDIR)

saber_src: $(CHAN_SRCS)
	#load -C $(CFLAGS) $(CHAN_SRCS) $(LIBPP) $(LIBSYS)

saber_obj:$(CHAN_OBJS)
	#load -C $(CHAN_OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
p2explode.o: p2explode.c
p2explode.o: ../../h/util.h
p2explode.o: ../../h/config.h
p2explode.o: ../../h/ll_log.h
p2explode.o: ../../h/retcode.h
p2explode.o: ../../h/tb_bpt88.h
p2explode.o: ../../h/IOB-types.h
p2explode.o: ../../h/MTA-types.h
p2explode.o: ../../h/Toks-types.h
p2explode.o: ../../h/Auth-types.h
p2explode.o: ../../h/q.h
p2explode.o: ../../h/adr.h
p2explode.o: ../../h/list_rchan.h
p2explode.o: ../../h/chan.h
p2explode.o: ../../h/table.h
p2explode.o: ../../h/list_bpt.h
p2explode.o: ../../h/auth.h
p2explode.o: ../../h/list_bpt.h
p2explode.o: ../../h/extension.h
p2explode.o: ../../h/mta.h
p2explode.o: ../../h/adr.h
p2explode.o: ../../h/list_bpt.h
p2explode.o: ../../h/aparse.h
p2explode.o: ../../h/ap.h
p2explode.o: ../../h/util.h
p2explode.o: ../../h/or.h
p2explode.o: ../../h/chan.h
p2explode.o: ../../h/auth.h
p2explode.o: ../../h/list_rchan.h
p2explode.o: ../../h/mta.h
p2explode.o: ../../h/list_bpt.h
explode_only.o: explode_only.c
explode_only.o: ../../h/util.h
explode_only.o: ../../h/config.h
explode_only.o: ../../h/ll_log.h
explode_only.o: ../../h/retcode.h
explode_only.o: ../../h/util.h
explode_chan.o: explode_chan.c
explode_chan.o: ../../h/util.h
explode_chan.o: ../../h/config.h
explode_chan.o: ../../h/ll_log.h
explode_chan.o: ../../h/qmgr.h
explode_chan.o: ../../h/Qmgr-types.h
explode_chan.o: ../../h/Qmgr-ops.h
explode_chan.o: ../../h/q.h
explode_chan.o: ../../h/adr.h
explode_chan.o: ../../h/list_rchan.h
explode_chan.o: ../../h/chan.h
explode_chan.o: ../../h/table.h
explode_chan.o: ../../h/list_bpt.h
explode_chan.o: ../../h/auth.h
explode_chan.o: ../../h/list_bpt.h
explode_chan.o: ../../h/extension.h
explode_chan.o: ../../h/mta.h
explode_chan.o: ../../h/adr.h
explode_chan.o: ../../h/list_bpt.h
explode_chan.o: ../../h/aparse.h
explode_chan.o: ../../h/ap.h
explode_chan.o: ../../h/util.h
explode_chan.o: ../../h/or.h
explode_chan.o: ../../h/chan.h
explode_chan.o: ../../h/auth.h
explode_chan.o: ../../h/list_rchan.h
explode_chan.o: ../../h/mta.h
explode_chan.o: ../../h/prm.h
explode_chan.o: ../../h/retcode.h
explode_chan.o: ../../h/tb_bpt88.h
explode_chan.o: ../../h/dr.h
explode_chan.o: ../../h/extension.h
explode_chan.o: ../../h/mta.h
explode_chan.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
