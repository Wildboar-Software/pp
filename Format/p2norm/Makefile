# P2 normalisation including P22 to P2 downgrade 
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/p2norm/RCS/Makefile,v 6.0 1991/12/18 20:20:24 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:20:24  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= p2norm.c
OBJS	= p2norm.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

#LIBDSAP =	-ldsap
#LIBASN1 =	../../Chans/x40088/asn1/libasn1.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xp2norm

default: $(PROGS)

xp2norm: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBPP) \
	$(LIBDSAP) $(LIBSYS) -o $@

install: inst-p2norm

inst-p2norm: $(FORMDIR)/p2norm
$(FORMDIR)/p2norm: xp2norm
	-$(BACKUP) $@ zp2norm
	rm -f $@
	$(INSTALL) xp2norm $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "p2 normalisation filter installed normally"; echo ""

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f $(PROGS) core a.out Makefile.old


saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lint: l-targets

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
p2norm.o: p2norm.c
p2norm.o: ../../h/head.h
p2norm.o: ../../h/util.h
p2norm.o: ../../h/config.h
p2norm.o: ../../h/ll_log.h
p2norm.o: ../../h/retcode.h
p2norm.o: ../../h/util.h
p2norm.o: ../../h/IOB-types.h
p2norm.o: ../../h/MTA-types.h
p2norm.o: ../../h/Toks-types.h
p2norm.o: ../../h/Auth-types.h
p2norm.o: ../../h/or.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
