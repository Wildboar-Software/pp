# Installation file for the INTERNET basic setup
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

TABLES	= \
	aliases	\
	auth.channel \
	auth.mta \
	auth.qmgr \
	auth.user \
	ch.list \
	ch.local \
	ch.shell \
	ch.uucp \
	ch.x400in84 \
	ch.x400in88 \
	ch.x400out84 \
	ch.x400out88 \
	channel \
	domain \
	foo \
	or \
	or2rfc \
	rfc1148gate \
	rfc2or \
	users

WARNINGS = \
	warning.1 \
	warning.2

BUILD	= $(TABLES) $(WARNINGS) README isoentities.add tailor x500.sample
EXT	= I
WARNDIR	= $(TBLDIR)/warnings

############################################################
#
# Building Rules
#
############################################################

default: build

install: build inst-tailor inst-tables inst-warnings


inst-tailor: tailor
	@sh -c "if [ -f $(TAILOR) ];\
	then echo $(TAILOR) already exists - installation aborted; exit 1;\
	else exit 0; \
	fi"
	cp tailor $(TAILOR)

inst-tables: $(TABLES)
	@for i in $(TABLES);\
	do \
		if [ -f $(TBLDIR)/$$i ]; \
		then \
			echo mv $(TBLDIR)/$$i $(TBLDIR)/$$i.bak; \
			mv $(TBLDIR)/$$i $(TBLDIR)/$$i.bak || exit 1; \
		fi; \
		echo cp $$i $(TBLDIR); \
		cp $$i $(TBLDIR) || exit 1; \
	done

# This was edited manually to fix some weird bash scripting error.
inst-warnings: $(WARNINGS)
	mkdir -p $(WARNDIR)
	$(CHOWN) pp $(WARNDIR)
	$(CHMOD) 755 $(WARNDIR)
	cp warning.1 $(WARNDIR)
	cp warning.2 $(WARNDIR)


rebuild: clean build

clean: ; rm -f $(BUILD)

build: $(BUILD)
$(BUILD):
	-@rm -f $@
	@sh -c "if [ -f ../master/$@-$(EXT) ]; then \
		echo ln ../master/$@-$(EXT) $@; \
		ln ../master/$@-$(EXT) $@ || exit 1; \
	else \
		echo ln ../master/$@ $@; \
		ln ../master/$@ $@ || exit 1; \
	fi"

############################################################
#
# End of Building Rules
#
############################################################
