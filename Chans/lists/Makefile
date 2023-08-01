# dirlist Makefile
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Chans/lists/RCS/Makefile,v 6.0 1991/12/18 20:10:43 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:10:43  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################


SRCS =  DL_tables.c MD_tables.c \
           list.c dish.c dl.c dl_permit.c dl_util.c dl_x500.c \
           init.c lmnpq_dish.c or2addr.c \
           syn_oraddr.c syn_orname.c syn_permit.c syn_policy.c 
OBJS =  DL_tables.o MD_tables.o \
           list.o dirlist.o dish.o dl.o dl_permit.o dl_util.o dl_x500.o \
           init.o lmnpq_dish.o or2addr.o \
           syn_oraddr.o syn_orname.o syn_permit.o syn_policy.o 
# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

TBSRCS = list.c
TBOBJS = list.o

OLIB    = MD_tables.o DL_tables.o \
	  syn_oraddr.o syn_orname.o syn_policy.o syn_permit.o \
	  or2addr.o dl_util.o dl_x500.o dl_permit.o init.o hack.o \
	  lmnpq_dish.o

HEADERS 	= ../../h
LIBPP   	= ../../Lib/libpp.a
INCLUDE 	= -I$(HEADERS) -I$(PEPSYHDRS)

#ILIBS    	= -ldsap -lisode
LIBS    	= $(LIBPP) $(LIBSYS) 
LIBDISH 	= -ldish

# GNU Readline (if needed by dish)
LIBRL		=
#LIBRL		= -lreadline -ltermcap
#LIBRL		=  -L /xtel/users/cjr/src/readline -lreadline -ltermcap

CFLAGS  	= $(CCOPTIONS) $(LCF) $(INCLUDE) 
LDFLAGS 	= $(LDOPTIONS) $(LLF)

LINTDSAP	= /usr/local/lib/lint/llib-ldsap.ln
LINTISODE	= /usr/local/lib/lint/llib-lisode.ln
LLFLAGS 	= $(LINTFLAGS) $(INCLUDE)
LINTLIBS 	= $(LINTISODE) $(LINTDSAP) ../../Lib/llib-lpp.ln 

############################################################
#
# Building Rules
#
############################################################

PROGS 		= xdl xdirlist xlist

default: 	$(PROGS)

dirlist: 	xdirlist
lsd:		xlsd
dl:		xdl

install: 	inst-channel inst-uip
inst-channel:	inst-dir inst-dirlist inst-list
inst-uip:	inst-dl

# List channel using directory...
xdirlist:  	libdl.a dirlist.o $(LIBPP) 
		$(CC) $(LDFLAGS) -o $@ dirlist.o libdl.a $(LIBS)

# dl - User interface
xdl:		libdl.a  dl.o hack.o $(LIBPP) 
		$(CC) $(LDOPTIONS) -o $@ dl.o libdl.a $(LIBS) 


# if dish uses GNU Readline uncomment the following macro
LGNUREADLINE=   # -lreadline -ltermcap

# if you have ISODE > 6.8 then it is easiest to make xlsd in
# the ISODE tree as dish.  See ISODE/config/OPTIONS for details.
# You then get MHS attributes in ALL DUAs.
xlsd:		libdl.a dish.o $(QLIB) $(LIBPP)
		-$(CC) $(OPTIONS) -o $@ dish.o \
			libdl.a $(LIBDISH) $(LIBS) $(LIBRL) $(LGNUREADLINE)

dish.o:		dish.c 
		$(CC) $(CFLAGS) -DUSE_PP -c $*.c

dirlist.c:	list.c
		rm -f dirlist.c
		ln list.c dirlist.c

dirlist.o:	dirlist.c
		$(CC) $(CFLAGS) -DDIRLIST -c dirlist.c

libdl:
		rm -f libdl.a
		$(AR) $(ARFLAGS)r $@.a $(OLIB)
		$(RANLIB) $@.a

libdl.a: 	$(OLIB)
		$(AR) $(ARFLAGS)r $@ $?
		$(RANLIB) $@
	
lint:		l-dirlist l-list

l-dirlist:	$(SRCS)
		$(LINT) $(LLFLAGS) -DDIRLIST $(SRCS) $(LINTLIBS)


inst-dirlist: 	$(CHANDIR)/dirlist

$(CHANDIR)/dirlist: xdirlist
		-$(BACKUP) $@ zxdirlist
		rm -f $@
		$(INSTALL) xdirlist $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@
		-@echo "dirlist channel installed normally"; echo ""

inst-dl: 	$(USRBINDIR)/dl

$(USRBINDIR)/dl: xdl
		-$(BACKUP) $@ zxdl
		rm -f $@
		$(INSTALL) xdl $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@

# could install this as dish.
# and link 'lmnpq' to 'list'
inst-lsd: 	$(USRBINDIR)/lsd

$(USRBINDIR)/lsd: xlsd
		@echo You might want to install this a dish \!
		-$(BACKUP) $@ zxlsd
		rm -f $@
		$(INSTALL) xlsd $@
		-$(CHMOD) $(PGMPROT) $@
		-$(CHOWN) $(PPUSER) $@
		-@ls -ls $@

inst-dir: 	$(CHANDIR)

clean: tidy
		rm -f $(OBJS) $(OLIB) DL* MD* libdl.a dirlist.c \
		$(HEADERS)/MD-types.h $(HEADERS)/MD_defs.h \
		$(HEADERS)/MD_pre_defs.h \
		$(HEADERS)/DL-types.h $(HEADERS)/DL_defs.h \
		$(HEADERS)/DL_pre_defs.h

tidy:
		rm -f core a.out *.cld $(PROGS) *.BAK zx* *.ph \
		      xufn xtest xlsd

depend:
		$(DEPEND) $(INCLUDE) $(SRCS)

define:
		$(DEFINE) Makefile


.py.c:;		$(PEPY) $*.py

.py.o:;		$(PEPY) $*.py
		$(CC) $(CFLAGS) -c $*.c

IOB.ph:		../../Lib/x400/IOB.ph
		cp ../../Lib/x400/IOB.ph .

MD-types.h MD_tables.c: md.py IOB.ph dlist.h
		$(PEPSY) -i dlist.h -A md.py
		cp MD-types.h $(HEADERS)
		-if [ -f MD_defs.h ]; then \
		cp MD_defs.h MD_pre_defs.h $(HEADERS); \
		fi

DL-types.h DL_tables.c: dl.py dlist.h
		$(PEPSY) -i dlist.h -A dl.py
		cp DL-types.h $(HEADERS)
		-if [ -f DL_defs.h ]; then\
		cp DL_defs.h DL_pre_defs.h $(HEADERS); \
		fi

MD_tables.c: DL_tables.c

#dish.c:		$(ISODESRC)/quipu/dish/dish.c
#		@echo WARNING $? has changed \!

############################################################
#
# table based list channel
#
############################################################

list: xlist

xlist:  $(TBOBJS) $(LIBPP) 
	$(CC) $(LDFLAGS) -o $@ $(TBOBJS) $(LIBPP) $(LIBSYS)

l-list: $(TBSRCS)
	$(LINT) $(LLFLAGS) $(TBSRCS) $(LINTLIBS)

inst-list: $(CHANDIR)/list
$(CHANDIR)/list: xlist
	   -$(BACKUP) $@ zxlist
	   rm -f $@
	   $(INSTALL) xlist $@
	   -$(CHMOD) $(PGMPROT) $@
	   -$(CHOWN) $(PPUSER) $@
	   -@ls -ls $@
	   -@echo "list channel installed normally"; echo ""

inst-dir: $(CHANDIR)


############################################################
#
# End of Building Rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
DL_tables.o: DL_tables.c
DL_tables.o: DL-types.h
DL_tables.o: dlist.h
DL_tables.o: ../../h/util.h
DL_tables.o: ../../h/config.h
DL_tables.o: ../../h/ll_log.h
DL_tables.o: ../../h/or.h
DL_tables.o: DL-types.h
DL_tables.o: MD-types.h
DL_tables.o: dlist.h
DL_tables.o: ../../h/IOB-types.h
DL_tables.o: ../../h/MTA-types.h
DL_tables.o: ../../h/Toks-types.h
DL_tables.o: ../../h/Auth-types.h
MD_tables.o: MD_tables.c
MD_tables.o: MD-types.h
MD_tables.o: dlist.h
MD_tables.o: ../../h/util.h
MD_tables.o: ../../h/config.h
MD_tables.o: ../../h/ll_log.h
MD_tables.o: ../../h/or.h
MD_tables.o: DL-types.h
MD_tables.o: dlist.h
MD_tables.o: MD-types.h
MD_tables.o: ../../h/IOB-types.h
MD_tables.o: ../../h/MTA-types.h
MD_tables.o: ../../h/Toks-types.h
MD_tables.o: ../../h/Auth-types.h
list.o: list.c
list.o: ../../h/util.h
list.o: ../../h/config.h
list.o: ../../h/ll_log.h
list.o: ../../h/head.h
list.o: ../../h/util.h
list.o: ../../h/retcode.h
list.o: ../../h/qmgr.h
list.o: ../../h/Qmgr-types.h
list.o: ../../h/Qmgr-ops.h
list.o: ../../h/chan.h
list.o: ../../h/table.h
list.o: ../../h/list_bpt.h
list.o: ../../h/q.h
list.o: ../../h/adr.h
list.o: ../../h/list_rchan.h
list.o: ../../h/chan.h
list.o: ../../h/auth.h
list.o: ../../h/list_bpt.h
list.o: ../../h/extension.h
list.o: ../../h/mta.h
list.o: ../../h/adr.h
list.o: ../../h/list_bpt.h
list.o: ../../h/aparse.h
list.o: ../../h/ap.h
list.o: ../../h/util.h
list.o: ../../h/or.h
list.o: ../../h/chan.h
list.o: ../../h/auth.h
list.o: ../../h/list_rchan.h
list.o: ../../h/mta.h
list.o: ../../h/dr.h
list.o: ../../h/extension.h
list.o: ../../h/mta.h
list.o: ../../h/or.h
list.o: ../../h/prm.h
list.o: ../../h/dl.h
list.o: ../../h/retcode.h
list.o: ../../h/sys.file.h
dish.o: dish.c
dl.o: dl.c
dl.o: dlist.h
dl.o: ../../h/util.h
dl.o: ../../h/config.h
dl.o: ../../h/ll_log.h
dl.o: ../../h/or.h
dl.o: DL-types.h
dl.o: dlist.h
dl.o: MD-types.h
dl.o: dlist.h
dl.o: ../../h/IOB-types.h
dl.o: ../../h/MTA-types.h
dl.o: ../../h/Toks-types.h
dl.o: ../../h/Auth-types.h
dl.o: ../../h/retcode.h
dl.o: ../../h/dl.h
dl_permit.o: dl_permit.c
dl_permit.o: dlist.h
dl_permit.o: ../../h/util.h
dl_permit.o: ../../h/config.h
dl_permit.o: ../../h/ll_log.h
dl_permit.o: ../../h/or.h
dl_permit.o: DL-types.h
dl_permit.o: dlist.h
dl_permit.o: MD-types.h
dl_permit.o: dlist.h
dl_permit.o: ../../h/IOB-types.h
dl_permit.o: ../../h/MTA-types.h
dl_permit.o: ../../h/Toks-types.h
dl_permit.o: ../../h/Auth-types.h
dl_permit.o: ../../h/adr.h
dl_permit.o: ../../h/list_rchan.h
dl_permit.o: ../../h/chan.h
dl_permit.o: ../../h/table.h
dl_permit.o: ../../h/list_bpt.h
dl_permit.o: ../../h/auth.h
dl_permit.o: ../../h/list_bpt.h
dl_permit.o: ../../h/extension.h
dl_permit.o: ../../h/mta.h
dl_permit.o: ../../h/adr.h
dl_permit.o: ../../h/list_bpt.h
dl_permit.o: ../../h/aparse.h
dl_permit.o: ../../h/ap.h
dl_permit.o: ../../h/util.h
dl_permit.o: ../../h/or.h
dl_permit.o: ../../h/chan.h
dl_permit.o: ../../h/auth.h
dl_permit.o: ../../h/list_rchan.h
dl_permit.o: ../../h/or.h
dl_util.o: dl_util.c
dl_util.o: dlist.h
dl_util.o: ../../h/util.h
dl_util.o: ../../h/config.h
dl_util.o: ../../h/ll_log.h
dl_util.o: ../../h/or.h
dl_util.o: DL-types.h
dl_util.o: dlist.h
dl_util.o: MD-types.h
dl_util.o: dlist.h
dl_util.o: ../../h/IOB-types.h
dl_util.o: ../../h/MTA-types.h
dl_util.o: ../../h/Toks-types.h
dl_util.o: ../../h/Auth-types.h
dl_util.o: ../../h/retcode.h
dl_util.o: ../../h/adr.h
dl_util.o: ../../h/list_rchan.h
dl_util.o: ../../h/chan.h
dl_util.o: ../../h/table.h
dl_util.o: ../../h/list_bpt.h
dl_util.o: ../../h/auth.h
dl_util.o: ../../h/list_bpt.h
dl_util.o: ../../h/extension.h
dl_util.o: ../../h/mta.h
dl_util.o: ../../h/adr.h
dl_util.o: ../../h/list_bpt.h
dl_util.o: ../../h/aparse.h
dl_util.o: ../../h/ap.h
dl_util.o: ../../h/util.h
dl_util.o: ../../h/or.h
dl_util.o: ../../h/chan.h
dl_util.o: ../../h/auth.h
dl_util.o: ../../h/list_rchan.h
dl_util.o: ../../h/or.h
dl_x500.o: dl_x500.c
dl_x500.o: dlist.h
dl_x500.o: ../../h/util.h
dl_x500.o: ../../h/config.h
dl_x500.o: ../../h/ll_log.h
dl_x500.o: ../../h/or.h
dl_x500.o: DL-types.h
dl_x500.o: dlist.h
dl_x500.o: MD-types.h
dl_x500.o: dlist.h
dl_x500.o: ../../h/IOB-types.h
dl_x500.o: ../../h/MTA-types.h
dl_x500.o: ../../h/Toks-types.h
dl_x500.o: ../../h/Auth-types.h
init.o: init.c
init.o: dlist.h
init.o: ../../h/util.h
init.o: ../../h/config.h
init.o: ../../h/ll_log.h
init.o: ../../h/or.h
init.o: DL-types.h
init.o: dlist.h
init.o: MD-types.h
init.o: dlist.h
init.o: ../../h/IOB-types.h
init.o: ../../h/MTA-types.h
init.o: ../../h/Toks-types.h
init.o: ../../h/Auth-types.h
lmnpq_dish.o: lmnpq_dish.c
lmnpq_dish.o: dlist.h
lmnpq_dish.o: ../../h/util.h
lmnpq_dish.o: ../../h/config.h
lmnpq_dish.o: ../../h/ll_log.h
lmnpq_dish.o: ../../h/or.h
lmnpq_dish.o: DL-types.h
lmnpq_dish.o: dlist.h
lmnpq_dish.o: MD-types.h
lmnpq_dish.o: dlist.h
lmnpq_dish.o: ../../h/IOB-types.h
lmnpq_dish.o: ../../h/MTA-types.h
lmnpq_dish.o: ../../h/Toks-types.h
lmnpq_dish.o: ../../h/Auth-types.h
or2addr.o: or2addr.c
or2addr.o: dlist.h
or2addr.o: ../../h/util.h
or2addr.o: ../../h/config.h
or2addr.o: ../../h/ll_log.h
or2addr.o: ../../h/or.h
or2addr.o: DL-types.h
or2addr.o: dlist.h
or2addr.o: MD-types.h
or2addr.o: dlist.h
or2addr.o: ../../h/IOB-types.h
or2addr.o: ../../h/MTA-types.h
or2addr.o: ../../h/Toks-types.h
or2addr.o: ../../h/Auth-types.h
or2addr.o: ../../h/retcode.h
or2addr.o: ../../h/or.h
or2addr.o: ../../h/adr.h
or2addr.o: ../../h/list_rchan.h
or2addr.o: ../../h/chan.h
or2addr.o: ../../h/table.h
or2addr.o: ../../h/list_bpt.h
or2addr.o: ../../h/auth.h
or2addr.o: ../../h/list_bpt.h
or2addr.o: ../../h/extension.h
or2addr.o: ../../h/mta.h
or2addr.o: ../../h/adr.h
or2addr.o: ../../h/list_bpt.h
or2addr.o: ../../h/aparse.h
or2addr.o: ../../h/ap.h
or2addr.o: ../../h/util.h
or2addr.o: ../../h/or.h
or2addr.o: ../../h/chan.h
or2addr.o: ../../h/auth.h
or2addr.o: ../../h/list_rchan.h
syn_oraddr.o: syn_oraddr.c
syn_oraddr.o: ../../h/util.h
syn_oraddr.o: ../../h/config.h
syn_oraddr.o: ../../h/ll_log.h
syn_oraddr.o: ../../h/retcode.h
syn_oraddr.o: ../../h/adr.h
syn_oraddr.o: ../../h/list_rchan.h
syn_oraddr.o: ../../h/chan.h
syn_oraddr.o: ../../h/table.h
syn_oraddr.o: ../../h/list_bpt.h
syn_oraddr.o: ../../h/auth.h
syn_oraddr.o: ../../h/list_bpt.h
syn_oraddr.o: ../../h/extension.h
syn_oraddr.o: ../../h/mta.h
syn_oraddr.o: ../../h/adr.h
syn_oraddr.o: ../../h/list_bpt.h
syn_oraddr.o: ../../h/aparse.h
syn_oraddr.o: ../../h/ap.h
syn_oraddr.o: ../../h/util.h
syn_oraddr.o: ../../h/or.h
syn_oraddr.o: ../../h/chan.h
syn_oraddr.o: ../../h/auth.h
syn_oraddr.o: ../../h/list_rchan.h
syn_oraddr.o: ../../h/ap.h
syn_oraddr.o: dlist.h
syn_oraddr.o: ../../h/util.h
syn_oraddr.o: ../../h/or.h
syn_oraddr.o: DL-types.h
syn_oraddr.o: dlist.h
syn_oraddr.o: MD-types.h
syn_oraddr.o: dlist.h
syn_oraddr.o: ../../h/IOB-types.h
syn_oraddr.o: ../../h/MTA-types.h
syn_oraddr.o: ../../h/Toks-types.h
syn_oraddr.o: ../../h/Auth-types.h
syn_orname.o: syn_orname.c
syn_orname.o: ../../h/util.h
syn_orname.o: ../../h/config.h
syn_orname.o: ../../h/ll_log.h
syn_orname.o: ../../h/retcode.h
syn_orname.o: ../../h/adr.h
syn_orname.o: ../../h/list_rchan.h
syn_orname.o: ../../h/chan.h
syn_orname.o: ../../h/table.h
syn_orname.o: ../../h/list_bpt.h
syn_orname.o: ../../h/auth.h
syn_orname.o: ../../h/list_bpt.h
syn_orname.o: ../../h/extension.h
syn_orname.o: ../../h/mta.h
syn_orname.o: ../../h/adr.h
syn_orname.o: ../../h/list_bpt.h
syn_orname.o: ../../h/aparse.h
syn_orname.o: ../../h/ap.h
syn_orname.o: ../../h/util.h
syn_orname.o: ../../h/or.h
syn_orname.o: ../../h/chan.h
syn_orname.o: ../../h/auth.h
syn_orname.o: ../../h/list_rchan.h
syn_orname.o: ../../h/ap.h
syn_orname.o: dlist.h
syn_orname.o: ../../h/util.h
syn_orname.o: ../../h/or.h
syn_orname.o: DL-types.h
syn_orname.o: dlist.h
syn_orname.o: MD-types.h
syn_orname.o: dlist.h
syn_orname.o: ../../h/IOB-types.h
syn_orname.o: ../../h/MTA-types.h
syn_orname.o: ../../h/Toks-types.h
syn_orname.o: ../../h/Auth-types.h
syn_permit.o: syn_permit.c
syn_permit.o: dlist.h
syn_permit.o: ../../h/util.h
syn_permit.o: ../../h/config.h
syn_permit.o: ../../h/ll_log.h
syn_permit.o: ../../h/or.h
syn_permit.o: DL-types.h
syn_permit.o: dlist.h
syn_permit.o: MD-types.h
syn_permit.o: dlist.h
syn_permit.o: ../../h/IOB-types.h
syn_permit.o: ../../h/MTA-types.h
syn_permit.o: ../../h/Toks-types.h
syn_permit.o: ../../h/Auth-types.h
syn_policy.o: syn_policy.c
syn_policy.o: dlist.h
syn_policy.o: ../../h/util.h
syn_policy.o: ../../h/config.h
syn_policy.o: ../../h/ll_log.h
syn_policy.o: ../../h/or.h
syn_policy.o: DL-types.h
syn_policy.o: dlist.h
syn_policy.o: MD-types.h
syn_policy.o: dlist.h
syn_policy.o: ../../h/IOB-types.h
syn_policy.o: ../../h/MTA-types.h
syn_policy.o: ../../h/Toks-types.h
syn_policy.o: ../../h/Auth-types.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
