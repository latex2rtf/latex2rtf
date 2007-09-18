# $Id: Makefile,v 1.9 2002/11/29 16:36:42 prahl Exp $

all:	latex2rtf.html latex2rtf.info latex2rtf.pdf latex2rtf.txt \
		latex2pn.txt latex2rt.txt almostclean

latex2rtf.html:	latex2rtf.texi
	makeinfo --html --no-split latex2rtf
	chmod 644 latex2rtf.html

latex2rtf.info: latex2rtf.texi
	makeinfo latex2rtf
	chmod 644 latex2rtf.info

latex2rtf.txt:  latex2rtf.texi
	makeinfo --no-headers latex2rtf > latex2rtf.txt
	chmod 644 latex2rtf.txt

latex2rtf.pdf : latex2rtf.texi
	texi2dvi --pdf latex2rtf.texi
	chmod 644 latex2rtf.pdf

latex2pn.txt: latex2png.1
	nroff -man <latex2png.1 | perl -pe 's/.\x08//g' > latex2pn.txt
	
latex2rt.txt: latex2rtf.1
	nroff -man <latex2rtf.1 | perl -pe 's/.\x08//g' > latex2rt.txt
	
almostclean:
	rm -f *.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps *.ind *.ilg *.tmp

clean:	almostclean
	rm -f latex2rtf.pdf latex2rtf.info latex2rtf.txt latex2rtf.html
	rm -f latex2rtf.hlp latex2png.hlp
