#  Build the PP database
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/dbmbuild/RCS/Makefile,v 6.0 1991/12/18 20:29:34 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:29:34  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  dbmbuild.c
OBJS =  dbmbuild.o
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

PROGS = xdbmbuild

default: $(PROGS)

dbmbuild: xdbmbuild
xdbmbuild: dbmbuild.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ dbmbuild.o $(LIBPP) $(LIBSYS)

dbmbuild.o: dbmbuild.c
	$(OLDCC) -c $(CFLAGS) dbmbuild.c

install: inst-dir inst-dbmbuild

inst-dbmbuild: $(CMDDIR)/dbmbuild
$(CMDDIR)/dbmbuild: xdbmbuild
	-$(BACKUP) $@ zxdbmbuild
	rm -f $@
	$(INSTALL) xdbmbuild $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "dbmbuild installed normally"; echo ""

inst-dir: $(CMDDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxdbmbuild *.old *.BAK

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
dbmbuild.o: dbmbuild.c
dbmbuild.o: ../../h/head.h
dbmbuild.o: ../../h/util.h
dbmbuild.o: ../../h/config.h
dbmbuild.o: ../../h/ll_log.h
dbmbuild.o: ../../h/retcode.h
dbmbuild.o: ../../h/util.h
dbmbuild.o: ../../h/chan.h
dbmbuild.o: ../../h/table.h
dbmbuild.o: ../../h/list_bpt.h
dbmbuild.o: ../../h/dbase.h
dbmbuild.o: ../../h/config.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
