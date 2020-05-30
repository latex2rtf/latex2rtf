CC?=gcc
TAR?=gnutar
RM?=rm -f
MKDIR?=mkdir -p
RMDIR?=rm -rf
PKGMANDIR?=man

#reasonable flags for developing
#CFLAGS = -g -D_FORTIFY_SOURCE=2 -Wall -Waggregate-return -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes -Wformat=2

PLATFORM?=-DUNIX   # Mac OS X, Linux, BSD
#PLATFORM?=-DMSDOS # Windows/DOS
#PLATFORM?=-DOS2   # OS/2 (does anyone still use this?)
#PLATFORM?=-DMSDOS -DNOSTDERR  # Windows/DOS without stderr

#Uncomment for some windows machines (neither needed for djgpp nor for MinGW)
#EXE_SUFFIX=.exe

#Base directory - adapt as needed
# Unix:
PREFIX?=/usr/local
#Uncomment next 2 lines for Windows
#DESTDIR_DRIVE=C:
#PREFIX?=$(DESTDIR_DRIVE)/PROGRA~1/latex2rtf

#Name of executable binary --- beware of 8.3 restriction under DOS
BINARY_NAME=latex2rtf$(EXE_SUFFIX)

# Location of binary, man, info, and support files - adapt as needed
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/$(PKGMANDIR)/man1
INFODIR=$(PREFIX)/share/info
SUPPORTDIR=$(PREFIX)/share/latex2rtf
CFGDIR=$(PREFIX)/share/latex2rtf/cfg

#Uncomment next 5 lines for Windows
#BINDIR=
#MANDIR=
#INFODIR=
#SUPPORTDIR=
#CFGDIR=/cfg

# Nothing to change below this line - except:
# for Windows, change "all : checkdir latex2rtf" to "all : latex2rtf"

CFLAGS:=$(CFLAGS) $(PLATFORM)

LIBS= -lm

L2R_VERSION:="latex2rtf-`grep 'Version' version.h | sed 's/[^"]*"\([^" ]*\).*/\1/'`"

SRCS=commands.c chars.c direct.c encodings.c fonts.c funct1.c tables.c ignore.c \
	main.c stack.c cfg.c utils.c parser.c lengths.c counters.c letterformat.c \
	preamble.c equations.c convert.c xrefs.c definitions.c graphics.c \
	mygetopt.c styles.c preparse.c vertical.c fields.c \
	labels.c biblio.c acronyms.c auxfile.c

HDRS=commands.h chars.h direct.h encodings.h fonts.h funct1.h tables.h ignore.h \
    main.h stack.h cfg.h utils.h parser.h lengths.h counters.h letterformat.h \
    preamble.h equations.h convert.h xrefs.h definitions.h graphics.h encoding_tables.h \
    version.h mygetopt.h styles.h preparse.h vertical.h fields.h labels.h biblio.h acronyms.h \
	auxfile.h

CFGS=cfg/fonts.cfg cfg/direct.cfg cfg/ignore.cfg cfg/style.cfg \
    cfg/afrikaans.cfg cfg/bahasa.cfg cfg/basque.cfg cfg/brazil.cfg cfg/breton.cfg \
    cfg/catalan.cfg cfg/croatian.cfg cfg/czech.cfg cfg/danish.cfg cfg/dutch.cfg \
    cfg/english.cfg cfg/esperanto.cfg cfg/estonian.cfg cfg/finnish.cfg cfg/french.cfg \
    cfg/galician.cfg cfg/german.cfg cfg/icelandic.cfg cfg/irish.cfg cfg/italian.cfg \
    cfg/latin.cfg cfg/lsorbian.cfg cfg/magyar.cfg cfg/norsk.cfg cfg/nynorsk.cfg \
    cfg/polish.cfg cfg/portuges.cfg cfg/romanian.cfg cfg/samin.cfg cfg/scottish.cfg \
    cfg/serbian.cfg cfg/slovak.cfg cfg/slovene.cfg cfg/spanish.cfg cfg/swedish.cfg \
    cfg/turkish.cfg cfg/usorbian.cfg cfg/welsh.cfg cfg/russian.cfg \
    cfg/ukrainian.cfg cfg/frenchb.cfg cfg/greek.cfg cfg/british.cfg

DOCS= doc/latex2rtf.1   doc/latex2png.1    doc/latex2rtf.texi doc/latex2rtf.pdf \
      doc/latex2rtf.txt doc/latex2rtf.info doc/latex2rtf.html doc/credits \
      doc/copying.txt   doc/Makefile       doc/latex2pn.txt  doc/latex2rt.txt

README= README README.DOS README.Mac README.OS2 README.Solaris README.VMS README.OSX \
        Copyright ChangeLog

SCRIPTS= scripts/latex2png scripts/latex2png_1 scripts/latex2png_2 \
	scripts/pdf2pnga scripts/README \
	scripts/Makefile scripts/test1.tex scripts/test2.tex scripts/test3.tex \
	scripts/test3a.tex scripts/test4.tex scripts/test1fig.eps

TEST=  \
	test/Makefile                test/enc_cp852.tex      test/fig_testd.pdf     \
	test/accentchars.tex         test/enc_cp855.tex      test/fig_testd.ps      \
	test/align.tex               test/enc_cp865.tex      test/fig_teste.pdf     \
	test/array.tex               test/enc_cp866.tex      test/fig_testf.png     \
	test/babel_czech.tex         test/enc_decmulti.tex   test/fig_tex.eps       \
	test/babel_french.tex        test/enc_koi8-r.tex     test/fonts.tex         \
	test/babel_frenchb.tex       test/enc_koi8-u.tex     test/fontsize.tex      \
	test/babel_german.tex        test/enc_latin1.tex     test/fonttest.tex      \
	test/babel_russian.tex       test/enc_latin2.tex     test/frac.tex          \
	test/babel_spanish.tex       test/enc_latin3.tex     test/geometry.tex      \
	test/bib_apacite.tex         test/enc_latin4.tex     test/geotest.tex       \
	test/bib_apacite_dblsp.tex   test/enc_latin5.tex     test/german.tex        \
	test/bib_apalike.tex         test/enc_latin9.tex     test/head_article.tex  \
	test/bib_apalike2.tex        test/enc_maccyr.tex     test/head_book.tex     \
	test/bib_apanat.tex          test/head_report.tex                           \
	test/bib_authordate.tex      test/enc_next.tex       test/hndout.sty        \
	test/bib_harvard.bib         test/enc_utf8x.tex      test/ifclause.tex      \
	test/bib_harvard.tex         test/endnote.tex        test/include.tex       \
	test/bib_natbib1.tex         test/eqnnumber.tex      test/include1.tex      \
	test/bib_natbib2.tex         test/eqnnumber2.tex     test/include2.tex      \
	test/bib_natbib3.tex         test/eqns-koi8.tex      test/include3.tex      \
	test/bib_simple.bib          test/eqns.tex           test/linux.tex         \
	test/bib_simple.tex          test/eqns2.tex          test/list.tex          \
	test/bib_super.tex           test/essential.tex      test/logo.tex          \
	test/bibentry_apalike.bib    test/excalibur.tex      test/misc1.tex         \
	test/bibentry_apalike.tex    test/fancy.tex          test/misc2.tex         \
	test/bibentry_plain.bib      test/fig_endfloat.tex   test/misc3.tex         \
	test/bibentry_plain.tex      test/fig_oval.eps       test/misc4.tex         \
	test/box.tex                 test/fig_oval.pict      test/oddchars.tex      \
	test/bracecheck              test/fig_oval.png       test/overstrike.tex    \
	test/ch.tex                  test/fig_size.tex       test/percent.tex       \
	test/chem.tex                test/fig_test.eps       test/picture.tex       \
	test/color.tex               test/fig_test.tex       test/qualisex.tex      \
	test/dblspace.tex            test/fig_test2.tex      test/report.tex        \
	test/defs.tex                test/fig_test3.tex      test/spago1.tex        \
	test/enc_applemac.tex        test/fig_test4.tex      test/subsup.tex        \
	test/enc_cp1250.tex          test/fig_testa.eps      test/tabbing.tex       \
	test/enc_cp1251.tex          test/fig_testb.pdf      test/tabular.tex       \
	test/enc_cp1252.tex          test/fig_testb.ps       test/theorem.tex       \
	test/enc_cp437.tex           test/fig_testc.pdf      test/ttgfsr7.tex       \
	test/enc_cp850.tex           test/fig_testc.ps       test/ucsymbols.tex     \
	test/bib_apa.tex             test/bib_apa.bib        test/bib_apacite2.tex  \
	test/bib_apacite2.bib        test/fig_subfig.tex     test/include4.tex      \
	test/include5.tex            test/hyperref.tex       test/bib_super.bib     \
	test/longstack.tex           test/table_array1.tex   test/table_array2.tex  \
	test/bib_apacite3.tex        test/bib_apacite3.bib   test/color2.tex        \
	test/fig_png.tex             test/fig_10x15.png      test/psfig.sty         \
	test/cyrillic.tex            test/greek.tex          test/direct.tex        \
	test/acronym.tex             test/acronym.bib        test/style.tex         \
	test/enc_moroz_koi8.tex      test/enc_moroz_ot2.tex  test/enc_moroz_utf8.tex\
	test/enc_ot2.tex             test/keywords.tex       test/bib_natbib4.tex   \
	test/graphicspath.tex        test/bib_style.tex      test/tikz.tex          \
	test/tikz2.tex
	
OBJS=fonts.o direct.o encodings.o commands.o stack.o funct1.o tables.o \
	chars.o ignore.o cfg.o main.o utils.o parser.o lengths.o counters.o \
	preamble.o letterformat.o equations.o convert.o xrefs.o definitions.o graphics.o \
	mygetopt.o styles.o preparse.o vertical.o fields.o \
	labels.o biblio.o auxfile.o	acronyms.o

all : checkdir latex2rtf    # Windows: remove "checkdir"

latex2rtf: $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS)	$(LIBS) -o $(BINARY_NAME)

cfg.o: Makefile cfg.c
	$(CC) $(CFLAGS) -DCFGDIR=\"$(CFGDIR)\" $(PLATFORM) -c $*.c

main.o: Makefile main.c
	$(CC) $(CFLAGS) -DCFGDIR=\"$(CFGDIR)\" $(PLATFORM) -c $*.c

%.o: Makefile %.c
	$(CC) $(CFLAGS) $(PLATFORM) -c $*.c

check test: latex2rtf
	cd test && $(MAKE) clean
	cd test && $(MAKE)
	cd test && $(MAKE) check
	
fullcheck: latex2rtf
	cd scripts && $(MAKE)
	cd test && $(MAKE) clean
	cd test && $(MAKE) all
	cd test && $(MAKE) check

checkdir: $(README) $(SRCS) $(HDRS) $(CFGS) $(SCRIPTS) $(TEST) doc/latex2rtf.texi Makefile vms_make.com

clean: checkdir
	-$(RM) $(OBJS) core $(BINARY_NAME)
	-$(RMDIR) tmp

depend: $(SRCS)
	$(CC) -MM $(SRCS) >makefile.depend
	@echo "***** Append makefile.depend to Makefile manually ******"

dist: checkdir releasedate latex2rtf doc $(SRCS) $(HDRS) $(CFGS) $(README) Makefile vms_make.com $(SCRIPTS) $(DOCS) $(TEST)
	$(MAKE) releasedate
	$(RMDIR) $(L2R_VERSION)
	$(MKDIR) $(L2R_VERSION)
	$(MKDIR) $(L2R_VERSION)/cfg
	$(MKDIR) $(L2R_VERSION)/doc
	$(MKDIR) $(L2R_VERSION)/test
	$(MKDIR) $(L2R_VERSION)/scripts
	ln $(SRCS)         $(L2R_VERSION)
	ln $(HDRS)         $(L2R_VERSION)
	ln $(README)       $(L2R_VERSION)
	ln Makefile        $(L2R_VERSION)
	ln vms_make.com    $(L2R_VERSION)
	ln $(CFGS)         $(L2R_VERSION)/cfg
	ln $(DOCS)         $(L2R_VERSION)/doc
	ln $(SCRIPTS)      $(L2R_VERSION)/scripts
	ln $(TEST)         $(L2R_VERSION)/test
	$(TAR) cvfz $(L2R_VERSION).tar.gz $(L2R_VERSION)
	$(RMDIR) $(L2R_VERSION)

uptodate:
	perl -pi.bak -e '$$date=scalar localtime; s/\(.*/($$date)";/' version.h
	$(RM) version.h.bak

releasedate:
	perl -pi.bak -e '$$date=scalar localtime; s/\(.*/(released $$date)";/; s/d ..../d /;s/\d\d:\d\d:\d\d //;' version.h
	$(RM) version.h.bak

doc: doc/latex2rtf.texi doc/Makefile
	cd doc && $(MAKE) -k

install: latex2rtf doc/latex2rtf.1 $(CFGS) scripts/latex2png
	cd doc && $(MAKE)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(MKDIR) $(DESTDIR)$(MANDIR)
	$(MKDIR) $(DESTDIR)$(CFGDIR)
	cp -p scripts/latex2png  $(DESTDIR)$(BINDIR)
	cp -p doc/latex2rtf.1    $(DESTDIR)$(MANDIR)
	cp -p doc/latex2png.1    $(DESTDIR)$(MANDIR)
	cp -p $(CFGS)            $(DESTDIR)$(CFGDIR)
	cp -p doc/latex2rtf.html $(DESTDIR)$(SUPPORTDIR)
	cp -p doc/latex2rtf.pdf  $(DESTDIR)$(SUPPORTDIR)
	cp -p doc/latex2rtf.txt  $(DESTDIR)$(SUPPORTDIR)
	cp -p $(BINARY_NAME)     $(DESTDIR)$(BINDIR)
	@echo "******************************************************************"
	@echo "*** latex2rtf successfully installed as \"$(BINARY_NAME)\""
	@echo "*** in directory \"$(BINDIR)\""
	@echo "***"
	@echo "*** \"make install-info\" will install TeXInfo files "
	@echo "***"
	@echo "*** latex2rtf was compiled to search for its configuration files in"
	@echo "***           \"$(CFGDIR)\" "
	@echo "***"
	@echo "*** If the configuration files are moved then either"
	@echo "***   1) set the environment variable RTFPATH to this new location, or"
	@echo "***   2) use the command line option -P /path/to/cfg, or"
	@echo "***   3) edit the Makefile and recompile"
	@echo "******************************************************************"

install-info: doc/latex2rtf.info
	$(MKDIR) $(DESTDIR)$(INFODIR)
	cp -p doc/latex2rtf.info $(DESTDIR)$(INFODIR)
	install-info --info-dir=$(DESTDIR)$(INFODIR) doc/latex2rtf.info

realclean: checkdir clean
	$(RM) makefile.depend $(L2R_VERSION).tar.gz
	cd doc && $(MAKE) clean
	cd test && $(MAKE) clean

appleclean:
	sudo xattr -r -d com.apple.FinderInfo ../trunk
	sudo xattr -r -d com.apple.TextEncoding ../trunk
	
splint:
	splint -weak $(SRCS) $(HDRS)
	
.PHONY: all check checkdir clean depend dist doc install install_info realclean latex2rtf uptodate releasedate splint fullcheck

# created using "make depend"
commands.o: commands.c cfg.h main.h convert.h chars.h fonts.h preamble.h \
  funct1.h tables.h equations.h letterformat.h commands.h parser.h \
  xrefs.h ignore.h lengths.h definitions.h graphics.h vertical.h \
  encodings.h labels.h acronyms.h biblio.h
chars.o: chars.c main.h commands.h fonts.h cfg.h ignore.h encodings.h \
  parser.h chars.h funct1.h convert.h utils.h vertical.h fields.h
direct.o: direct.c main.h direct.h fonts.h cfg.h utils.h
encodings.o: encodings.c main.h encoding_tables.h encodings.h fonts.h \
  chars.h parser.h vertical.h
fonts.o: fonts.c main.h convert.h fonts.h funct1.h commands.h cfg.h \
  parser.h stack.h vertical.h
funct1.o: funct1.c main.h convert.h funct1.h commands.h stack.h fonts.h \
  cfg.h ignore.h utils.h encodings.h parser.h counters.h lengths.h \
  definitions.h preamble.h xrefs.h equations.h direct.h styles.h \
  graphics.h vertical.h
tables.o: tables.c main.h convert.h fonts.h commands.h funct1.h tables.h \
  stack.h cfg.h parser.h counters.h utils.h lengths.h preamble.h \
  graphics.h vertical.h
ignore.o: ignore.c main.h direct.h fonts.h cfg.h ignore.h funct1.h \
  commands.h parser.h convert.h utils.h vertical.h
main.o: main.c main.h mygetopt.h convert.h commands.h chars.h fonts.h \
  stack.h direct.h ignore.h version.h funct1.h cfg.h encodings.h utils.h \
  parser.h lengths.h counters.h preamble.h xrefs.h preparse.h vertical.h \
  fields.h
stack.o: stack.c main.h stack.h fonts.h
cfg.o: cfg.c main.h convert.h funct1.h cfg.h utils.h
utils.o: utils.c cfg.h main.h utils.h parser.h
parser.o: parser.c main.h commands.h cfg.h stack.h utils.h parser.h \
  fonts.h lengths.h definitions.h funct1.h
lengths.o: lengths.c main.h utils.h lengths.h parser.h
counters.o: counters.c main.h utils.h counters.h
letterformat.o: letterformat.c main.h parser.h letterformat.h cfg.h \
  commands.h funct1.h convert.h vertical.h
preamble.o: preamble.c main.h convert.h utils.h preamble.h fonts.h cfg.h \
  encodings.h parser.h funct1.h lengths.h ignore.h commands.h counters.h \
  xrefs.h direct.h styles.h vertical.h auxfile.h acronyms.h
equations.o: equations.c main.h convert.h commands.h stack.h fonts.h \
  cfg.h ignore.h parser.h equations.h counters.h funct1.h lengths.h \
  utils.h graphics.h xrefs.h chars.h preamble.h vertical.h fields.h
convert.o: convert.c main.h convert.h commands.h chars.h funct1.h fonts.h \
  stack.h tables.h equations.h direct.h ignore.h cfg.h encodings.h \
  utils.h parser.h lengths.h counters.h preamble.h vertical.h fields.h
xrefs.o: xrefs.c main.h utils.h convert.h funct1.h commands.h cfg.h \
  xrefs.h parser.h preamble.h lengths.h fonts.h styles.h definitions.h \
  equations.h vertical.h fields.h counters.h auxfile.h labels.h \
  acronyms.h biblio.h
definitions.o: definitions.c main.h convert.h definitions.h parser.h \
  funct1.h utils.h cfg.h counters.h
graphics.o: graphics.c main.h cfg.h graphics.h parser.h utils.h \
  commands.h convert.h funct1.h preamble.h counters.h vertical.h
mygetopt.o: mygetopt.c main.h mygetopt.h
styles.o: styles.c main.h direct.h fonts.h cfg.h utils.h parser.h \
  styles.h vertical.h
preparse.o: preparse.c preparse.h cfg.h main.h utils.h definitions.h \
  parser.h funct1.h
vertical.o: vertical.c main.h funct1.h cfg.h utils.h parser.h lengths.h \
  vertical.h convert.h commands.h styles.h fonts.h stack.h xrefs.h \
  counters.h fields.h acronyms.h
fields.o: fields.c main.h fields.h
labels.o: labels.c main.h parser.h utils.h auxfile.h labels.h
biblio.o: biblio.c main.h utils.h parser.h auxfile.h biblio.h
acronyms.o: acronyms.c main.h parser.h utils.h cfg.h convert.h commands.h \
  lengths.h vertical.h auxfile.h acronyms.h biblio.h labels.h
auxfile.o: auxfile.c main.h utils.h parser.h convert.h
