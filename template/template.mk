# Preliminary comments
#
############################################################
#
# @(#) $Header$
#
# $Log$
#
############################################################
#
# Definitions
#
############################################################

SRCS	=
OBJS	=

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: default-targets

install:;

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS) $(LIBX) $(LM)

clean: tidy
tidy:
	rm -f $(OBJS) core a.out Makefile.old

lint: l-targets

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
