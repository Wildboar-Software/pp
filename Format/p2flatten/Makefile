#
# P2flattener -takes P2 split into directory structure 
#	      and flattens it to one file
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/p2flatten/RCS/Makefile,v 6.0 1991/12/18 20:20:12 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:20:12  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  flatten_only.c flatten_chan.c p2flatten.c
OBJS =  flatten_only.o flatten_chan.o p2flatten.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

CHAN_SRCS       = p2flatten.c flatten_chan.c
CHAN_OBJS       = p2flatten.o flatten_chan.o
ALONE_SRCS      = p2flatten.c flatten_only.c
ALONE_OBJS      = p2flatten.o flatten_only.o

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

default: $(SRCS) xflatten_chan

clean: tidy
	rm -f $(OBJS) *.o
tidy:
	rm -f xflatten_only xflatten_chan zxflatten_chan p2flatten \
		core a.out *.old *.BAK *.ph *.log

lint: l-p2flatten l-flatten-alone

l-p2flatten: $(CHAN_SRCS)
	$(LINT) $(LLFLAGS) $(CHAN_SRCS) $(LINTLIBS)

l-flatten-alone: $(ALONE_SRCS)
		 $(LINT) $(LLFLAGS) $(ALONE_SRCS) S(LINTLIBS)
define:
	$(DEFINE) Makefile

depend: $(SRCS)
	$(DEPEND) -I$(HEADERS) -I$(ISODEHDRS) $(SRCS)


xflatten_chan: $(CHAN_OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(CHAN_OBJS)  -o $@ $(LIBPP) $(LIBSYS)

alone: $(ALONE_OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(ALONE_OBJS)  -o xflatten_only \
	$(LIBPP) $(LIBSYS)

install: inst-dir inst-p2flatten

inst-p2flatten: $(CHANDIR)/p2flatten
$(CHANDIR)/p2flatten: xflatten_chan
	-$(BACKUP) $@ zxflatten_chan
	rm -f $@
	$(INSTALL) xflatten_chan $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "p2flatten channel installed normally"; echo ""

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
flatten_only.o: flatten_only.c
flatten_only.o: ../../h/retcode.h
flatten_only.o: ../../h/util.h
flatten_only.o: ../../h/config.h
flatten_only.o: ../../h/ll_log.h
flatten_chan.o: flatten_chan.c
flatten_chan.o: ../../h/util.h
flatten_chan.o: ../../h/config.h
flatten_chan.o: ../../h/ll_log.h
flatten_chan.o: ../../h/qmgr.h
flatten_chan.o: ../../h/Qmgr-types.h
flatten_chan.o: ../../h/Qmgr-ops.h
flatten_chan.o: ../../h/q.h
flatten_chan.o: ../../h/adr.h
flatten_chan.o: ../../h/list_rchan.h
flatten_chan.o: ../../h/chan.h
flatten_chan.o: ../../h/table.h
flatten_chan.o: ../../h/list_bpt.h
flatten_chan.o: ../../h/auth.h
flatten_chan.o: ../../h/list_bpt.h
flatten_chan.o: ../../h/extension.h
flatten_chan.o: ../../h/mta.h
flatten_chan.o: ../../h/adr.h
flatten_chan.o: ../../h/list_bpt.h
flatten_chan.o: ../../h/aparse.h
flatten_chan.o: ../../h/ap.h
flatten_chan.o: ../../h/util.h
flatten_chan.o: ../../h/or.h
flatten_chan.o: ../../h/chan.h
flatten_chan.o: ../../h/auth.h
flatten_chan.o: ../../h/list_rchan.h
flatten_chan.o: ../../h/mta.h
flatten_chan.o: ../../h/prm.h
flatten_chan.o: ../../h/retcode.h
flatten_chan.o: ../../h/sys.file.h
p2flatten.o: p2flatten.c
p2flatten.o: ../../h/util.h
p2flatten.o: ../../h/config.h
p2flatten.o: ../../h/ll_log.h
p2flatten.o: ../../h/retcode.h
p2flatten.o: ../../h/tb_bpt88.h
p2flatten.o: ../../h/IOB-types.h
p2flatten.o: ../../h/MTA-types.h
p2flatten.o: ../../h/Toks-types.h
p2flatten.o: ../../h/Auth-types.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
