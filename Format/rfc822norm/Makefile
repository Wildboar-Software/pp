# Makefile for rfc822 normalizing filter
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/rfc822norm/RCS/Makefile,v 6.0 1991/12/18 20:20:54 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:20:54  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  rfc822norm.c
OBJS =  rfc822norm.o 
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a

CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS) -DAP_DEBUG
LDFLAGS         = $(LDOPTIONS) $(LLF)

LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS  = xrfc822norm

default: $(PROGS)

xrfc822norm:    $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBPP) $(LIBSYS)

install: inst-dir inst-rfc822norm

inst-rfc822norm: $(FORMDIR)/rfc822norm

$(FORMDIR)/rfc822norm: xrfc822norm
		     -$(BACKUP) $@ zxrfc822norm
		     rm -f $@           
		     $(INSTALL) xrfc822norm $@
		     -$(CHMOD) $(PGMPROT) $@
		     -$(CHOWN) $(PPUSER) $@
		     -@ls -ls $@
		     -@echo "rfc822norm filter installed normally"; echo ""

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

inst-dir: $(CHANDIR)

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f $(PROGS) core a.out *.old *.BAK zxrfc822norm


lint: l-rfc822norm

l-rfc822norm: $(SRCS)
	      $(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
rfc822norm.o: rfc822norm.c
rfc822norm.o: ../../h/util.h
rfc822norm.o: ../../h/config.h
rfc822norm.o: ../../h/ll_log.h
rfc822norm.o: ../../h/ap.h
rfc822norm.o: ../../h/util.h
rfc822norm.o: ../../h/alias.h
rfc822norm.o: ../../h/chan.h
rfc822norm.o: ../../h/table.h
rfc822norm.o: ../../h/list_bpt.h
rfc822norm.o: ../../h/retcode.h
rfc822norm.o: ../../h/adr.h
rfc822norm.o: ../../h/list_rchan.h
rfc822norm.o: ../../h/chan.h
rfc822norm.o: ../../h/auth.h
rfc822norm.o: ../../h/list_bpt.h
rfc822norm.o: ../../h/extension.h
rfc822norm.o: ../../h/mta.h
rfc822norm.o: ../../h/adr.h
rfc822norm.o: ../../h/list_bpt.h
rfc822norm.o: ../../h/aparse.h
rfc822norm.o: ../../h/ap.h
rfc822norm.o: ../../h/or.h
rfc822norm.o: ../../h/chan.h
rfc822norm.o: ../../h/auth.h
rfc822norm.o: ../../h/list_rchan.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
