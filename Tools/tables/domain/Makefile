# t-getdomain = test getdomain
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/tables/domain/RCS/Makefile,v 6.0 1991/12/18 20:33:42 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:33:42  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= t-getdomain.c
OBJS	= t-getdomain.o

HEADERS	=	../../../h
LIBPP	=	../../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: xt-getdomain

xt-getdomain: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) \
		$(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old xt-getdomain

lint: l-targets

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
