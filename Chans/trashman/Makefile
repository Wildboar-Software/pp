# Makefile for trash channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/trashman/RCS/Makefile,v 6.0 1991/12/18 20:12:57 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:57  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  trash.c
OBJS =  trash.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS) # -DDBG_CHAN
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS   = xtrash

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# trash clean out
#
############################################################

trash: xtrash

xtrash:        $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################


install: inst-dir inst-trash

inst-trash: $(CHANDIR)/trash
$(CHANDIR)/trash: xtrash
	-$(BACKUP) $@ zxtrash
	rm -f $@
	$(INSTALL) xtrash $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "trash channel installed normally"; echo ""

inst-dir: $(CHANDIR)


clean: tidy
	rm -f $(OBJS)
tidy:
	rm -f $(PROGS) zxtrash core a.out Makefile.old *.BAK

lint: l-trash_channel

l-trash_channel: $(SRCS) true
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
trash.o: trash.c
trash.o: ../../h/head.h
trash.o: ../../h/util.h
trash.o: ../../h/config.h
trash.o: ../../h/ll_log.h
trash.o: ../../h/retcode.h
trash.o: ../../h/qmgr.h
trash.o: ../../h/Qmgr-types.h
trash.o: ../../h/Qmgr-ops.h
trash.o: ../../h/q.h
trash.o: ../../h/adr.h
trash.o: ../../h/list_rchan.h
trash.o: ../../h/chan.h
trash.o: ../../h/table.h
trash.o: ../../h/list_bpt.h
trash.o: ../../h/auth.h
trash.o: ../../h/list_bpt.h
trash.o: ../../h/extension.h
trash.o: ../../h/mta.h
trash.o: ../../h/adr.h
trash.o: ../../h/list_bpt.h
trash.o: ../../h/aparse.h
trash.o: ../../h/ap.h
trash.o: ../../h/util.h
trash.o: ../../h/or.h
trash.o: ../../h/chan.h
trash.o: ../../h/auth.h
trash.o: ../../h/list_rchan.h
trash.o: ../../h/mta.h
trash.o: ../../h/prm.h
trash.o: ../../h/sys.file.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
