# $Id: Makefile,v 1.4 2001/11/10 19:31:56 prahl Exp $

all:	latex2rtf.html latex2rtf.info latex2rtf.pdf latex2rtf.txt almostclean

latex2rtf.html:	latex2rtf.texi
	makeinfo --html latex2rtf

latex2rtf.info: latex2rtf.texi
	makeinfo latex2rtf

latex2rtf.txt:  latex2rtf.texi
	makeinfo --no-headers latex2rtf > latex2rtf.txt

latex2rtf.pdf : latex2rtf.texi
	texi2dvi --pdf latex2rtf.texi

almostclean:
	rm -f *.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps *.ind *.ilg *.tmp

clean:	almostclean
	rm -f latex2rtf.pdf latex2rtf.info* latex2rtf.html latex2rtf.txt
