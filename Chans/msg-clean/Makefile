# Makefile for msg_clean channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/msg-clean/RCS/Makefile,v 6.0 1991/12/18 20:11:18 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:11:18  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  msg-clean.c 
OBJS =  msg-clean.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS   = xmsg_clean

############################################################
#
# Building Rules
#
############################################################

default: msg_clean

############################################################
#
# Msg clean out
#
############################################################

msg_clean: xmsg-clean

xmsg-clean: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

sh_rmmsg: ;

############################################################
#
# Misc stuff
#
############################################################

install: inst-msg-clean

inst-msg-clean: inst-dir $(CHANDIR)/msg-clean
$(CHANDIR)/msg-clean:xmsg-clean
	-$(BACKUP) $@ zxmsg-clean
	rm -f $@
	$(INSTALL) xmsg-clean $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "msg-clean channel installed normally"; echo ""

$(CHANDIR)/sh_rmmsg: sh_rmmsg inst-dir
	$(INSTALL) sh_rmmsg $@

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

inst-dir: $(CHANDIR)

clean: tidy
	rm -f $(OBJS)
tidy:
	rm -f core a.out Makefile.old xmsg-clean zxmsg-clean *.BAK

lint: l-msg_clean

l-msg_clean: $(SRCS) true
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

true:;

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
msg-clean.o: msg-clean.c
msg-clean.o: ../../h/util.h
msg-clean.o: ../../h/config.h
msg-clean.o: ../../h/ll_log.h
msg-clean.o: ../../h/head.h
msg-clean.o: ../../h/util.h
msg-clean.o: ../../h/retcode.h
msg-clean.o: ../../h/qmgr.h
msg-clean.o: ../../h/Qmgr-types.h
msg-clean.o: ../../h/Qmgr-ops.h
msg-clean.o: ../../h/q.h
msg-clean.o: ../../h/adr.h
msg-clean.o: ../../h/list_rchan.h
msg-clean.o: ../../h/chan.h
msg-clean.o: ../../h/table.h
msg-clean.o: ../../h/list_bpt.h
msg-clean.o: ../../h/auth.h
msg-clean.o: ../../h/list_bpt.h
msg-clean.o: ../../h/extension.h
msg-clean.o: ../../h/mta.h
msg-clean.o: ../../h/adr.h
msg-clean.o: ../../h/list_bpt.h
msg-clean.o: ../../h/aparse.h
msg-clean.o: ../../h/ap.h
msg-clean.o: ../../h/util.h
msg-clean.o: ../../h/or.h
msg-clean.o: ../../h/chan.h
msg-clean.o: ../../h/auth.h
msg-clean.o: ../../h/list_rchan.h
msg-clean.o: ../../h/mta.h
msg-clean.o: ../../h/prm.h
msg-clean.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
