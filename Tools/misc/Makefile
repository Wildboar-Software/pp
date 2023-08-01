#               miscellaneous - used by pp-hackers only
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Tools/misc/RCS/Makefile,v 6.0 1991/12/18 20:30:40 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:30:40  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  t-chkpe.c t-conf.c t-norm.c t-pefrag.c t-prf.c t-rdmsg.c \
	   t-single.c t-time.c t-rtime.c t-txt2dr.c
OBJS =  t-chkpe.o t-conf.o t-norm.o t-pefrag.o t-prf.o t-rdmsg.o \
	   t-single.o t-time.o t-rtime.o t-txt2dr.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a

LDFLAGS		= $(LDOPTIONS) $(LLF)
CFLAGS          =  $(CCOPTIONS) $(LCF) -DDEBUG -I$(HEADERS)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
#               Building Rules
#
############################################################


PROGS = xt-chkpe xt-conf xt-norm xt-pefrag xt-prf xt-rdmsg \
	xt-single xt-time xt-rtime xt-txt2dr

default: $(PROGS)
all:    default

lint:   l-t-chkpe l-t-conf l-t-norm l-t-pefrag l-t-prf l-t-rdmsg \
	l-t-single l-t-time l-t-txt2dr

install: inst-dir inst-t-chkpe inst-t-conf inst-t-norm inst-t-pefrag \
	inst-t-prf inst-t-rdmsg inst-t-single inst-t-time inst-t-txt2dr

inst-dir: $(TOOLDIR)


############################################################
#
#       Start of Program compilations & installations
#
############################################################


t-chkpe:                xt-chkpe
xt-chkpe:               t-chkpe.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-chkpe.o $(LIBPP) $(LIBSYS)
l-t-chkpe:              t-chkpe.c
			$(LINT) $(LINTFLAGS t-chkpe.c $(LINTLIBS)
inst-t-chkpe:           $(TOOLDIR)/t-chkpe
$(TOOLDIR)/t-chkpe:     xt-chkpe
			-$(BACKUP) $@ zxt-chkpe
			rm -f $@
			$(INSTALL) xt-chkpe $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-chkpe tool installed normally"; echo ""



t-conf:                 xt-conf
xt-conf:                t-conf.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-conf.o $(LIBPP) $(LIBSYS)
l-t-conf:               t-conf.c
			$(LINT) $(LINTFLAGS t-conf.c $(LINTLIBS)
inst-t-conf:            $(TOOLDIR)/t-conf
$(TOOLDIR)/t-conf:      xt-conf
			-$(BACKUP) $@ zxt-conf
			rm -f $@
			$(INSTALL) xt-conf $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-conf tool installed normally"; echo ""


t-norm:                 xt-norm
xt-norm:                t-norm.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-norm.o $(LIBPP) $(LIBSYS)
l-t-norm:               t-norm.c
			$(LINT) $(LINTFLAGS) t-norm.c $(LINTLIBS)
inst-t-norm:            $(TOOLDIR)/t-norm
$(TOOLDIR)/t-norm:      xt-norm
			-$(BACKUP) $@ zxt-norm
			rm -f $@
			$(INSTALL) xt-norm $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-norm tool installed normally"; echo ""


t-pefrag:               xt-pefrag
xt-pefrag:              t-pefrag.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-pefrag.o $(LIBPP) $(LIBSYS)
l-t-pefrag:             t-pefrag.c
			$(LINT) $(LINTFLAGS) t-pefrag.c $(LINTLIBS)
inst-t-pefrag:          $(TOOLDIR)/t-pefrag
$(TOOLDIR)/t-pefrag:    xt-pefrag
			-$(BACKUP) $@ zxt-pefrag
			rm -f $@
			$(INSTALL) xt-pefrag $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-pefrag tool installed normally"; echo ""


t-prf:                  xt-prf
xt-prf:                 t-prf.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-prf.o $(LIBPP) $(LIBSYS)
l-t-prf:                t-prf.c
			$(LINT) $(LINTFLAGS t-prf.c $(LINTLIBS)
inst-t-prf:             $(TOOLDIR)/t-prf
$(TOOLDIR)/t-prf:       xt-prf
			-$(BACKUP) $@ zxt-prf
			rm -f $@
			$(INSTALL) xt-prf $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-prf tool installed normally"; echo ""


t-rdmsg:                xt-rdmsg
xt-rdmsg:               t-rdmsg.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-rdmsg.o $(LIBPP) $(LIBSYS)
l-t-rdmsg:              t-rdmsg.c
			$(LINT) $(LINTFLAGS t-rdmsg.c $(LINTLIBS)
inst-t-rdmsg:           $(TOOLDIR)/t-rdmsg
$(TOOLDIR)/t-rdmsg:     xt-rdmsg
			-$(BACKUP) $@ zxt-rdmsg
			rm -f $@
			$(INSTALL) xt-rdmsg $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-rdmsg tool installed normally"; echo ""


t-single:          xt-single
xt-single:         t-single.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-single.o \
			$(LIBPP) $(LIBSYS)
l-t-single:        t-single.c
			$(LINT) $(LINTFLAGS) t-single.c $(LINTISODE)
inst-t-single:     $(TOOLDIR)/t-single
$(TOOLDIR)/t-single: xt-single
			-$(BACKUP) $@ zxt-single
			rm -f $@
			$(INSTALL) xt-single $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-single tool installed normally"; \
			echo ""


t-time:                 xt-time
xt-time:                t-time.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-time.o $(LIBPP) $(LIBSYS)
l-t-time:               t-time.c
			$(LINT) $(LINTFLAGS t-time.c $(LINTLIBS)
inst-t-time:            $(TOOLDIR)/t-time
$(TOOLDIR)/t-time:      xt-time
			-$(BACKUP) $@ zxt-time
			rm -f $@
			$(INSTALL) xt-time $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-time tool installed normally"; echo ""

t-rtime:                 xt-rtime
xt-rtime:                t-rtime.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-rtime.o $(LIBPP) $(LIBSYS)
l-t-rtime:               t-rtime.c
			$(LINT) $(LINTFLAGS t-rtime.c $(LINTLIBS)
inst-t-rtime:            $(TOOLDIR)/t-rtime
$(TOOLDIR)/t-rtime:      xt-rtime
			-$(BACKUP) $@ zxt-rtime
			rm -f $@
			$(INSTALL) xt-rtime $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-rtime tool installed normally"; echo ""


t-txt2dr:               xt-txt2dr
xt-txt2dr:              t-txt2dr.o $(LIBPP)
			$(CC) $(LDFLAGS) -o $@ t-txt2dr.o $(LIBPP) $(LIBSYS)
l-t-txt2dr:             t-txt2dr.c
			$(LINT) $(LINTFLAGS t-txt2dr.c $(LINTLIBS)
inst-t-txt2dr:          $(TOOLDIR)/t-txt2dr
$(TOOLDIR)/t-txt2dr:    xt-txt2dr
			-$(BACKUP) $@ zxt-txt2dr
			rm -f $@
			$(INSTALL) xt-txt2dr $@
			-$(CHMOD) $(PGMPROT) $@
			-$(CHOWN) $(PPUSER) $@
			-@ls -ls $@
			-@echo "t-txt2dr tool installed normally"; echo ""






clean:  tidy
	rm -f $(OBJS)
tidy:
	rm -f core.* $(PROGS) zxt-* *.old *.BAK
depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)
define:
	$(DEFINE) Makefile



############################################################
#
#               End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
t-chkpe.o: t-chkpe.c
t-chkpe.o: ../../h/head.h
t-chkpe.o: ../../h/util.h
t-chkpe.o: ../../h/config.h
t-chkpe.o: ../../h/ll_log.h
t-chkpe.o: ../../h/retcode.h
t-chkpe.o: ../../h/or.h
t-conf.o: t-conf.c
t-norm.o: t-norm.c
t-norm.o: ../../h/util.h
t-norm.o: ../../h/config.h
t-norm.o: ../../h/ll_log.h
t-norm.o: ../../h/chan.h
t-norm.o: ../../h/table.h
t-norm.o: ../../h/list_bpt.h
t-pefrag.o: t-pefrag.c
t-pefrag.o: ../../h/util.h
t-pefrag.o: ../../h/config.h
t-pefrag.o: ../../h/ll_log.h
t-prf.o: t-prf.c
t-prf.o: ../../h/head.h
t-prf.o: ../../h/util.h
t-prf.o: ../../h/config.h
t-prf.o: ../../h/ll_log.h
t-prf.o: ../../h/retcode.h
t-rdmsg.o: t-rdmsg.c
t-rdmsg.o: ../../h/head.h
t-rdmsg.o: ../../h/util.h
t-rdmsg.o: ../../h/config.h
t-rdmsg.o: ../../h/ll_log.h
t-rdmsg.o: ../../h/retcode.h
t-rdmsg.o: ../../h/prm.h
t-rdmsg.o: ../../h/q.h
t-rdmsg.o: ../../h/adr.h
t-rdmsg.o: ../../h/list_rchan.h
t-rdmsg.o: ../../h/chan.h
t-rdmsg.o: ../../h/table.h
t-rdmsg.o: ../../h/list_bpt.h
t-rdmsg.o: ../../h/auth.h
t-rdmsg.o: ../../h/list_bpt.h
t-rdmsg.o: ../../h/extension.h
t-rdmsg.o: ../../h/mta.h
t-rdmsg.o: ../../h/adr.h
t-rdmsg.o: ../../h/list_bpt.h
t-rdmsg.o: ../../h/mta.h
t-single.o: t-single.c
t-single.o: ../../h/util.h
t-single.o: ../../h/config.h
t-single.o: ../../h/ll_log.h
t-single.o: ../../h/ap.h
t-single.o: ../../h/util.h
t-single.o: ../../h/chan.h
t-single.o: ../../h/table.h
t-single.o: ../../h/list_bpt.h
t-single.o: ../../h/retcode.h
t-time.o: t-time.c
t-time.o: ../../h/util.h
t-time.o: ../../h/config.h
t-time.o: ../../h/ll_log.h
t-txt2dr.o: t-txt2dr.c
t-txt2dr.o: ../../h/util.h
t-txt2dr.o: ../../h/config.h
t-txt2dr.o: ../../h/ll_log.h
t-txt2dr.o: ../../h/q.h
t-txt2dr.o: ../../h/adr.h
t-txt2dr.o: ../../h/list_rchan.h
t-txt2dr.o: ../../h/chan.h
t-txt2dr.o: ../../h/table.h
t-txt2dr.o: ../../h/list_bpt.h
t-txt2dr.o: ../../h/auth.h
t-txt2dr.o: ../../h/list_bpt.h
t-txt2dr.o: ../../h/extension.h
t-txt2dr.o: ../../h/mta.h
t-txt2dr.o: ../../h/adr.h
t-txt2dr.o: ../../h/list_bpt.h
t-txt2dr.o: ../../h/mta.h
t-txt2dr.o: ../../h/dr.h
t-txt2dr.o: ../../h/extension.h
t-txt2dr.o: ../../h/mta.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
