#
# Initial version 28 May 2001 Scott Prahl
#

all:	accentchars.rtf array.rtf cite.rtf eqns.rtf fonts.rtf fontsize.rtf \
        frac.rtf list.rtf logo.rtf misc1.rtf misc2.rtf oddchars.rtf \
        tabular.rtf

RTFPATH= ../cfg

%.rtf:	%.tex
	../latex2rtf $<

cite.rtf: cite.tex cite.bib
	latex cite > latex.log
	bibtex cite > bibtex.log
	latex cite > latex.log
	latex cite > latex.log
	../latex2rtf cite

misc2.rtf: misc2.tex
	latex misc2 > latex.log
	latex misc2 > latex.log
	../latex2rtf misc2

clean:
	rm -f *.dvi *.rtf *.aux *.blg *.bbl *.log

.PHONY: clean
