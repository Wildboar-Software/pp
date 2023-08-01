##########################################################################
# Instructions to Make, for generation of nice-looking slides using SLiTeX
##########################################################################

.SUFFIXES:     .b .xb .xwd .c .grf .pct .pic .plt .s .web .tex .dvi .imp .ps .
fig

.c.tex:;       sh -c 'if tgrind -f -lc $< | sed -e 1d -e \$$d > $@; \
                       then exit 0; else rm $@; exit 1; fi'

.fig.pic:;      sh -c 'if f2p < $< > $@; \
                         then exit 0; else rm $@; exit 1; fi'


.pic.tex:;     tpic $<

.web.tex:;     sh -c 'if weave $<; then exit 0; else rm $@; exit 1; fi'

.tex.dvi:;     sh -c 'if slitex \\nonstopmode\\input $<; \
                         then exit 0; else rm $@; exit 1; fi'

.dvi.ps:;      sh -c 'if dvi2ps $(DFLAGS) $< > $@; \
                         then exit 0; else rm $@; exit 1; fi'


all:           ifip88-pp.dvi

ifip88-pp.dvi: ifip88-pp.tex structure.pic structure.tex dir.pic dir.tex

print: ifip88-pp.dvi
               dvi2ps ifip88-pp.dvi | lpr -Ppsc

clean:;                rm -f ifip88-pp.dvi


