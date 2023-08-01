# Utility routines & functions
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Lib/util/RCS/Makefile,v 6.0 1991/12/18 20:25:18 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:25:18  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################



SRCS =  arg2str.c compress.c err_abrt.c expand.c getfpath.c lock.c \
           multcat.c multcpy.c pe_fragment.c prefix.c rename_log.c \
           rp_valstr.c siginit.c str2arg.c timeout.c timer.c tryfork.c \
           utctime.c check_close.c flip_bits.c sys5_regex.c \
	   cmdbsrch.c hier_scanQ.c oid2lstr.c lstr2oid.c recrm.c diskfull.c
OBJS =  arg2str.o compress.o err_abrt.o expand.o getfpath.o lock.o \
           multcat.o multcpy.o pe_fragment.o prefix.o rename_log.o \
           rp_valstr.o siginit.o str2arg.o timeout.o timer.o tryfork.o \
           utctime.o check_close.o flip_bits.o sys5_regex.o \
	   cmdbsrch.o hier_scanQ.o oid2lstr.o lstr2oid.o recrm.o diskfull.o

# DO NOT DELETE THIS LINE MAKE DEFINE NEEDS IT

HEADERS         = ../../h
CFLAGS          = $(LIBCCOPTIONS) -I$(HEADERS) $(LCF)
LLFLAGS         = $(LINTFLAGS) -I$(HEADERS) -u
SHADOW          = util-made

LINTLIBS        = ../addr/llib-laddr.ln \
		  ../charset/llib-lcs.ln \
		  ../format/llib-lfmt.ln \
		  ../io/llib-lio.ln \
		  ../or/llib-lor.ln \
		  ../parse/llib-lparse.ln \
		  ../pp/llib-lpp.ln \
		  ../qmgr/llib-lqmgr.ln \
		  ../table/llib-ltable.ln \
		  ../tai/llib-ltai.ln \
		  ../x400/llib-lx400.ln


############################################################
#
# Building Rules
#
############################################################

default: ${SHADOW}

${SHADOW}: $(OBJS)
	$(AR) r$(ARFLAGS) ../libpp.a $?
	touch $@

saber_src: $(SRCS)
	#load -C $(CFLAGS) $(SRCS) $(LIBPP) $(LIBSYS)

saber_obj: $(OBJS) 
	#load -C $(OBJS) $(LIBPP) $(LIBSYS)

lintlib: llib-lutil.ln
llib-lutil.ln: ${SRCS}
	$(LINT) -Cutil $(LLFLAGS) ${SRCS}

lint:
	$(LINT) $(LLFLAGS) $(SRCS) $(LINTLIBS) $(LINTISODE)

install:;

clean:  tidy
	rm -f $(OBJS) llib-lutil.ln
tidy:
	rm -f core ${SHADOW} *.old *.BAK


define:
	$(DEFINE) Makefile

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)


############################################################
#
# End of building rules
#
############################################################

# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
arg2str.o: arg2str.c
arg2str.o: ../../h/util.h
arg2str.o: ../../h/config.h
arg2str.o: ../../h/ll_log.h
arg2str.o: ../../h/retcode.h
compress.o: compress.c
compress.o: ../../h/util.h
compress.o: ../../h/config.h
compress.o: ../../h/ll_log.h
err_abrt.o: err_abrt.c
err_abrt.o: ../../h/head.h
err_abrt.o: ../../h/util.h
err_abrt.o: ../../h/config.h
err_abrt.o: ../../h/ll_log.h
err_abrt.o: ../../h/retcode.h
expand.o: expand.c
expand.o: ../../h/util.h
expand.o: ../../h/config.h
expand.o: ../../h/ll_log.h
expand.o: ../../h/expand.h
getfpath.o: getfpath.c
getfpath.o: ../../h/util.h
getfpath.o: ../../h/config.h
getfpath.o: ../../h/ll_log.h
lock.o: lock.c
lock.o: ../../h/util.h
lock.o: ../../h/config.h
lock.o: ../../h/ll_log.h
lock.o: ../../h/sys.file.h
multcat.o: multcat.c
multcat.o: ../../h/util.h
multcat.o: ../../h/config.h
multcat.o: ../../h/ll_log.h
multcpy.o: multcpy.c
pe_fragment.o: pe_fragment.c
prefix.o: prefix.c
prefix.o: ../../h/util.h
prefix.o: ../../h/config.h
prefix.o: ../../h/ll_log.h
rename_log.o: rename_log.c
rename_log.o: ../../h/head.h
rename_log.o: ../../h/util.h
rename_log.o: ../../h/config.h
rename_log.o: ../../h/ll_log.h
rename_log.o: ../../h/retcode.h
rp_valstr.o: rp_valstr.c
rp_valstr.o: ../../h/head.h
rp_valstr.o: ../../h/util.h
rp_valstr.o: ../../h/config.h
rp_valstr.o: ../../h/ll_log.h
rp_valstr.o: ../../h/retcode.h
siginit.o: siginit.c
siginit.o: ../../h/util.h
siginit.o: ../../h/config.h
siginit.o: ../../h/ll_log.h
str2arg.o: str2arg.c
str2arg.o: ../../h/util.h
str2arg.o: ../../h/config.h
str2arg.o: ../../h/ll_log.h
timeout.o: timeout.c
timeout.o: ../../h/util.h
timeout.o: ../../h/config.h
timeout.o: ../../h/ll_log.h
timer.o: timer.c
timer.o: ../../h/util.h
timer.o: ../../h/config.h
timer.o: ../../h/ll_log.h
tryfork.o: tryfork.c
tryfork.o: ../../h/util.h
tryfork.o: ../../h/config.h
tryfork.o: ../../h/ll_log.h
utctime.o: utctime.c
utctime.o: ../../h/util.h
utctime.o: ../../h/config.h
utctime.o: ../../h/ll_log.h
check_close.o: check_close.c
check_close.o: ../../h/util.h
check_close.o: ../../h/config.h
check_close.o: ../../h/ll_log.h
flip_bits.o: flip_bits.c
sys5_regex.o: sys5_regex.c
sys5_regex.o: ../../h/config.h
cmdbsrch.o: cmdbsrch.c
cmdbsrch.o: ../../h/util.h
cmdbsrch.o: ../../h/config.h
cmdbsrch.o: ../../h/ll_log.h
hier_scanQ.o: hier_scanQ.c
hier_scanQ.o: ../../h/util.h
hier_scanQ.o: ../../h/config.h
hier_scanQ.o: ../../h/ll_log.h
oid2lstr.o: oid2lstr.c
oid2lstr.o: ../../h/util.h
oid2lstr.o: ../../h/config.h
oid2lstr.o: ../../h/ll_log.h
lstr2oid.o: lstr2oid.c
lstr2oid.o: ../../h/util.h
lstr2oid.o: ../../h/config.h
lstr2oid.o: ../../h/ll_log.h
recrm.o: recrm.c
recrm.o: ../../h/util.h
recrm.o: ../../h/config.h
recrm.o: ../../h/ll_log.h
diskfull.o: diskfull.c
diskfull.o: ../../h/util.h
diskfull.o: ../../h/config.h
diskfull.o: ../../h/ll_log.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
