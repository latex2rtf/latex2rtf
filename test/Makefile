#
# Initial version 28 May 2001 Scott Prahl
#
LATEX= latex
BIBTEX= bibtex
LATEX2RTF= ../latex2rtf
OPTIONS  = -Z3 -P ../cfg/:../scripts/ -T tmp/

LATEX_LOG = >latex.log
#LATEX_LOG = 

RTF=	align.rtf	 array.rtf 		box.rtf 			ch.rtf	             \
		chem.rtf 	 color.rtf 		dblspace.rtf		defs.rtf             \
		endnote.rtf	 essential.rtf	excalibur.rtf 		fancy.rtf            \
		fonts.rtf	 fontsize.rtf 	fonttest.rtf  		geometry.rtf         \
		geotest.rtf  german.rtf		head_article.rtf	head_book.rtf        \
		list.rtf 	 ifclause.rtf	include.rtf			linux.rtf            \
		logo.rtf 	 misc1.rtf 		misc3.rtf 			misc4.rtf            \
		percent.rtf	 report.rtf		tabbing.rtf			tabular.rtf          \
		hyperref.rtf longstack.rtf

BIB_TESTS=  bib_apacite.rtf        bib_harvard.rtf        bib_super.rtf      \
			bib_apacite_dblsp.rtf  bib_natbib1.rtf        bib_apalike.rtf    \
			bib_natbib2.rtf        bibentry_apalike.rtf   bib_apalike2.rtf   \
			bib_natbib3.rtf        bib_apanat.rtf         bibentry_plain.rtf \
			bib_authordate.rtf     bib_simple.rtf         bib_apa.rtf        \
			bib_apacite2.rtf
	
FIG_TESTS=  fig_endfloat.rtf  fig_test.rtf      fig_test3.rtf \
            fig_size.rtf      fig_test2.rtf     fig_test4.rtf \
            picture.rtf       fig_subfig.rtf

ENC_TESTS= 	enc_applemac.rtf enc_cp437.rtf  enc_cp865.rtf    enc_latin2.rtf enc_latin5.rtf \
			enc_cp1250.rtf   enc_cp850.rtf  enc_decmulti.rtf enc_latin3.rtf enc_latin9.rtf\
			enc_cp1252.rtf   enc_cp852.rtf  enc_latin1.rtf   enc_latin4.rtf enc_next.rtf\
			enc_cp1251.rtf   enc_cp855.rtf  enc_cp866.rtf    enc_koi8-r.rtf enc_koi8-u.rtf \
			enc_maccyr.rtf   enc_macukr.rtf eqns-koi8.rtf 	 enc_utf8x.rtf

BABEL_TESTS=babel_german.rtf babel_russian.rtf babel_czech.rtf babel_french.rtf \
	        babel_spanish.rtf babel_frenchb.rtf bib_simple.rtf
	        
EQN_TESTS=  eqns.rtf	eqns2.rtf eqns-koi8.rtf ttgfsr7.rtf spago1.rtf misc2.rtf\
            frac.rtf	subsup.rtf	theorem.rtf eqnnumber.rtf eqnnumber2.rtf

CHAR_TESTS= accentchars.rtf oddchars.rtf unicode.rtf overstrike.rtf

basic:
	$(MAKE) basic_tests
	
bib:
	$(MAKE) bib_tests

fig:
	mkdir -p tmp
	$(MAKE) fig_tests

enc:
	$(MAKE) enc_tests
	
eqn:
	$(MAKE) eqn_tests

babel:
	$(MAKE) babel_tests

char:
	$(MAKE) char_tests

all:
	mkdir -p tmp
	$(MAKE) basic_tests
	$(MAKE) bib_tests
	$(MAKE) eqn_tests
	$(MAKE) fig_tests
	$(MAKE) enc_tests
	$(MAKE) char_tests

basic_tests: $(RTF)

bib_tests: $(BIB_TESTS)

fig_tests: $(FIG_TESTS)

enc_tests: $(ENC_TESTS)

babel_tests: $(BABEL_TESTS)

eqn_tests: $(EQN_TESTS)

char_tests: $(CHAR_TESTS)

#these tests require that some non-standard latex style be installed

%.rtf:	%.tex
	$(LATEX2RTF) $(OPTIONS) $<

bib_simple.rtf: bib_simple.tex bib_simple.bib
	$(LATEX) bib_simple $(LATEX_LOG)
	$(BIBTEX) bib_simple > bibtex.log
	$(LATEX) bib_simple $(LATEX_LOG)
	$(LATEX) bib_simple $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_simple

bib_apalike.rtf: bib_apalike.tex
	$(LATEX) bib_apalike $(LATEX_LOG)
	$(LATEX) bib_apalike $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apalike

bib_apa.rtf: bib_apa.tex
	$(LATEX) bib_apa $(LATEX_LOG)
	$(BIBTEX) bib_apa > bibtex.log
	$(LATEX) bib_apa $(LATEX_LOG)
	$(LATEX) bib_apa $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apa

bib_apacite.rtf: bib_apacite.tex
	$(LATEX) bib_apacite $(LATEX_LOG)
	$(LATEX) bib_apacite $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apacite

bib_apacite2.rtf: bib_apacite2.tex
	$(LATEX) bib_apacite2 $(LATEX_LOG)
	$(BIBTEX) bib_apacite2 > bibtex.log
	$(LATEX) bib_apacite2 $(LATEX_LOG)
	$(LATEX) bib_apacite2 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apacite2

bib_apacite_dblsp.rtf: bib_apacite_dblsp.tex
	$(LATEX) bib_apacite_dblsp $(LATEX_LOG)
	$(LATEX) bib_apacite_dblsp $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apacite_dblsp

bib_apalike2.rtf: bib_apalike2.tex
	$(LATEX) bib_apalike2 $(LATEX_LOG)
	$(LATEX) bib_apalike2 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apalike2

bib_natbib1.rtf: bib_natbib1.tex
	$(LATEX) bib_natbib1 $(LATEX_LOG)
	$(LATEX) bib_natbib1 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_natbib1

bib_natbib2.rtf: bib_natbib2.tex
	$(LATEX) bib_natbib2 $(LATEX_LOG)
	$(LATEX) bib_natbib2 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_natbib2

bib_natbib3.rtf: bib_natbib3.tex
	$(LATEX) bib_natbib3 $(LATEX_LOG)
	$(LATEX) bib_natbib3 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_natbib3

bib_apanat.rtf: bib_apanat.tex
	$(LATEX) bib_apanat $(LATEX_LOG)
	$(LATEX) bib_apanat $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_apanat

bib_authordate.rtf: bib_authordate.tex
	$(LATEX) bib_authordate $(LATEX_LOG)
	$(LATEX) bib_authordate $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_authordate

bib_harvard.rtf: bib_harvard.tex bib_harvard.bib
	$(LATEX) bib_harvard $(LATEX_LOG)
	$(BIBTEX) bib_harvard > bibtex.log
	$(LATEX) bib_harvard $(LATEX_LOG)
	$(LATEX) bib_harvard $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_harvard
	rm -rf bib_harvard.out

bibentry_plain.rtf: bibentry_plain.tex bibentry_plain.bib
	$(LATEX) bibentry_plain $(LATEX_LOG)
	$(BIBTEX) bibentry_plain > bibtex.log
	$(LATEX) bibentry_plain $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bibentry_plain

bibentry_apalike.rtf: bibentry_apalike.tex bibentry_apalike.bib
	$(LATEX) bibentry_apalike $(LATEX_LOG)
	$(BIBTEX) bibentry_apalike > bibtex.log
	$(LATEX) bibentry_apalike $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bibentry_apalike

bib_super.rtf: bib_super.tex
	$(LATEX) bib_super $(LATEX_LOG)
	$(BIBTEX) bib_super > bibtex.log
	$(LATEX) bib_super $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) bib_super

misc2.rtf: misc2.tex
	$(LATEX) misc2 $(LATEX_LOG)
	$(LATEX) misc2 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) misc2

german.rtf: german.tex
	$(LATEX) german $(LATEX_LOG)
	$(LATEX) german $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) german

russian.rtf: russian.tex
	$(LATEX) russian > russian.log
	$(LATEX) russian > russian.log
	$(LATEX2RTF) $(OPTIONS) russian

ttgfsr7.rtf: ttgfsr7.tex
	$(LATEX) ttgfsr7 $(LATEX_LOG)
	$(LATEX) ttgfsr7 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) ttgfsr7

fig_test.rtf: fig_test.tex fig_test.eps fig_testc.ps
	$(LATEX) fig_test $(LATEX_LOG)
	$(LATEX) fig_test $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) -F fig_test

fig_test2.rtf: fig_test2.tex fig_testb.pdf fig_testc.pdf
	$(LATEX2RTF) $(OPTIONS) -D 300 fig_test2
	mv fig_test2.rtf fig_test2a.rtf
	$(LATEX2RTF) $(OPTIONS) fig_test2

fig_test3.rtf: fig_test3.tex fig_testd.ps fig_testd.pdf
	$(LATEX2RTF) $(OPTIONS) fig_test3

linux.rtf: linux.tex
	$(LATEX) linux $(LATEX_LOG)
	$(LATEX) linux $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) linux

eqnnumber.rtf: eqnnumber.tex
	$(LATEX) eqnnumber $(LATEX_LOG)
	$(LATEX) eqnnumber $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) eqnnumber

eqnnumber2.rtf: eqnnumber2.tex
	$(LATEX) eqnnumber2 $(LATEX_LOG)
	$(LATEX) eqnnumber2 $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) eqnnumber2

fig_endfloat.rtf: fig_endfloat.tex
	$(LATEX) fig_endfloat $(LATEX_LOG)
	$(LATEX) fig_endfloat $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) fig_endfloat

include.rtf: include.tex include1.tex include2.tex include3.tex
	$(LATEX) include $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) include

report.rtf: report.tex
	$(LATEX) report $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) report

excalibur.rtf: excalibur.tex
	$(LATEX) excalibur $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) excalibur

dblspace.rtf: dblspace.tex
	$(LATEX) dblspace $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) dblspace

eqns.rtf: eqns.tex
	$(LATEX) eqns $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) eqns

essential.rtf: essential.tex
	$(LATEX) essential $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) essential

hyperref.rtf: hyperref.tex
	$(LATEX) hyperref $(LATEX_LOG)
	$(LATEX2RTF) $(OPTIONS) hyperref

check:
	./bracecheck
	
clean:
	mv fig_testb.pdf fig_testb.tmp
	mv fig_testc.pdf fig_testc.tmp
	mv fig_testd.pdf fig_testd.tmp
	mv fig_teste.pdf fig_teste.tmp
	rm -f *.dvi *.rtf *.aux *.blg *.bbl *.log *.pdf *.toc *.lot *.idx *.ilg *.ind
	rm -f l2r*tex frlicense.dat *.fff *.lof
	mv fig_testb.tmp fig_testb.pdf
	mv fig_testc.tmp fig_testc.pdf
	mv fig_testd.tmp fig_testd.pdf
	mv fig_teste.tmp fig_teste.pdf
	rm -rf tmp

.PHONY: clean all check
