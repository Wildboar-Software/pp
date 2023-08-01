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

.dvi.ps:;	sh -c 'if dvi2ps $(DFLAGS) $< > $@; \
			  then exit 0; else rm $@; exit 1; fi'


#############################################################
# Here it is...
#############################################################

all:		i-all
print:		i-print


#############################################################
# tailor
#############################################################

install:;

i-all:		tailor.dvi

i-print:	tailor.dvi true
		dvisp $(DFLAGS) tailor


# customization files, et. al., not included below
tailor.dvi:	tailor.vrsn

tailor.vrsn:	tailor.tex version.sh
		@version.sh

preview:	tailor.dvi true
		xdvi tailor.dvi

index:;		idx2ind -l tailor

clean:;		rm -f tailor.imp tailor.ps tailor.dvi tailor.log \
			tailor.blg _*

true:;
