# Makefile for ckaddress program
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ckadr/RCS/Makefile,v 6.0 1991/12/18 20:28:46 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:28:46  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ckadr.c
OBJS =  ckadr.o

HEADERS = ../../h -I/usr/include/isode
LIBPP   = ../../Lib/libpp.a
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) -u $(INCLUDE)
LINTLIBS = $(LINTISODE) ../../Lib/llib-lpp.ln

PROGS = xckadr

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# CheckAdr
#
############################################################

ckadr: xckadr

xckadr: $(OBJS) $(LIBPP)
	    rm -f $@
	    $(CC) $(LDFLAGS) -o $@ $(OBJS)\
		  $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc stuff
#
############################################################


install: inst-dir inst-ckadr
inst-ckadr: $(TOOLDIR)/ckadr
$(TOOLDIR)/ckadr: xckadr
	-$(BACKUP) $@ zxckadr
	rm -f $@
	$(INSTALL) xckadr $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "ckadr tool installed normally"; echo ""

inst-dir: $(TOOLDIR)


clean:  tidy
	rm -f $(OBJS)

tidy:
	rm -f core a.out *.old $(PROGS) *.BAK zx* 


lint: l-ckadr

l-ckadr: $(SRCS) true
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

true:;

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
ckadr.o: ckadr.c
ckadr.o: ../../h/util.h
ckadr.o: ../../h/config.h
ckadr.o: ../../h/ll_log.h
ckadr.o: ../../h/adr.h
ckadr.o: ../../h/list_rchan.h
ckadr.o: ../../h/chan.h
ckadr.o: ../../h/table.h
ckadr.o: ../../h/list_bpt.h
ckadr.o: ../../h/auth.h
ckadr.o: ../../h/list_bpt.h
ckadr.o: ../../h/extension.h
ckadr.o: ../../h/mta.h
ckadr.o: ../../h/adr.h
ckadr.o: ../../h/list_bpt.h
ckadr.o: ../../h/aparse.h
ckadr.o: ../../h/ap.h
ckadr.o: ../../h/util.h
ckadr.o: ../../h/or.h
ckadr.o: ../../h/chan.h
ckadr.o: ../../h/auth.h
ckadr.o: ../../h/list_rchan.h
ckadr.o: ../../h/retcode.h
ckadr.o: ../../h/list_rchan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
