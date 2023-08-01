# ckconfig: management program to check the configuration of the PP system.
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/ckconfig/RCS/Makefile,v 6.0 1991/12/18 20:29:03 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:29:03  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  ckconfig.c 
OBJS =  ckconfig.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS =       ../../h
LIBPP   = ../../Lib/libpp.a

CFLAGS  = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

PROGS = xckconfig

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# CheckConfig
#
############################################################
ckconfig: xckconfig
xckconfig: $(OBJS) $(LIBPP)
	    rm -f $@
	    $(CC) $(LDFLAGS) -o $@ $(OBJS) \
		  $(LIBPP) $(LIBSYS) 

############################################################
#
# Misc stuff
#
############################################################


install: inst-dir inst-ckconfig
inst-ckconfig: $(CMDDIR)/ckconfig
$(CMDDIR)/ckconfig: xckconfig
	-$(BACKUP) $@ zxckconfig
	rm -f $@
	$(INSTALL) xckconfig $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "ckconfig tool installed normally"; echo ""

inst-dir: $(TOOLDIR)


clean: tidy
	rm -f $(PROGS)
tidy:
	rm -f $(OBJS) core a.out *.old *.BAK zxckconfig

lint: l-ckconfig

l-ckconfig: $(SRCS) true
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS)

true:;

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
ckconfig.o: ckconfig.c
ckconfig.o: ../../h/util.h
ckconfig.o: ../../h/config.h
ckconfig.o: ../../h/ll_log.h
ckconfig.o: ../../h/chan.h
ckconfig.o: ../../h/table.h
ckconfig.o: ../../h/list_bpt.h
ckconfig.o: ../../h/table.h
ckconfig.o: ../../h/list_bpt.h
ckconfig.o: ckconfig.c
ckconfig.o: ../../h/util.h
ckconfig.o: ../../h/config.h
ckconfig.o: ../../h/ll_log.h
ckconfig.o: ../../h/chan.h
ckconfig.o: ../../h/table.h
ckconfig.o: ../../h/list_bpt.h
ckconfig.o: ../../h/table.h
ckconfig.o: ../../h/list_bpt.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
