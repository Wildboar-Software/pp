#########################################################################
# Instructions to Make, for generation of nice-looking papers using LaTeX
#########################################################################

.SUFFIXES:	.pic .c .h .s .ry .py .bnf .make .web .tex .dvi .imp .ps \
		.tai .mf

.mf.tex:;	sh -c 'if $(TGRIND) -f -d $(GRINDEFS) -lmf $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.tex.dvi:;	sh -c 'if $(LATEX) \\nonstopmode\\input $<; \
		then exit 0; else rm $@; exit 1; fi'

.pic.tex:;	tpic $<

.tai.tex:;	sh -c 'if $(TGRIND) -f -d $(GRINDEFS) -ltai $< \
				| sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.c.tex:;	sh -c 'if $(TGRIND) -f -lc $< | sed -e 1d -e \$$d > $@; \
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

all:		volume3.dvi

inst-all:;

install:	inst-all clean

lint:;

clean:;		rm -f volume3.imp volume3.ps volume3.dvi volume3.log \
			volume3.blg _* \#* $(INPUTS)

grind:		volume3.dvi 
			true
		$(DVISP) $(DFLAGS) volume3

imp:		volume3.imp 

ps:		volume3.ps

true:;


#############################################################
# volume3
#############################################################

# customization files, et. al., not included below
volume3.dvi:	volume3.vrsn 

volume3.vrsn:	volume3.tex \
		    $(INPUTS) \
		    intro.tex \
		    local.tex \
			local-examp.tex \
			local-bnf.tex \
		    lists.tex \
		    utility.tex \
		    volume3.ind \
		version.sh
		@version.sh volume3

$(INPUTS):
	ln -s ../texinputs/$@ .

#############################################################
# miscellany
#############################################################

biblio:;	bibtex volume3

preview:	volume3.dvi \
			true
		xdvi volume3.dvi

index:;		idx2ind -l volume3
