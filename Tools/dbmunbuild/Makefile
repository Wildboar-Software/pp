#  Unbuild the PP database
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/dbmunbuild/RCS/Makefile,v 6.0 1991/12/18 20:30:06 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:30:06  jpo
# Release 6.0
#
# 
#
############################################################
#
# Definitions
#
############################################################



SRCS =  dbmunbuild.c
OBJS =  dbmunbuild.o
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

PROGS = xdbmunbuild

default: $(PROGS)

dbmunbuild: xdbmunbuild
xdbmunbuild: dbmunbuild.o $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ dbmunbuild.o $(LIBPP) $(LIBSYS)

dbmunbuild.o: dbmunbuild.c
	$(OLDCC) -c $(CFLAGS) dbmunbuild.c

install: inst-dir inst-dbmunbuild

inst-dbmunbuild: $(CMDDIR)/dbmunbuild
$(CMDDIR)/dbmunbuild: xdbmunbuild
	-$(BACKUP) $@ zxdbmunbuild
	rm -f $@
	$(INSTALL) xdbmunbuild $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "dbmunbuild installed normally"; echo ""

inst-dir: $(CMDDIR)

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core $(PROGS) zxdbmunbuild *.old *.BAK

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
dbmunbuild.o: dbmunbuild.c
dbmunbuild.o: ../../h/head.h
dbmunbuild.o: ../../h/util.h
dbmunbuild.o: ../../h/config.h
dbmunbuild.o: ../../h/ll_log.h
dbmunbuild.o: ../../h/retcode.h
dbmunbuild.o: ../../h/util.h
dbmunbuild.o: ../../h/chan.h
dbmunbuild.o: ../../h/table.h
dbmunbuild.o: ../../h/list_bpt.h
dbmunbuild.o: ../../h/dbase.h
dbmunbuild.o: ../../h/config.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
