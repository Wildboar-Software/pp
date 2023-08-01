# List maintance tool
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/mlist/RCS/Makefile,v 6.0 1991/12/18 20:31:28 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:31:28  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =mlist.c
OBJS =mlist.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   =       ../../Lib/libpp.a
INCLUDE = -I$(HEADERS)
CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS=xmlist

default: $(PROGS)

mlist:xmlist

xmlist: $(OBJS) $(LIBPP)
	rm -f $@
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)
	chmod u+s $@

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lint: l-mlist

l-mlist: $(SRCS)
	 $(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

install: inst-dir inst-mlist

inst-dir: $(USRBINDIR)

inst-mlist: $(USRBINDIR)/mlist

$(USRBINDIR)/mlist: xmlist
	-$(BACKUP) $@ zxmlist
	-rm -f $@
	$(INSTALL) xmlist $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-$(CHMOD) u+s $@
	-rm -f $(USRBINDIR)/malias
	ln -s $@ $(USRBINDIR)/malias
	-@ls -ls $@ $(USRBINDIR)/malias
	-@echo "mlist and malias installed normally"; echo ""

clean: tidy
	rm -f $(OBJS)
tidy:
	rm -f $(PROGS) core a.out Makefile.old zxmlist

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
mlist.o: mlist.c
mlist.o: ../../h/head.h
mlist.o: ../../h/util.h
mlist.o: ../../h/config.h
mlist.o: ../../h/ll_log.h
mlist.o: ../../h/retcode.h
mlist.o: ../../h/adr.h
mlist.o: ../../h/list_rchan.h
mlist.o: ../../h/chan.h
mlist.o: ../../h/table.h
mlist.o: ../../h/list_bpt.h
mlist.o: ../../h/auth.h
mlist.o: ../../h/list_bpt.h
mlist.o: ../../h/extension.h
mlist.o: ../../h/mta.h
mlist.o: ../../h/adr.h
mlist.o: ../../h/list_bpt.h
mlist.o: ../../h/or.h
mlist.o: ../../h/dl.h
mlist.o: ../../h/alias.h
mlist.o: ../../h/chan.h
mlist.o: ../../h/retcode.h
mlist.o: ../../h/ap.h
mlist.o: ../../h/util.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
