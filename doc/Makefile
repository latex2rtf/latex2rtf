# $Id: Makefile,v 1.8 2002/11/28 18:08:05 prahl Exp $

all:	latex2rtf.html latex2rtf.info latex2rtf.pdf latex2rtf.txt \
		latex2png.hlp latex2rtf.hlp almostclean

latex2rtf.html:	latex2rtf.texi
	makeinfo --html --no-split latex2rtf

latex2rtf.info: latex2rtf.texi
	makeinfo latex2rtf

latex2rtf.txt:  latex2rtf.texi
	makeinfo --no-headers latex2rtf > latex2rtf.txt

latex2rtf.pdf : latex2rtf.texi
	texi2dvi --pdf latex2rtf.texi

latex2png.hlp: latex2png.1
	nroff -man <latex2png.1 | perl -pe 's/.\x08//g' > latex2png.hlp
	
latex2rtf.hlp: latex2rtf.1
	nroff -man <latex2rtf.1 | perl -pe 's/.\x08//g' > latex2rtf.hlp
	
almostclean:
	rm -f *.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps *.ind *.ilg *.tmp

clean:	almostclean
	rm -f latex2rtf.pdf latex2rtf.info latex2rtf.txt latex2rtf.html
	rm -f latex2rtf.hlp latex2png.hlp
