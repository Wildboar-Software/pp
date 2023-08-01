#########################################################################
# Instructions to Make, for generation of nice-looking papers using LaTeX
#########################################################################

.SUFFIXES:	.b .xb .xwd .c .grf .pct .pic .plt .s .web .tex .dvi .imp .ps \
		.ry .py .grap .chem

.ry.tex:;	sh -c 'if tgrind -d grindefs -f -lasn1 $< |\
			sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.py.tex:;	sh -c 'if tgrind -d grindefs -f -lasn1 $< |\
			sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.grap.tex:;	grap $< > $*.pic
		tpic $*.pic
		rm -f $*.pic

.chem.tex:;	chem $< > $*.pic
		tpic $*.pic
		rm -f $*.pic

.b.tex:;	itbit $<

.xb.tex:;	xtbit $<

.xwd.tex:;	xtwd $<

.c.tex:;	sh -c 'if tgrind -f -lc $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.grf.tex:;	sh -c 'if graph < $< | plot -TeX > $@; \
			then exit 0; else rm $@; exit 1; fi'

.pct.pic:;	pct2pic $<

.pic.tex:;	tpic $<

.plt.tex:;	sh -c 'if plot -TeX < $< > $@; \
			then exit 0; else rm $@; exit 1; fi'

.s.tex:;	sh -c 'if tgrind -f -la68k $< | sed -e 1d -e \$$d > $@; \
			then exit 0; else rm $@; exit 1; fi'

.web.tex:;	sh -c 'if weave $<; then exit 0; else rm $@; exit 1; fi'

.tex.dvi:;	sh -c 'if latex \\nonstopmode\\input $<; \
			  then exit 0; else rm $@; exit 1; fi'

.dvi.imp:;	dviimp $(DFLAGS) -i $@ $<

.dvi.ps:;	sh -c 'if dvialw $(DFLAGS) $< ; \
			  then exit 0; else rm $@; exit 1; fi'
		mv $*.alw $@


#############################################################
# Here it is...
#############################################################



#############################################################
# pp
#############################################################

all:		pp.dvi pp-slides.dvi bof-slides.dvi

print:		pp.ps pp-slides.ps bof-slides.ps

# customization files, et. al., not included below
pp.dvi:	pp.vrsn

pp.vrsn:	pp.tex pp.bbl version.sh figure1.tex figure2.tex
		@version.sh pp

pp-slides.dvi: pp-slides.tex
		sh -c 'if slitex \\nonstopmode\\input $<; \
			  then exit 0; else rm $@; exit 1; fi'
	
bof-slides.dvi: bof-slides.tex
		sh -c 'if slitex \\nonstopmode\\input $<; \
			  then exit 0; else rm $@; exit 1; fi'

biblio:;	bibtex pp

preview:	pp.dvi true
		xtex pp.dvi

index:;		idx2ind -l pp

clean:;		rm -f *.ps *.dvi *.log \
			*.blg _* 

true:;
