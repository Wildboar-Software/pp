# Dummmm v6-like mail interface
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Uip/mail/RCS/Makefile,v 6.0 1991/12/18 20:35:57 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:35:57  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  mail.c
OBJS =  mail.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP   = ../../Lib/libpp.a
CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) $(LLF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

all: xmail

mail: xmail
	@true
xmail: $(OBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBPP) $(LIBSYS)

clean: tidy
	rm -f $(OBJS)

tidy:
	rm -f zxmail xmail core* a.out *.old *.BAK


install: inst-dir inst-mail

inst-dir: $(CMDDIR)

inst-mail: $(CMDDIR)/mail
$(CMDDIR)/mail: xmail
	-$(BACKUP) $@ zxmail
	rm -f $@
	$(INSTALL) xmail $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "Mail installed normally"; echo ""

lint: $(SRCS)
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
mail.o: mail.c
mail.o: ../../h/util.h
mail.o: ../../h/config.h
mail.o: ../../h/ll_log.h
mail.o: ../../h/retcode.h
mail.o: mail.c
mail.o: ../../h/util.h
mail.o: ../../h/config.h
mail.o: ../../h/ll_log.h
mail.o: ../../h/retcode.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
