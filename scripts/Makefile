#
# Initial version 28 May 2001 Scott Prahl
#
LATEX2PNG= ./latex2png

all:	test1.png test2.png test3.png test4.png /tmp/test1.png test1fig.png

%.png:	%.tex
	$(LATEX2PNG) $<

test1.png: test1.tex
	$(LATEX2PNG) test1
	$(LATEX2PNG) test1.tex
	$(LATEX2PNG) -d 200 test1
	$(LATEX2PNG) -m test1
	$(LATEX2PNG) -c test1
	$(LATEX2PNG) -g test1
	$(LATEX2PNG) -d 200 -g test1

test3.png: test3.tex test3a.tex

/tmp/test1.png: test1.tex
	cp test1.tex /tmp
	$(LATEX2PNG) -H . /tmp/test1
	cp test1fig.eps /tmp
	$(LATEX2PNG) /tmp/test1

test1fig.png: test1fig.eps
	$(LATEX2PNG) test1fig.eps
	$(LATEX2PNG) -d 200 test1fig.eps
	$(LATEX2PNG) -m test1fig.eps
	$(LATEX2PNG) -c test1fig.eps
	$(LATEX2PNG) -g test1fig.eps
	$(LATEX2PNG) -d 200 -g test1fig.eps

clean:
	rm -f *.png *.pbm *.aux *.log *.dvi *.map
	
.PHONY: clean all
