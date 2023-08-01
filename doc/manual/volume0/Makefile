#########################################################################
# Instructions to Make, for generation of nice-looking papers using LaTeX
#########################################################################

.SUFFIXES:	.pic .c .h .s .ry .py .bnf .make .web .tex .dvi .imp .ps

.pic.tex:;	tpic $<

.c.tex:;	sh -c 'if $(TGRIND) -f -lc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.h.tex:;	sh -c 'if $(TGRIND) -f -lc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.s.tex:;	sh -c 'if $(TGRIND) -f -la68k $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.ry.tex:;	sh -c 'if $(TGRIND) -d grindefs -f -lpepy $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.py.tex:;	sh -c 'if $(TGRIND) -d grindefs -f -lpepy $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.bnf.tex:;      sh -c 'if $(TGRIND) -d grindefs -f -lbnf $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.make.tex:;	sh -c 'if $(TGRIND) -f -lsrc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.web.tex:;	sh -c 'if weave $<; then exit 0; else rm $@; exit 1; fi'

.tex.dvi:;	sh -c 'if $(LATEX) \\nonstopmode\\input $<; \
			  then exit 0; else rm $@; exit 1; fi'

.dvi.imp:;	dviimp $(DFLAGS) -i $@ $<

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

all:		volume0.dvi COVER-LETTER.dvi

inst-all:;

install:	inst-all clean

lint:;

clean:;		rm -f volume0.imp volume0.ps volume0.dvi volume0.log \
			volume0.blg _* \#* COVER-LETTER.txt COVER-LETTER.ps \
			COVER-LETTER.dvi COVER-TXT.log \
			COVER-TXT.dvi $(INPUTS)
	
grind:		volume0.dvi 
			true
		dvisp $(DFLAGS) volume0

imp:		volume0.imp 

ps:		volume0.ps

true:;


#############################################################
# volume0
#############################################################

# customization files, et. al., not included below
volume0.dvi:	volume0.vrsn 

# volume0.bbl volume0.ind  -- later

volume0.vrsn:	volume0.tex \
			$(INPUTS) \
			highlights.tex \
			real.tex dist-params.tex distrib.tex \
		version.sh
		@version.sh volume0

$(INPUTS):
	ln -s ../texinputs/$@ .


############################################################
# COVER-LETTER
############################################################

COVER-TXT: COVER-TXT.dvi
	dvi2tty COVER-TXT.dvi | sed 's/ffl/o /g' > $@

COVER-TXT.dvi: COVER-TXT.tex $(INPUTS) contents.tex \
		distrib.tex dist-params.tex announce.tex

COVER-LETTER.dvi: COVER-LETTER.tex $(INPUTS) contents.tex \
		distrib.tex dist-params.tex announce.tex

#############################################################
# miscellany
#############################################################

bibtex:;	bibtex volume0

preview:	volume0.dvi \
			true
		xdvi volume0.dvi

index:;		idx2ind -l volume0

