# GreyBook channel
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/grey/RCS/Makefile,v 6.0 1991/12/18 20:10:19 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:10:19  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS	= io_grey.c ppp.c tppp.c
OBJS	= io_grey.o ppp.o tppp.o
UNQO	= $(NIFTPSRC)/lib/pqproc/ppQunspool.o
UNPO	= $(NIFTPSRC)/lib/pqproc/ppPunspool.o
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
LIBPP           = ../../Lib/libpp.a

CFLAGS          = $(CCOPTIONS) $(LCF) -I$(HEADERS)
LDFLAGS         = $(LDOPTIONS) $(LLF)

LLFLAGS         = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS        = ../../Lib/llib-lpp.ln $(LINTISODE)


############################################################
#
# Building Rules
#
############################################################

PROGS	= greyout greyin 
CHECKS	= checkQbin
default: $(CHECKS) $(PROGS)

install: inst-dir inst-greyout inst-greyin
inst-dir: $(CHANDIR)

# If unix-niftp has been re-installed, make io_grey.o newer to force copy
checkQbin: io_grey.o ALWAYS
	@sh -c "cmp -s io_grey.o $(UNQO) || \
		{ echo touch io_grey.o; touch io_grey.o; } || \
		kill $$$$"

# If the unix-niftp copy is out of date, Zap it.
$(UNQO): io_grey.o
	cp io_grey.o $@

greyin: xgreyin
xgreyin: $(NIFTPSRC)/bin/q$(NIFTPINTERFACE) $(LIBPP)
	cp $(NIFTPSRC)/bin/q$(NIFTPINTERFACE) xgreyin
$(NIFTPSRC)/bin/q$(NIFTPINTERFACE): $(UNQO)
	rm -f $(NIFTPSRC)/bin/q$(NIFTPINTERFACE)
	cd $(NIFTPSRC); make lib/pqproc q$(NIFTPINTERFACE) \
		NIFTPINTERFACE=$(NIFTPINTERFACE)

saber_greyin_src:io_grey.c
	#load -C $(CFLAGS) io_grey.c $(LIBPP) $(LIBSYS)
	#setopt path $(NIFTPSRC) $(NIFTPSRC)/lib/pqproc
	#load -C -I$(NIFTPSRC)/h \
		$(NIFTPSRC)/lib/gen/*.c \
		$(NIFTPSRC)/lib/mmdf/*.c \
		$(NIFTPSRC)/lib/sun/netlisten.c \
		$(NIFTPSRC)/lib/sun/netsubs.c
	#load -C  -I$(NIFTPSRC)/h \
		tcccomm.c rectrans.c timer.c dec.c data.c record.c \
		adr_trans.c \
		attr_log.c stat.c ts_buff_dec.c find_user.c \
		 perms.c qdocket.c rsft.c nrs_rev.c \
		ppQdummy.c
	#load -C -I$(NIFTPSRC)/h $(NIFTPSRC)/bin/ftq.c

inst-greyin: $(CHANDIR)/greyin
$(CHANDIR)/greyin: xgreyin
	-$(BACKUP) $@ zxgreyin
	rm -f $@
	$(INSTALL) xgreyin $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "greyin channel installed normally"; echo ""


greyout: xgreyout
xgreyout: $(UNPO) $(LIBPP)
	rm -f $(NIFTPSRC)/bin/$@
	cd $(NIFTPSRC); make lib/pqproc/libppp.a $@ NIFTPINTERFACE=$(NIFTPINTERFACE)
	cp $(NIFTPSRC)/bin/$@ $@

saber_greyout_src:ppp.c
	#load -C $(CFLAGS) ppp.c $(LIBPP) $(LIBSYS)
	#setopt path $(NIFTPSRC) $(NIFTPSRC)/lib/pqproc
	#load -C -I$(NIFTPSRC)/h \
		$(NIFTPSRC)/lib/gen/*.c \
		$(NIFTPSRC)/lib/mmdf/*.c \
		$(NIFTPSRC)/lib/sun/netopen.c \
		$(NIFTPSRC)/lib/sun/netsubs.c
	#load -C  -I$(NIFTPSRC)/h \
		ppPunspool.c \
		pdocket.c sft.c tcccomm.c rectrans.c timer.c \
		dec.c data.c record.c adr_trans.c \
		attr_log.c stat.c ts_buff_dec.c find_user.c
	#load -C -I$(NIFTPSRC)/h -DFULLPP $(NIFTPSRC)/bin/ftp.c 

$(UNPO): ppp.o
	cp ppp.o $@

inst-greyout: $(CHANDIR)/greyout
$(CHANDIR)/greyout: xgreyout
	-$(BACKUP) $@ zxgreyout
	rm -f $@
	$(INSTALL) xgreyout $@
	-$(CHMOD) $(PGMPROT) $@
	-$(CHOWN) $(PPUSER) $@
	-@ls -ls $@
	-@echo "greyout channel installed normally"; echo ""


tgreyin: xtgreyin

xtgreyin: tgreyin.o io_grey.o
	$(CC) $(LDFLAGS) -o $@ tgreyin.o io_grey.o $(LIBPP) $(LIBSYS)

saber_greyin: tgreyin.c io_grey.c
	#load -C $(CFLAGS) tgreyin.c io_grey.c $(LIBPP) $(LIBSYS)

clean: tidy
	rm -f $(OBJS) *.o
tidy:
	rm -f core* a.out *.old *.BAK xgreyout zx* xgreyin xtgreyin tgreyin

lint: l-greyout l-greyin

l-greyout: ppp.c
	$(LINT) $(LLFLAGS) ppp.c $(LINTLIBS)

l-greyin: io_grey.c
	$(LINT) $(LLFLAGS) io_grey.c $(LINTLIBS)


depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

define:
	$(DEFINE) Makefile

ALWAYS:

############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
io_grey.o: io_grey.c
io_grey.o: ../../h/util.h
io_grey.o: ../../h/config.h
io_grey.o: ../../h/ll_log.h
io_grey.o: ../../h/prm.h
io_grey.o: ../../h/q.h
io_grey.o: ../../h/adr.h
io_grey.o: ../../h/list_rchan.h
io_grey.o: ../../h/chan.h
io_grey.o: ../../h/table.h
io_grey.o: ../../h/list_bpt.h
io_grey.o: ../../h/auth.h
io_grey.o: ../../h/list_bpt.h
io_grey.o: ../../h/extension.h
io_grey.o: ../../h/mta.h
io_grey.o: ../../h/adr.h
io_grey.o: ../../h/list_bpt.h
io_grey.o: ../../h/aparse.h
io_grey.o: ../../h/ap.h
io_grey.o: ../../h/util.h
io_grey.o: ../../h/or.h
io_grey.o: ../../h/chan.h
io_grey.o: ../../h/auth.h
io_grey.o: ../../h/list_rchan.h
io_grey.o: ../../h/mta.h
io_grey.o: ../../h/adr.h
io_grey.o: ../../h/ap.h
io_grey.o: ../../h/retcode.h
io_grey.o: ../../h/tb_q.h
io_grey.o: ../../h/tb_a.h
io_grey.o: ../../h/tb_prm.h
ppp.o: ppp.c
ppp.o: ../../h/util.h
ppp.o: ../../h/config.h
ppp.o: ../../h/ll_log.h
ppp.o: ../../h/ppp.h
ppp.o: ../../h/qmgr.h
ppp.o: ../../h/Qmgr-types.h
ppp.o: ../../h/Qmgr-ops.h
ppp.o: ../../h/prm.h
ppp.o: ../../h/q.h
ppp.o: ../../h/adr.h
ppp.o: ../../h/list_rchan.h
ppp.o: ../../h/chan.h
ppp.o: ../../h/table.h
ppp.o: ../../h/list_bpt.h
ppp.o: ../../h/auth.h
ppp.o: ../../h/list_bpt.h
ppp.o: ../../h/extension.h
ppp.o: ../../h/mta.h
ppp.o: ../../h/adr.h
ppp.o: ../../h/list_bpt.h
ppp.o: ../../h/aparse.h
ppp.o: ../../h/ap.h
ppp.o: ../../h/util.h
ppp.o: ../../h/or.h
ppp.o: ../../h/chan.h
ppp.o: ../../h/auth.h
ppp.o: ../../h/list_rchan.h
ppp.o: ../../h/mta.h
ppp.o: ../../h/adr.h
ppp.o: ../../h/chan.h
ppp.o: ../../h/retcode.h
ppp.o: ../../h/dr.h
ppp.o: ../../h/extension.h
ppp.o: ../../h/mta.h
ppp.o: ../../h/ap.h
tppp.o: tppp.c
tppp.o: ../../h/util.h
tppp.o: ../../h/config.h
tppp.o: ../../h/ll_log.h
tppp.o: ../../h/ppp.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
