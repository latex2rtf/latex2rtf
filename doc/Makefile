# $Id: Makefile,v 1.1 2001/08/12 21:15:46 prahl Exp $
# History:
# $Log: Makefile,v $
# Revision 1.1  2001/08/12 21:15:46  prahl
#         Removed last two // comments
#         Explicitly cast char to int in isalpha() and isdigit()
#         Began the process of supporting Babel better
#
# Revision 1.2  1998/11/12 16:48:35  glehner
# Updated and added r2l.txt target. Adapted for MS-DOS filenames
#
# Revision 1.1  1998/07/03 06:34:41  glehner
# Initial revision
#

all:	l2r.html latex2rtf.info l2r.pdf l2r.txt latex2rtf.dvi almostclean


%.html:	%.texi
	texi2html -monolithic $<

%.info: %.texi
	makeinfo $<

l2r.texi:	latex2rtf.texi
	sed -e"s/@ltor{}/LaTeX2rtf/" -e"s/@LaTeX{}/LaTeX/" \
	    -e"s/@ltor/LaTeX2rtf/" -e"s/@LaTeX/LaTeX/" \
	$< >l2r.texi

l2r.txt: latex2rtf.texi
	makeinfo --no-headers --no-split --no-validate -o- $< | pr -f > l2r.txt

l2r.dvi: latex2rtf.texi
	tex $<
	mv latex2rtf.dvi l2r.dvi

%.ps:	%.dvi
	dvips $< -o

%.pdf:	%.ps
	gs -sDEVICE=pdfwrite -dNOPAUSE -sOutputFile=$*.pdf -c -- $<


almostclean:
	rm -rf *.ps  core *~ ./#* \
	*.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps \
	l2r.texi

clean:	almostclean
	rm -rf *.pdf *.info* *.html *.dvi l2r.txt



