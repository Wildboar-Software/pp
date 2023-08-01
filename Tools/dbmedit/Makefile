#  Edits the PP database
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/dbmedit/RCS/Makefile,v 6.0 1991/12/18 20:29:41 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:29:41  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  dbmedit.c
OBJS =  dbmedit.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS		= $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

PROGS = xdbmedit

default: $(PROGS)

dbmedit: xdbmedit
xdbmedit: dbmedit.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ dbmedit.o $(LIBPP) $(LIBSYS)

dbmedit.o: dbmedit.c
	$(OLDCC) -c $(CFLAGS) dbmedit.c

install: inst-dir inst-dbmedit

inst-dbmedit: $(CMDDIR)/dbmedit
$(CMDDIR)/dbmedit: xdbmedit
	-$(BACKUP) $@ zxdbmedit
	rm -f $@
	$(INSTALL) xdbmedit $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "dbmedit installed normally"; echo ""

inst-dir: $(CMDDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxdbmedit *.old *.BAK

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
dbmedit.o: dbmedit.c
dbmedit.o: ../../h/head.h
dbmedit.o: ../../h/util.h
dbmedit.o: ../../h/config.h
dbmedit.o: ../../h/ll_log.h
dbmedit.o: ../../h/retcode.h
dbmedit.o: ../../h/util.h
dbmedit.o: ../../h/dbase.h
dbmedit.o: ../../h/config.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
