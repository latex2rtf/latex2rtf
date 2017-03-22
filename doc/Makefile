all:	latex2rtf.info latex2rtf.pdf latex2rtf.txt \
		latex2pn.txt latex2rt.txt latex2rtf.html 

latex2rtf.html:	latex2rtf.texi
	makeinfo --html --no-split --no-headers latex2rtf.texi > latex2rtf.html
	chmod 644 latex2rtf.html

latex2rtf.info: latex2rtf.texi
	makeinfo latex2rtf
	chmod 644 latex2rtf.info

latex2rtf.txt:  latex2rtf.texi
	makeinfo --no-headers latex2rtf > latex2rtf.txt
	chmod 644 latex2rtf.txt

latex2rtf.pdf : latex2rtf.texi
	texi2pdf latex2rtf.texi
	chmod 644 latex2rtf.pdf

latex2pn.txt: latex2png.1
	man ./latex2png.1 | col -b > latex2pn.txt

latex2rt.txt: latex2rtf.1
	man ./latex2rtf.1 | col -b > latex2rt.txt

web/manpage.html: header.html footer.html latex2rtf.1
	man ./latex2rtf.1 | man2html --pgsize=100000 --bare > mid.html
	perl -pi.bak -e 's/^ {3}//' mid.html
	cat header.html mid.html footer.html > man.html
	sed "s/HHTITLEHH/latex2rtf: man page/" man.html > web/manpage.html
	rm mid.html mid.html.bak man.html

web/latex2png.html: header.html footer.html latex2png.1
	man ./latex2png.1 | man2html --pgsize=100000 --bare > mid.html
	perl -pi.bak -e 's/^ {3}//' mid.html
	cat header.html mid.html footer.html > man.html
	sed "s/HHTITLEHH/latex2rtf: man mage/" man.html > web/latex2png.html
	rm mid.html mid.html.bak man.html

web/usage.html: header.html footer.html latex2rtf.html
    # drop first 24 lines
	tail -n +24 latex2rtf.html > temp1.html
	# drop last 3 lines
	sed -n -e :a -e '1,3!{P;N;D;};N;ba' temp1.html > temp2.html
	# assemble
	cat header.html temp2.html footer.html > temp3.html
	# insert title
	sed "s/HHTITLEHH/latex2rtf: User's Manual/" temp3.html > temp4.html
	# makeinfo creates html files using name=
	sed "s/name=/id=/" temp4.html > web/usage.html
	rm -f temp1.html temp2.html temp3.html temp4.html

install: web/usage.html web/manpage.html web/latex2png.html
	rsync -ave ssh web/ web.sourceforge.net:/home/project-web/latex2rtf/htdocs

clean:
	rm -f *.aux *.toc *.fn *.fns *.ky *.log *.pg *.tp *.vr *.vrs \
	*.cp *.cps *.ind *.ilg *.tmp
	rm -f latex2rt.txt latex2pn.txt

realclean:	clean
	rm -f latex2rtf.pdf latex2rtf.info latex2rtf.txt latex2rtf.html
	rm -f latex2rtf.hlp latex2png.hlp
	rm -f web/usage.html web/latex2png.html web/manpage.html
	rm -f latex2pn.txt latex2rt.txt
	
.PHONY: install clean realclean
