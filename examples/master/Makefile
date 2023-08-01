# Master Makefile.
#
############################################################
#
# @(#) $Header$
#
# $Log$
#
############################################################
#
# Definitions
#
############################################################

FILES	= README-I README-J README-L aliases auth.channel auth.mta \
	auth.qmgr auth.user ch.list ch.local ch.shell ch.uucp ch.x400in84 \
	ch.x400in88 ch.x400out84 ch.x400out88 channel-I channel-J channel-L \
	domain-I domain-J domain-L foo isoentities.add or or2rfc rfc2or \
	tailor-I tailor-J tailor-L users


############################################################
#
# Building Rules
#
############################################################

build: $(FILES)
	@echo "All built"

clean:
	rm -f README-? sed.cmds tailor-? tailor-?.rest

install:;

README-I: README-master Makefile sed.cmds
	sed -e '/^%BEGIN(I)/d' -e '/^%END(I)/d' -e '/^%BEGIN/,/^%END/d' \
		-f sed.cmds \
		README-master | nroff > $@

README-J: README-master Makefile sed.cmds
	sed -e '/^%BEGIN(J)/d' -e '/^%END(J)/d' -e '/^%BEGIN/,/^%END/d' \
		-f sed.cmds \
		README-master | nroff > $@

README-L: README-master Makefile sed.cmds
	sed -e '/^%BEGIN(L)/d' -e '/^%END(L)/d' -e '/^%BEGIN/,/^%END/d' \
		-f sed.cmds \
		README-master | nroff > $@

sed.cmds: ../../Make.defs Makefile
	echo "s:%TAILOR%:$(TAILOR):" > $@
	echo "s:%TABLES%:$(TBLDIR):" >> $@
	echo "s:%CMDDIR%:$(CMDDIR):" >> $@
	echo "s:%TOOLS%:$(TOOLDIR):" >> $@

tailor-I: tailor-I.head tailor-I.rest
	cat tailor-I.head tailor-I.rest > $@
tailor-I.rest: tailor.common
	sed -e '/^%BEGIN(I)/d' -e '/^%END(I)/d' -e '/^%BEGIN/,/^%END/d' \
		$? > $@

tailor-J: tailor-J.head tailor-J.rest
	cat tailor-J.head tailor-J.rest > $@
tailor-J.rest:tailor.common
	sed -e '/^%BEGIN(J)/d' -e '/^%END(J)/d' -e '/^%BEGIN/,/^%END/d' \
		$? > $@

tailor-L: tailor-L.head tailor-L.rest
	cat tailor-L.head tailor-L.rest > $@
tailor-L.rest:tailor.common
	sed -e '/^%BEGIN(L)/d' -e '/^%END(L)/d' -e '/^%BEGIN/,/^%END/d' \
		$? > $@

############################################################
#
# End of Building Rules
#
############################################################
