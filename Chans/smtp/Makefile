# Smtp channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/smtp/RCS/Makefile,v 6.0 1991/12/18 20:12:19 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:12:19  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  sm_ns.c sm_wtmail.c smtp.c smtpd.c smtpsrvr.c
OBJS =  sm_ns.o sm_wtmail.o smtp.o smtpd.o smtpsrvr.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS = ../../h
LIBPP   = ../../Lib/libpp.a 
INCLUDE = -I$(HEADERS)

CFLAGS  = $(CCOPTIONS) $(LCF) $(INCLUDE)
LDFLAGS = $(LDOPTIONS) $(LLF)
LLFLAGS = $(LINTFLAGS) $(INCLUDE)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS=xsmtp xsmtpsrvr xsmtpd

default: all

all: $(PROGS)

lint: l-smtp l-smtpsrvr l-smtpd

############################################################
#
# Smtp outbound channel
#
############################################################

SMTPSRCS = smtp.c sm_wtmail.c sm_ns.c
SMTPOBJS = smtp.o sm_wtmail.o sm_ns.o


smtp: xsmtp

xsmtp: $(SMTPOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(SMTPOBJS) $(LIBPP) $(LIBRESOLV) $(LIBSYS)

saber_smtp_src: $(SMTPSRCS)
	#load -C $(CFLAGS) $(SMTPSRCS) $(LIBPP) $(LIBRESOLV) $(LIBSYS)

saber_smtp_obj:$(SMTPOBJS)
	#load -C smtp.o $(SMTPOBJS) $(LIBPP) $(LIBRESOLV) $(LIBSYS)

l-smtp: $(SMTPSRCS)
	$(LINT) $(LLFLAGS) $(SMTPSRCS) $(LINTLIBS)

############################################################
#
# SMTP stub dameon
#
############################################################

SMTPDSRCS	= smtpd.c
SMTPDOBJS	= smtpd.o

smtpd: xsmtpd

xsmtpd: $(SMTPDOBJS) $(LIBPP)
	$(CC) $(LDFLAGS) $(SMTPDOBJS) $(LIBPP) \
		$(LIBSYS) -o xsmtpd

saber_smtpd_src: $(SMTPDSRCS)
	#load -C $(CFLAGS) $(SMTPDSRCS) $(LIBPP) $(LIBSYS)

saber_smtpd_obj:$(SMTPDOBJS)
	#load -C $(SMTPDOBJS) $(LIBPP) $(LIBSYS)

l-smtpd: $(SMTPDSRCS)
	$(LINT) $(LLFLAGS) $(SMTPDSRCS) $(LINTLIBS)

############################################################
#
# smtp server process
#
############################################################

SMTPSRVRSRCS	= smtpsrvr.c sm_ns.c
SMTPSRVROBJS	= smtpsrvr.o sm_ns.o
smtpsrvr: xsmtpsrvr

xsmtpsrvr: $(SMTPSRVROBJS) $(LIBPP)
	$(CC) $(LDFLAGS) -o $@ $(SMTPSRVROBJS) \
		$(LIBPP) $(LIBSYS) $(LIBRESOLV)

saber_smtpsrvr_src: $(SMTPSRVRSRCS)
	#load -C $(CFLAGS) $(SMTPSRVRSRCS) $(LIBPP) $(LIBSYS) $(LIBRESOLV)

saber_smtpsrvr_obj:$(SMTPSRVROBJS)
	#load -C $(SMTPSRVROBJS) $(LIBPP) $(LIBSYS) $(LIBRESOLV)

l-smtpsrvr:  $(SMTPSRVRSRCS)
	$(LINT) $(LLFLAGS)  $(SMTPSRVRSRCS) $(LINTLIBS)

############################################################
#
# misc things
#
############################################################

install: inst-dir inst-smtp inst-smtpsrvr inst-smtpd

inst-dir: $(CHANDIR)

inst-smtp: $(CHANDIR)/smtp

$(CHANDIR)/smtp: xsmtp
	-$(BACKUP) $@ zxsmtp
	rm -f $@
	$(INSTALL) xsmtp $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "smtp channel installed normally"; echo ""

inst-smtpsrvr: $(CHANDIR)/smtpsrvr

$(CHANDIR)/smtpsrvr: xsmtpsrvr
	-$(BACKUP) $@ zxsmtpsrvr
	rm -f $@
	$(INSTALL) xsmtpsrvr $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "smtp server installed normally"; echo " "

inst-smtpd: $(CHANDIR)/smtpd

$(CHANDIR)/smtpd:  xsmtpd
	-$(BACKUP) $@ zxsmtpd
	rm -f $@
	$(INSTALL) xsmtpd $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "smtp daemon installed normally"; echo " "


clean: tidy
	rm -f $(OBJS) $(PROGS) core a.out *old *.BAK zxsmtp*

tidy:
	rm -f $(PROGS) zxsmtp* core a.out *old *.BAK

depend:
	$(DEPEND) $(INCLUDE) $(SRCS)

define:
	$(DEFINE) Makefile



 ############################################################
 #
 # End of Building Rules
 #
 ############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
sm_ns.o: sm_ns.c
sm_ns.o: ../../h/util.h
sm_ns.o: ../../h/config.h
sm_ns.o: ../../h/ll_log.h
sm_ns.o: ../../h/retcode.h
sm_ns.o: ../../h/chan.h
sm_ns.o: ../../h/table.h
sm_ns.o: ../../h/list_bpt.h
sm_wtmail.o: sm_wtmail.c
sm_wtmail.o: ../../h/util.h
sm_wtmail.o: ../../h/config.h
sm_wtmail.o: ../../h/ll_log.h
sm_wtmail.o: ../../h/retcode.h
sm_wtmail.o: ../../h/chan.h
sm_wtmail.o: ../../h/table.h
sm_wtmail.o: ../../h/list_bpt.h
sm_wtmail.o: ../../h/ap.h
sm_wtmail.o: ../../h/util.h
smtp.o: smtp.c
smtp.o: ../../h/head.h
smtp.o: ../../h/util.h
smtp.o: ../../h/config.h
smtp.o: ../../h/ll_log.h
smtp.o: ../../h/retcode.h
smtp.o: ../../h/chan.h
smtp.o: ../../h/table.h
smtp.o: ../../h/list_bpt.h
smtp.o: ../../h/prm.h
smtp.o: ../../h/q.h
smtp.o: ../../h/adr.h
smtp.o: ../../h/list_rchan.h
smtp.o: ../../h/chan.h
smtp.o: ../../h/auth.h
smtp.o: ../../h/list_bpt.h
smtp.o: ../../h/extension.h
smtp.o: ../../h/mta.h
smtp.o: ../../h/adr.h
smtp.o: ../../h/list_bpt.h
smtp.o: ../../h/aparse.h
smtp.o: ../../h/ap.h
smtp.o: ../../h/util.h
smtp.o: ../../h/or.h
smtp.o: ../../h/chan.h
smtp.o: ../../h/auth.h
smtp.o: ../../h/list_rchan.h
smtp.o: ../../h/mta.h
smtp.o: ../../h/dr.h
smtp.o: ../../h/extension.h
smtp.o: ../../h/mta.h
smtp.o: ../../h/qmgr.h
smtp.o: ../../h/Qmgr-types.h
smtp.o: ../../h/Qmgr-ops.h
smtp.o: ../../h/sys.file.h
smtpd.o: smtpd.c
smtpd.o: ../../h/util.h
smtpd.o: ../../h/config.h
smtpd.o: ../../h/ll_log.h
smtpd.o: ../../h/sys.file.h
smtpsrvr.o: smtpsrvr.c
smtpsrvr.o: ../../h/head.h
smtpsrvr.o: ../../h/util.h
smtpsrvr.o: ../../h/config.h
smtpsrvr.o: ../../h/ll_log.h
smtpsrvr.o: ../../h/retcode.h
smtpsrvr.o: ../../h/prm.h
smtpsrvr.o: ../../h/q.h
smtpsrvr.o: ../../h/adr.h
smtpsrvr.o: ../../h/list_rchan.h
smtpsrvr.o: ../../h/chan.h
smtpsrvr.o: ../../h/table.h
smtpsrvr.o: ../../h/list_bpt.h
smtpsrvr.o: ../../h/auth.h
smtpsrvr.o: ../../h/list_bpt.h
smtpsrvr.o: ../../h/extension.h
smtpsrvr.o: ../../h/mta.h
smtpsrvr.o: ../../h/adr.h
smtpsrvr.o: ../../h/list_bpt.h
smtpsrvr.o: ../../h/aparse.h
smtpsrvr.o: ../../h/ap.h
smtpsrvr.o: ../../h/util.h
smtpsrvr.o: ../../h/or.h
smtpsrvr.o: ../../h/chan.h
smtpsrvr.o: ../../h/auth.h
smtpsrvr.o: ../../h/list_rchan.h
smtpsrvr.o: ../../h/mta.h
smtpsrvr.o: ../../h/dl.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
