# Makefile for the pp tsap daemon
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Src/pptsapd/RCS/Makefile,v 6.0 1991/12/18 20:27:31 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:27:31  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  pptsapd.c
OBJS =  pptsapd.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: xpptsapd

pptsapd: xpptsapd
xpptsapd: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS)
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

install: inst-dir $(CMDDIR)/pptsapd

$(CMDDIR)/pptsapd: xpptsapd
	-$(BACKUP) $@ zxpptsapd
	rm -f $@
	$(INSTALL) xpptsapd $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "pptsapd installed normally"; echo ""

xpptsapd-stat: pptsapd-stat.o
	$(CC) $(LDFLAGS) -o $@ pptsapd-stat.o $(LIBPP) $(LIBSYS)
inst-dir: $(CMDDIR)

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f xpptsapd zxpptsapd core a.out *.old *.BAK

lint: l-pptsapd
l-pptsapd: $(SRCS)
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

define:
	$(DEFINE) Makefile
depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
pptsapd.o: pptsapd.c
pptsapd.o: ../../h/util.h
pptsapd.o: ../../h/config.h
pptsapd.o: ../../h/ll_log.h
pptsapd.o: ../../h/chan.h
pptsapd.o: ../../h/table.h
pptsapd.o: ../../h/list_bpt.h
pptsapd.o: pptsapd.c
pptsapd.o: ../../h/util.h
pptsapd.o: ../../h/config.h
pptsapd.o: ../../h/ll_log.h
pptsapd.o: ../../h/chan.h
pptsapd.o: ../../h/table.h
pptsapd.o: ../../h/list_bpt.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
