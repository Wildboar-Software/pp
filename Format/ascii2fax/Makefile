# ascii2fax
#
############################################################
#
# @(#) $Header: /xtel/pp/pp-beta/Format/ascii2fax/RCS/Makefile,v 6.0 1991/12/18 20:15:19 jpo Rel $
#
# $Log: Makefile,v $
# Revision 6.0  1991/12/18  20:15:19  jpo
# Release 6.0
#
#
############################################################
#
# Definitions
#
############################################################

SRCS	= bitmap_util.c char_util.c font_util.c str_util.c \
	ia52fax.c hdr2fax.c
OBJS	= bitmap_util.o char_util.o font_util.o str_util.o \
	ia52fax.o hdr2fax.o

PROGS	= xia52fax xhdr2fax xfontconv ximageconv

UTIL	= bitmap_util.o char_util.o font_util.o str_util.o

HEADERS	=	../../h
LIBPP	=	../../Lib/libpp.a

CFLAGS	= $(CCOPTIONS) -I$(HEADERS) $(FAXPAGESIZE)
LDFLAGS	= $(LDOPTIONS)

LLFLAGS = $(LINTFLAGS) -I$(HEADERS)
LINTLIBS = ../../Lib/llib-lpp.ln $(LINTISODE)

############################################################
#
# Building Rules
#
############################################################

default: $(PROGS)

############################################################
#
# ia52fax converts pages of ia5 to g3fax images
#
############################################################

xia52fax: $(UTIL) ia52fax.o image2fax.o $(LIBPP)
	       $(CC) $(LDFLAGS) $(UTIL) ia52fax.o image2fax.o \
	       -o $@ $(LIBPP) $(LIBSYS) 

############################################################
#
# hdr2fax converts 822 hdr to g3fax image of coverpage
#
############################################################

xhdr2fax: $(UTIL) hdr2fax.o image2fax.o $(LIBPP)
	       $(CC) $(LDFLAGS) $(UTIL) hdr2fax.o image2fax.o \
	       -o $@ $(LIBPP) $(LIBSYS) 

############################################################
#
# font converter: converts X fonts to PP fonts
#
############################################################

xfontconv: Xfont2PPfont.o $(UTIL) $(LIBPP)
	$(CC) $(LDFLAGS) Xfont2PPfont.o $(UTIL) \
	-o $@ $(LIBPP) $(LIBSYS) -lXt -lX11

############################################################
#
# imageconv converts X images to PP bitmaps
#
############################################################

ximageconv: X2PPbitmap.o $(UTIL)
	       $(CC) $(LDFLAGS) X2PPbitmap.o $(UTIL) \
	       -o $@ $(LIBPP) $(LIBSYS) -lXt -lX11

############################################################
#
# Building Rules
#
############################################################


install: inst-dir inst-ia52fax inst-hdr2fax

inst-dir: $(FORMDIR)

inst-ia52fax: $(FORMDIR)/ia52fax

$(FORMDIR)/ia52fax: xia52fax
		     -$(BACKUP) $@ zxia52fax
		     rm -f $@           
		     $(INSTALL) xia52fax $@
		     -$(CHMOD) $(PGMPROT) $@
		     -$(CHOWN) $(PPUSER) $@
		     -@ls -ls $@
		     -@echo "ia52fax filter installed normally"; echo ""

inst-hdr2fax: $(FORMDIR)/hdr2fax

$(FORMDIR)/hdr2fax: xhdr2fax
		     -$(BACKUP) $@ zxhdr2fax
		     rm -f $@           
		     $(INSTALL) xhdr2fax $@
		     -$(CHMOD) $(PGMPROT) $@
		     -$(CHOWN) $(PPUSER) $@
		     -@ls -ls $@
		     -@echo "hdr2fax filter installed normally"; echo ""

clean: tidy
	rm -rf $(PROGS)

tidy:
	rm -f $(OBJS) Xfont2PPfont.o X2PPbitmap.o image2fax.o \
		core a.out Makefile.old

lint: l-targets

depend:
	$(DEPEND) -I$(HEADERS) $(SRCS)

############################################################
#
# End of Building Rules
#
############################################################
# DO NOT DELETE THIS LINE MAKE DEPEND NEEDS IT
# Dependencies follow
bitmap_util.o: bitmap_util.c
bitmap_util.o: ./fonts.h
bitmap_util.o: ../../h/util.h
bitmap_util.o: ../../h/config.h
bitmap_util.o: ../../h/ll_log.h
bitmap_util.o: ../../h/sys.file.h
char_util.o: char_util.c
char_util.o: ./fonts.h
char_util.o: ../../h/util.h
char_util.o: ../../h/config.h
char_util.o: ../../h/ll_log.h
char_util.o: ../../h/sys.file.h
font_util.o: font_util.c
font_util.o: ./fonts.h
font_util.o: ../../h/util.h
font_util.o: ../../h/config.h
font_util.o: ../../h/ll_log.h
font_util.o: ../../h/sys.file.h
str_util.o: str_util.c
str_util.o: ./fonts.h
str_util.o: ../../h/util.h
str_util.o: ../../h/config.h
str_util.o: ../../h/ll_log.h
str_util.o: ../../h/sys.file.h
ia52fax.o: ia52fax.c
ia52fax.o: ../../h/util.h
ia52fax.o: ../../h/config.h
ia52fax.o: ../../h/ll_log.h
ia52fax.o: ../../h/IOB-types.h
ia52fax.o: ../../h/MTA-types.h
ia52fax.o: ../../h/Toks-types.h
ia52fax.o: ../../h/Auth-types.h
ia52fax.o: ../../h/table.h
ia52fax.o: ./fonts.h
ia52fax.o: ../../h/util.h
ia52fax.o: ../../h/sys.file.h
ia52fax.o: ./pg_sizes.h
hdr2fax.o: hdr2fax.c
hdr2fax.o: ../../h/util.h
hdr2fax.o: ../../h/config.h
hdr2fax.o: ../../h/ll_log.h
hdr2fax.o: ../../h/or.h
hdr2fax.o: ../../h/IOB-types.h
hdr2fax.o: ../../h/MTA-types.h
hdr2fax.o: ../../h/Toks-types.h
hdr2fax.o: ../../h/Auth-types.h
hdr2fax.o: ../../h/table.h
hdr2fax.o: ./fonts.h
hdr2fax.o: ../../h/util.h
hdr2fax.o: ../../h/sys.file.h
hdr2fax.o: ../../h/ap.h
hdr2fax.o: ../../h/util.h
hdr2fax.o: ./pg_sizes.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
