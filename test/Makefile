#
# Initial version 28 May 2001 Scott Prahl
#

all:	accentchars.rtf array.rtf cite.rtf eqns.rtf fonts.rtf fontsize.rtf \
        frac.rtf list.rtf logo.rtf misc1.rtf misc2.rtf oddchars.rtf \
        tabular.rtf percent.rtf essential.rtf misc3.rtf misc4.rtf

LATEX= latex
BIBTEX= bibtex
LATEX2RTF= ../latex2rtf

%.rtf:	%.tex
	$(LATEX2RTF) $<

cite.rtf: cite.tex cite.bib
	$(LATEX) cite > latex.log
	$(BIBTEX) cite > bibtex.log
	$(LATEX) cite > latex.log
	$(LATEX) cite > latex.log
	$(LATEX2RTF) cite

misc2.rtf: misc2.tex
	$(LATEX) misc2 > latex.log
	$(LATEX) misc2 > latex.log
	$(LATEX2RTF) misc2

clean:
	rm -f *.dvi *.rtf *.aux *.blg *.bbl *.log *.pdf *.toc *.lot

.PHONY: clean all
