# $Id: Makefile,v 1.3 2001/10/29 05:00:51 prahl Exp $

all:	latex2rtf.html latex2rtf.info latex2rtf.pdf latex2rtf.txt almostclean

%.html:	%.texi
	makeinfo --html $<

%.info: %.texi
	makeinfo $<

%.txt:  %.texi
	makeinfo --no-headers $<  > latex2rtf.txt

%.pdf : %.texi
	texi2dvi --pdf $<

almostclean:
	rm -f *.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps *.ind *.ilg *.tmp

clean:	almostclean
	rm -f latex2rtf.pdf latex2rtf.info* latex2rtf.html latex2rtf.txt
