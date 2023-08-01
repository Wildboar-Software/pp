#########################################################################
# Instructions to Make, for generation of nice-looking papers using LaTeX
#########################################################################

.SUFFIXES:	.pic .c .h .s .ry .py .bnf .make .web .tex .dvi .imp .ps .tai

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

all:		volume2.dvi

inst-all:;

install:	inst-all clean

lint:;

clean:;		rm -f volume2.imp volume2.ps volume2.dvi volume2.log \
			volume2.blg _* \#* $(INPUTS)

grind:		volume2.dvi 
			true
		$(DVISP) $(DFLAGS) volume2

imp:		volume2.imp 

ps:		volume2.ps

true:;


#############################################################
# volume2
#############################################################

# customization files, et. al., not included below
volume2.dvi:	volume2.vrsn 

volume2.vrsn:	volume2.tex \
		    $(INPUTS) \
		    preface.tex \
		    general.tex \
			tablelog.tex \
		    submission.tex \
			adrstruct.tex qstruct.tex prmstruct.tex rpbstruct.tex \
			tablerpcodes.tex io.tex tablecrit.tex \
			tableexplicit.tex rchan.tex eit.tex \
		    channels.tex \
			qmgrif.tex tablediag.tex tablereason.tex \
			outbound.tex delv-rep.tex \
		    delivery.tex volume2.ind \
		version.sh
		@version.sh volume2

$(INPUTS):
	ln -s ../texinputs/$@ .

#############################################################
# miscellany
#############################################################

biblio:;	bibtex volume2

preview:	volume2.dvi \
			true
		xdvi volume2.dvi

index:;		idx2ind -l volume2
