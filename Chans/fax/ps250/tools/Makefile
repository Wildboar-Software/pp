# Fax Tools Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/fax/ps250/tools/RCS/Makefile,v 6.0 1991/12/18 20:08:25 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:08:25  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

PPBETA = ../../../..

PS250OBJS = ../error_codes.o ../ps250_basic.o ../ps250_intf.o \
	../ps250_manage.o ../ps250_stat.o ../ps250_trans.o ../ps250_util.o

HEADERS	=	$(PPBETA)/h
LIBPP	=	$(PPBETA)/Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS) $(PEPSYHEADERS)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = $(PPBETA)/Lib/llib-lpp.ln $(LINTISODE)

PROGS	 = xfax_print xfaxid xfaxenq1 xfaxsetid

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# Fax Print
#
############################################################

xfax_print:	fax_print.o $(PS250OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ \
	fax_print.o $(PS250OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Fax ID
#
############################################################

xfaxid:	faxid.o $(LIBPP) $(PS250OBJS)
	$(CC) $(LDFLAGS) -o $@ faxid.o $(PS250OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Fax enq1
#
############################################################

xfaxenq1:	faxenq1.o $(LIBPP) $(PS250OBJS)
	$(CC) $(LDFLAGS) -o $@ faxenq1.o $(PS250OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Fax set id
#
############################################################

xfaxsetid:	setid.o $(LIBPP) $(PS250OBJS)
	$(CC) $(LDFLAGS) -o $@ setid.o $(PS250OBJS) $(LIBPP) $(LIBSYS)

############################################################
#
# Misc
#
############################################################

clean:	tidy
	rm -f $(PROGS)

tidy:
	rm -f *.o core a.out Makefile.old

############################################################
#
# End of Building Rules
#
############################################################
