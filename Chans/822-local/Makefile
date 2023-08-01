# Local delivery channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/822-local/RCS/Makefile,v 6.0 1991/12/18 20:04:31 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:04:31  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =	ckmf.c code.c lex.c loc.c loc_child.c loc_parent.c parse_hdr.c
OBJS =

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
INCLUDE         = -I$(HEADERS)

CFLAGS          = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) $(INCLUDE)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xlocal xckmf

default: $(PROGS)

install: inst-dir inst-local inst-ckmf

############################################################
#
# Ckmf - check mailfilter file
#
############################################################

CHKOBJS = ckmf.o loc.o code.o parse_hdr.o
CHKSRCS = ckmf.c loc.c code.c parse_hdr.c

inst-ckmf: $(USRBINDIR)/ckmf

$(USRBINDIR)/ckmf: xckmf
	-$(BACKUP) $@ zxckmf
	rm -f $@
	$(INSTALL) xckmf $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "mailfilter check program installed normally"; echo ""   

ckmf: xckmf

xckmf: $(CHKOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(CHKOBJS) $(LIBPP) $(LIBSYS) -o $@

loc.o: lex.c

loc.o code.o: loc.h

loc.c: loc.y
	$(YACC) loc.y
	mv y.tab.c $@

l-ckmf: $(CHKSRCS) lex.c
	$(LINT) $(LLFLAGS) $(CHKSRCS) $(LINTLIBS)

############################################################
#
# local delivery channel
#
############################################################

LOCOBJS = loc_parent.o loc_child.o code.o loc.o parse_hdr.o
LOCSRCS = loc_parent.c loc_child.c code.c loc.c parse_hdr.c


inst-local: $(CHANDIR)/local
$(CHANDIR)/local: xlocal
	-$(BACKUP) $@ zxlocal
	rm -f $@
	$(INSTALL) xlocal $@
	-$(CHOWN) $(ROOTUSER) $@
	-$(CHMOD) 4755 $@
	-@ls -ls $@
	-@echo "local channel installed normally"; echo ""

local: xlocal

xlocal: $(LOCOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(LOCOBJS) $(LIBPP) $(LIBSYS) -o $@

saber_src: $(LOCSRCS)
	#load -C $(CFLAGS) $(LOCSRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(LOCOBJS)
	#load -C $(LOCOBJS) $(LIBPP) $(LIBSYS)

loc_parent.o: loc_parent.c ../../Make.defs
	$(CC) $(CFLAGS) -c -DUSRPATH='"$(USRBINDIR)"' loc_parent.c
l-local: $(LOCSRCS) lex.c
	$(LINT) $(LLFLAGS) $(LOCSRCS) $(LINTLIBS)

############################################################
#
# misc stuff
#
############################################################

clean:  tidy
	rm -f $(OBJS) *.o lex.c loc.c

tidy:
	rm -f $(PROGS) core a.out *old *.BAK zx*

lint: l-local l-ckmf



inst-dir: $(CHANDIR)

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
ckmf.o: ckmf.c
ckmf.o: ../../h/util.h
ckmf.o: ../../h/config.h
ckmf.o: ../../h/ll_log.h
ckmf.o: ../../h/expand.h
ckmf.o: ../../h/retcode.h
code.o: code.c
code.o: ../../h/util.h
code.o: ../../h/config.h
code.o: ../../h/ll_log.h
code.o: loc.h
code.o: ../../h/retcode.h
code.o: ../../h/expand.h
lex.o: lex.c
loc.o: loc.c
loc.o: ../../h/util.h
loc.o: ../../h/config.h
loc.o: ../../h/ll_log.h
loc.o: loc.h
loc.o: lex.c
loc_child.o: loc_child.c
loc_child.o: ../../h/util.h
loc_child.o: ../../h/config.h
loc_child.o: ../../h/ll_log.h
loc_child.o: ../../h/qmgr.h
loc_child.o: ../../h/Qmgr-types.h
loc_child.o: ../../h/Qmgr-ops.h
loc_child.o: ../../h/sys.file.h
loc_child.o: ../../h/retcode.h
loc_child.o: ../../h/expand.h
loc_child.o: ../../h/q.h
loc_child.o: ../../h/adr.h
loc_child.o: ../../h/list_rchan.h
loc_child.o: ../../h/chan.h
loc_child.o: ../../h/table.h
loc_child.o: ../../h/list_bpt.h
loc_child.o: ../../h/auth.h
loc_child.o: ../../h/list_bpt.h
loc_child.o: ../../h/extension.h
loc_child.o: ../../h/mta.h
loc_child.o: ../../h/adr.h
loc_child.o: ../../h/list_bpt.h
loc_child.o: ../../h/aparse.h
loc_child.o: ../../h/ap.h
loc_child.o: ../../h/util.h
loc_child.o: ../../h/or.h
loc_child.o: ../../h/chan.h
loc_child.o: ../../h/auth.h
loc_child.o: ../../h/list_rchan.h
loc_child.o: ../../h/mta.h
loc_child.o: ../../h/loc_user.h
loc_child.o: ../../h/ap.h
loc_parent.o: loc_parent.c
loc_parent.o: ../../h/util.h
loc_parent.o: ../../h/config.h
loc_parent.o: ../../h/ll_log.h
loc_parent.o: ../../h/retcode.h
loc_parent.o: ../../h/prm.h
loc_parent.o: ../../h/q.h
loc_parent.o: ../../h/adr.h
loc_parent.o: ../../h/list_rchan.h
loc_parent.o: ../../h/chan.h
loc_parent.o: ../../h/table.h
loc_parent.o: ../../h/list_bpt.h
loc_parent.o: ../../h/auth.h
loc_parent.o: ../../h/list_bpt.h
loc_parent.o: ../../h/extension.h
loc_parent.o: ../../h/mta.h
loc_parent.o: ../../h/adr.h
loc_parent.o: ../../h/list_bpt.h
loc_parent.o: ../../h/aparse.h
loc_parent.o: ../../h/ap.h
loc_parent.o: ../../h/util.h
loc_parent.o: ../../h/or.h
loc_parent.o: ../../h/chan.h
loc_parent.o: ../../h/auth.h
loc_parent.o: ../../h/list_rchan.h
loc_parent.o: ../../h/mta.h
loc_parent.o: ../../h/dr.h
loc_parent.o: ../../h/extension.h
loc_parent.o: ../../h/mta.h
loc_parent.o: ../../h/qmgr.h
loc_parent.o: ../../h/Qmgr-types.h
loc_parent.o: ../../h/Qmgr-ops.h
loc_parent.o: ../../h/loc_user.h
parse_hdr.o: parse_hdr.c
parse_hdr.o: ../../h/util.h
parse_hdr.o: ../../h/config.h
parse_hdr.o: ../../h/ll_log.h
parse_hdr.o: ../../h/expand.h
parse_hdr.o: loc.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
