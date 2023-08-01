# Makefile for UUCP channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/uucp/RCS/Makefile,v 6.0 1991/12/18 20:13:06 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:13:06  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  uucp_out.c rfc8222uu.c rmail.c
OBJS =  uucp_out.o rfc8222uu.o rmail.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   =       ../../Lib/libpp.a
INCLUDE =       -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS = xuucp_out xrmail

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# UUCP out channel
#
############################################################

xuucp_out: rfc8222uu.o uucp_out.o $(LIBPP)
	   $(CC) $(LDFLAGS) -o $@ rfc8222uu.o uucp_out.o $(LIBPP) $(LIBSYS)

xt-rfc8222uu: rfc8222uu.o t-rfc8222uu.o $(LIBPP)
	     $(CC) $(LDFLAGS) -o $@ rfc8222uu.o t-rfc8222uu.o $(LIBPP) $(LIBSYS)

saber_src: rfc8222uu.c uucp_out.c
	#load -C $(CFLAGS) rfc8222uu.c uucp_out.c $(LIBPP) $(LIBSYS)

install: inst-dir inst-uucp-out inst-uucp-in
inst-uucp-out: $(CHANDIR)/uucp-out
$(CHANDIR)/uucp-out: xuucp_out
		-$(BACKUP) $@ zxuucp_out
		rm -f $@
		$(INSTALL) xuucp_out $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "UUCP outbound channel installed normally"; echo ""

inst-uucp-in: $(CHANDIR)/rmail
$(CHANDIR)/rmail: xrmail
		-$(BACKUP) $@ zxrmail
		rm -f $@
		$(INSTALL) xrmail $@
		-$(CHOWN) $(PPUSER) $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHMOD) u+s $@
		-@ls -ls $@
		-@echo "UUCP inbound channel (rmail) installed normally"; echo ""

inst-dir: $(CHANDIR)

############################################################
#
# UUCP in channel
#
############################################################

xrmail: rmail.o $(LIBPP)
	   $(CC) $(LDFLAGS) -o $@ rmail.o $(LIBPP) $(LIBSYS)

clean:    tidy
	  rm -f $(OBJS)
tidy:
	rm -f core a.out *.old zxuucp_out $(PROGS) *.BAK

lint: l-uucp-out l-rmail

l-uucp-out: rfc8222uu.c uucp_out.c true
	    $(LINT) $(LLFLAGS) rfc8222uu.c uucp_out.c $(LINTLIBS)

l-rmail: rmail.c true
	    $(LINT) $(LLFLAGS) rmail.c $(LINTLIBS)

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
uucp_out.o: uucp_out.c
uucp_out.o: ../../h/util.h
uucp_out.o: ../../h/config.h
uucp_out.o: ../../h/ll_log.h
uucp_out.o: ../../h/head.h
uucp_out.o: ../../h/util.h
uucp_out.o: ../../h/retcode.h
uucp_out.o: ../../h/qmgr.h
uucp_out.o: ../../h/Qmgr-types.h
uucp_out.o: ../../h/Qmgr-ops.h
uucp_out.o: ../../h/q.h
uucp_out.o: ../../h/adr.h
uucp_out.o: ../../h/list_rchan.h
uucp_out.o: ../../h/chan.h
uucp_out.o: ../../h/table.h
uucp_out.o: ../../h/list_bpt.h
uucp_out.o: ../../h/auth.h
uucp_out.o: ../../h/list_bpt.h
uucp_out.o: ../../h/extension.h
uucp_out.o: ../../h/mta.h
uucp_out.o: ../../h/adr.h
uucp_out.o: ../../h/list_bpt.h
uucp_out.o: ../../h/aparse.h
uucp_out.o: ../../h/ap.h
uucp_out.o: ../../h/util.h
uucp_out.o: ../../h/or.h
uucp_out.o: ../../h/chan.h
uucp_out.o: ../../h/auth.h
uucp_out.o: ../../h/list_rchan.h
uucp_out.o: ../../h/mta.h
uucp_out.o: ../../h/dr.h
uucp_out.o: ../../h/extension.h
uucp_out.o: ../../h/mta.h
uucp_out.o: ../../h/prm.h
rfc8222uu.o: rfc8222uu.c
rfc8222uu.o: ../../h/util.h
rfc8222uu.o: ../../h/config.h
rfc8222uu.o: ../../h/ll_log.h
rfc8222uu.o: ../../h/chan.h
rfc8222uu.o: ../../h/table.h
rfc8222uu.o: ../../h/list_bpt.h
rfc8222uu.o: ../../h/ap.h
rfc8222uu.o: ../../h/util.h
rfc8222uu.o: ../../h/adr.h
rfc8222uu.o: ../../h/list_rchan.h
rfc8222uu.o: ../../h/chan.h
rfc8222uu.o: ../../h/auth.h
rfc8222uu.o: ../../h/list_bpt.h
rfc8222uu.o: ../../h/extension.h
rfc8222uu.o: ../../h/mta.h
rfc8222uu.o: ../../h/adr.h
rfc8222uu.o: ../../h/list_bpt.h
rfc8222uu.o: ../../h/aparse.h
rfc8222uu.o: ../../h/ap.h
rfc8222uu.o: ../../h/or.h
rfc8222uu.o: ../../h/chan.h
rfc8222uu.o: ../../h/auth.h
rfc8222uu.o: ../../h/list_rchan.h
rmail.o: rmail.c
rmail.o: ../../h/util.h
rmail.o: ../../h/config.h
rmail.o: ../../h/ll_log.h
rmail.o: ../../h/head.h
rmail.o: ../../h/util.h
rmail.o: ../../h/retcode.h
rmail.o: ../../h/q.h
rmail.o: ../../h/adr.h
rmail.o: ../../h/list_rchan.h
rmail.o: ../../h/chan.h
rmail.o: ../../h/table.h
rmail.o: ../../h/list_bpt.h
rmail.o: ../../h/auth.h
rmail.o: ../../h/list_bpt.h
rmail.o: ../../h/extension.h
rmail.o: ../../h/mta.h
rmail.o: ../../h/adr.h
rmail.o: ../../h/list_bpt.h
rmail.o: ../../h/aparse.h
rmail.o: ../../h/ap.h
rmail.o: ../../h/util.h
rmail.o: ../../h/or.h
rmail.o: ../../h/chan.h
rmail.o: ../../h/auth.h
rmail.o: ../../h/list_rchan.h
rmail.o: ../../h/mta.h
rmail.o: ../../h/prm.h
rmail.o: ../../h/ap.h
rmail.o: ../../h/adr.h
rmail.o: ../../h/chan.h
rmail.o: ../../h/retcode.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
