# Makefile for Shell channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/shell/RCS/Makefile,v 6.0 1991/12/18 20:11:52 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:11:52  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  shell.c tb_getprog.c
OBJS =  shell.o tb_getprog.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   =       ../../Lib/libpp.a
INCLUDE =       -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS = xshell

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# Shell channel
#
############################################################

xshell: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-dir inst-shell

inst-shell: $(CHANDIR)/shell
$(CHANDIR)/shell: xshell
		-$(BACKUP) $@ zxshell
		rm -f $@
		$(INSTALL) xshell $@
		-$(CHOWN) $(ROOTUSER) $@
		-$(CHMOD) 4755 $@
		-@ls -ls $@
		-@echo "shell channel installed normally"; echo ""

inst-dir: $(CHANDIR)

clean:    tidy
	  rm -f $(OBJS)
tidy:
	rm -f core a.out *.old zxshell $(PROGS) *.BAK

lint: l-shell 

l-shell: $(SRCS) true
	 $(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

define:
	$(DEFINE) Makefile

true:;

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
shell.o: shell.c
shell.o: ../../h/util.h
shell.o: ../../h/config.h
shell.o: ../../h/ll_log.h
shell.o: ../../h/expand.h
shell.o: ../../h/head.h
shell.o: ../../h/util.h
shell.o: ../../h/retcode.h
shell.o: ../../h/qmgr.h
shell.o: ../../h/Qmgr-types.h
shell.o: ../../h/Qmgr-ops.h
shell.o: ../../h/q.h
shell.o: ../../h/adr.h
shell.o: ../../h/list_rchan.h
shell.o: ../../h/chan.h
shell.o: ../../h/table.h
shell.o: ../../h/list_bpt.h
shell.o: ../../h/auth.h
shell.o: ../../h/list_bpt.h
shell.o: ../../h/extension.h
shell.o: ../../h/mta.h
shell.o: ../../h/adr.h
shell.o: ../../h/list_bpt.h
shell.o: ../../h/aparse.h
shell.o: ../../h/ap.h
shell.o: ../../h/util.h
shell.o: ../../h/or.h
shell.o: ../../h/chan.h
shell.o: ../../h/auth.h
shell.o: ../../h/list_rchan.h
shell.o: ../../h/mta.h
shell.o: ../../h/dr.h
shell.o: ../../h/extension.h
shell.o: ../../h/mta.h
shell.o: ../../h/prm.h
shell.o: prog.h
tb_getprog.o: tb_getprog.c
tb_getprog.o: ../../h/util.h
tb_getprog.o: ../../h/config.h
tb_getprog.o: ../../h/ll_log.h
tb_getprog.o: ../../h/table.h
tb_getprog.o: prog.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
