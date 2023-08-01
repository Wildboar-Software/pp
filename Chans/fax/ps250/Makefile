# dexNet200 fax driver Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/fax/ps250/RCS/Makefile,v 6.0 1991/12/18 20:07:26 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:07:26  jpo
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

PPINSRCS = ../fax_in.c
PPINOBJS = ../fax_in.o

SRCS	= ps250_intf.c ps250_util.c \
	ps250_stat.c ps250_manage.c ps250_trans.c \
	ps250_basic.c error_codes.c
OBJS	= ps250_intf.o ps250_util.o \
	ps250_stat.o ps250_manage.o ps250_trans.o \
	ps250_basic.o error_codes.o

HEADERS	=	../../../h
LIBPP	=	../../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../../Lib/llib-lpp.ln $(LINTISODE)

PROGS	= xfax xfaxd

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

install: inst-dir inst-out

inst-dir: $(CHANDIR)

lint: l-out l-in

############################################################
#
# Fax channel
#
############################################################

xfax: $(OBJS) $(PPOUTOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(PPOUTOBJS) $(LIBPP) $(LIBSYS)

saber_out_src:	$(PPOUTSRCS) $(SRCS)
	#load -C $(CFLAGS) $(PPOUTSRCS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_out_obj: $(OBJS) $(PPOUTOBJS)
        #load -C $(CFLAGS) $(OBJS) $(PPOUTOBJS) $(LIBPP) $(LIBSYS)

inst-out: $(CHANDIR)/ps250
$(CHANDIR)/ps250: xfax
	-$(BACKUP) $@ zxfax
	rm -f $@
	$(INSTALL) xfax $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Panasonic 250 outbound fax channel installed normally"; echo ""

l-out: $(SRCS) $(PPOUTSRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(PPOUTSRCS) $(LINTLIBS)

############################################################
#
# Fax daemon
#
############################################################

xfaxd: $(OBJS) $(PPINOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(PPINOBJS) $(LIBPP) $(LIBSYS)

saber_in_src:	$(SRCS) $(PPINSRCS)
	#load -C $(CFLAGS) $(SRCS) $(PPINSRCS) $(LIBPP) $(LIBSYS)

saber_in_obj: $(OBJS) $(PPINOBJS)
        #load -C $(CFLAGS) $(OBJS) $(PPINOBJS) $(LIBPP) $(LIBSYS)

inst-out: $(CHANDIR)/ps250d
$(CHANDIR)/ps250d: xfaxd
	-$(BACKUP) $@ zxfaxd
	rm -f $@
	$(INSTALL) xfaxd $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Panasonic 250 inbound fax daemon installed normally"; echo ""

l-in: $(SRCS) $(PPINSRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(PPINSRCS) $(LINTLIBS)

############################################################
#
# Misc
#
############################################################

clean:	tidy
	(echo "cd tools; $(MAKE) $@"; cd tools; $(MAKE) $@)
	rm -f $(PROGS)

tidy:
	(echo "cd tools; $(MAKE) $@"; cd tools; $(MAKE) $@)
	rm -f $(OBJS) core a.out Makefile.old

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
