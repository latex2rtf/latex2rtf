#
# Initial version 28 May 2001 Scott Prahl
#

all:	accentchars.rtf array.rtf cite.rtf eqns.rtf fonts.rtf fontsize.rtf \
        frac.rtf list.rtf logo.rtf misc1.rtf misc2.rtf oddchars.rtf \
        tabular.rtf percent.rtf essential.rtf misc3.rtf misc4.rtf fancy.rtf \
	align.rtf german.rtf box.rtf \
	enc_applemac.rtf enc_cp437.rtf enc_cp865.rtf    enc_latin2.rtf enc_latin5.rtf \
	enc_cp1250.rtf   enc_cp850.rtf enc_decmulti.rtf enc_latin3.rtf enc_latin9.rtf\
	enc_cp1252.rtf   enc_cp852.rtf enc_latin1.rtf   enc_latin4.rtf enc_next.rtf\
	defs.rtf qualisex.rtf ttgfsr7.rtf excalibur.rtf include.rtf ch.rtf\
	spago1.rtf theorem.rtf picture.rtf

LATEX= latex
BIBTEX= bibtex
LATEX2RTF= ../latex2rtf
OPTIONS  = -Z3 -P ../cfg

%.rtf:	%.tex
	$(LATEX2RTF) $(OPTIONS) $<

cite.rtf: cite.tex cite.bib
	$(LATEX) cite > latex.log
	$(BIBTEX) cite > bibtex.log
	$(LATEX) cite > latex.log
	$(LATEX) cite > latex.log
	$(LATEX2RTF) $(OPTIONS) cite

misc2.rtf: misc2.tex
	$(LATEX) misc2 > latex.log
	$(LATEX) misc2 > latex.log
	$(LATEX2RTF) $(OPTIONS) misc2

german.rtf: german.tex
	$(LATEX) german > latex.log
	$(LATEX) german > latex.log
	$(LATEX2RTF) $(OPTIONS) german

ttgfsr7.rtf: ttgfsr7.tex
	$(LATEX) ttgfsr7 > latex.log
	$(LATEX) ttgfsr7 > latex.log
	$(LATEX2RTF) $(OPTIONS) ttgfsr7

check:
	./bracecheck
	
clean:
	rm -f *.dvi *.rtf *.aux *.blg *.bbl *.log *.pdf *.toc *.lot *.idx *.ilg *.ind
	rm -f l2r*tex

.PHONY: clean all
