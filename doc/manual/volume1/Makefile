#########################################################################
# Instructions to Make, for generation of nice-looking papers using LaTeX
#########################################################################

.SUFFIXES:	.pic .c .h .s .ry .py .bnf .make .web .tex .dvi .imp \
		.ps .tai .sh .csh

.tex.dvi:;	sh -c 'if $(LATEX) \\nonstopmode\\input $<; \
		then exit 0; else rm $@; exit 1; fi'

.pic.tex:;	tpic $<

.tai.tex:;	sh -c 'if $(TGRIND) -f -d $(GRINDEFS) -ltai $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.c.tex:;	sh -c 'if $(TGRIND) -f -lc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.sh.tex:;	sh -c 'if $(TGRIND) -f -lsh $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.csh.tex:;	sh -c 'if $(TGRIND) -f -lcsh $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.h.tex:;	sh -c 'if $(TGRIND) -f -lc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.s.tex:;	sh -c 'if $(TGRIND) -f -la68k $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.ry.tex:;	sh -c 'if $(TGRIND) -d $(GRINDEFS) -f -lpepy $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.py.tex:;	sh -c 'if $(TGRIND) -d $(GRINDEFS) -f -lpepy $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.bnf.tex:;      sh -c 'if $(TGRIND) -d $(GRINDEFS) -f -lbnf $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.make.tex:;	sh -c 'if $(TGRIND) -f -lsrc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.web.tex:;	sh -c 'if $(WEAVE) $<; then exit 0; else rm $@; exit 1; fi'

.dvi.imp:;	$(DVIIMP) $(DFLAGS) -i $@ $<

.dvi.ps:;	sh -c 'if $(DVI2PS) $(DFLAGS) $< > $@; \
			  then exit 0; else rm $@; exit 1; fi'

#########################################################################
# End of extension rules
#########################################################################

INPUTS	= \
	bcustom.bib     grindefs        networking.bib  sfwmac.sty \
	captcont.sty    grindefs.sty    pp.bib          tgrind.sty \
	dist-params.tex lcustom.tex     sfwdoc.bib      trademark.tex \
	a4.sty

#############################################################
# Here it is...
#############################################################

all:		volume1.dvi

inst-all:;

install:	inst-all clean

lint:;

clean:;		rm -f volume1.imp volume1.ps volume1.dvi volume1.log \
			volume1.blg _* \#* $(INPUTS)

grind:		volume1.dvi 
			true
		$(DVISP) $(DFLAGS) volume1

imp:		volume1.imp 

ps:		volume1.ps

true:;


#############################################################
# volume1
#############################################################

# customization files, et. al., not included below
volume1.dvi:	volume1.vrsn 

volume1.vrsn:	volume1.tex \
		    $(INPUTS) \
		    dist-params.tex \
		    preface.tex \
		    general.tex \
		    install.tex \
			tablemakedefs1.tex \
			tablemakedefs2.tex \
		    configure.tex \
			log-examp.tex chan-examp.tex table-examp.tex \
			tableinfo_expan.tex filt-examp.tex \
			shelfilt.tex \
		    tables.tex \
			alias.tex \
			chantbl.tex domain.tex or.tex users.tex \
			or2rfc.tex rfc2or.tex lists-examp.tex \
			shell-examp.tex \
			tableauthreasons.tex rfc1148g.tex \
		    x400links.tex \
			tablex400info1.tex tablex400info2.tex \
			exampleortbl.tex exampleortbl2.tex \
			chx400out.tex chanx400examp.tex \
			x400in-examp2.tex logx400-examp.tex \
		    addon.tex \
			faxTable.tex faxtai.tex \
			asnfilter.tex \
		    management.tex \
		        console_tai.tex \
			tableconsole_accels.tex \
			tableconsole_mapping.tex \
			qfile.tex dreport.tex\
		    appendix.tex \
			tailor.tex pp_start.tex pp_nightly.tex \
			x400tables.tex \
		    qmgrproto.tex qmgrros.tex \
		    volume1.ind \
		    volume1.bbl \
		version.sh
		@version.sh volume1


$(INPUTS):
	ln -s ../texinputs/$@ .

#############################################################
# miscellany
#############################################################

biblio:;	bibtex volume1

preview:	volume1.dvi \
			true
		xdvi volume1.dvi

index:;		idx2ind -l volume1
