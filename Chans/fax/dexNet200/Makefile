# dexNet200 fax driver Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/fax/dexNet200/RCS/Makefile,v 6.0 1991/12/18 20:08:53 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:08:53  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

PPOUTSRCS = ../fax_out.c
PPOUTOBJS = ../fax_out.o

OUTSRCS	= dexnet_intf.c dexnet_util.c dexnet_conv.c
OUTOBJS	= dexnet_intf.o dexnet_util.o dexnet_conv.o

HEADERS	=	../../../h
LIBPP	=	../../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../../Lib/llib-lpp.ln $(LINTISODE)

PROGS	= xfax

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

install: inst-dir inst-out

inst-dir: $(CHANDIR)

lint: l-out

############################################################
#
# Fax channel
#
############################################################

xfax: $(OUTOBJS) $(PPOUTOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OUTOBJS) $(PPOUTOBJS) $(LIBPP) $(LIBSYS)

saber_src: $(OUTSRCS) $(PPOUTSRCS)
        #load -C $(CFLAGS) $(OUTSRCS) $(PPOUTSRCS) $(LIBPP) $(LIBSYS)

inst-out: $(CHANDIR)/dexNet200
$(CHANDIR)/dexNet200: xfax
	-$(BACKUP) $@ zxfax
	rm -f $@
	$(INSTALL) xfax $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "dexNet200 outbound fax channel installed normally"; echo ""
clean:	tidy
	rm -f $(PROGS)

tidy:
	rm -f $(OUTOBJS) core a.out Makefile.old

l-out: $(OUTSRCS) $(PPOUTSRCS)
	$(LINT) $(LLFLAGS) $(OUTSRCS) $(PPOUTSRCS) $(LINTLIBS)

depend:
	$(DEPEND) -I$(HEADERS) $(OUTSRCS)

############################################################
#
# End of Building Rules
#
############################################################
