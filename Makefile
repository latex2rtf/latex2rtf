# $Id: Makefile,v 1.113 2005/01/30 02:14:14 prahl Exp $

CC=gcc
MKDIR=mkdir -p
TAR=gnutar
RM=rm

CFLAGS:=-DUNIX
#CFLAGS:=-DMSDOS         #Windows/DOS
#CFLAGS:=-DMAC_CLASSIC   #MacOS 8/9
#CFLAGS:=-DOS2           #OS/2

#Uncomment for some windows machines (not needed for djgpp)
#EXE_SUFFIX=.exe

#Uncomment next line for windows machines
#PREFIX_DRIVE=c:

#Uncomment next line when using gcc compiler, target linux
LINK_FLAGS = -lm

#Uncomment next line when using rsx compiler, target win32
#CFLAGS:=$(CFLAGS) -Zwin32  

#Base directory - adapt as needed
PREFIX=$(PREFIX_DRIVE)/usr/local

#Name of executable binary --- beware of 8.3 restriction under DOS
BINARY_NAME=latex2rtf$(EXE_SUFFIX)

# Location of binary, man, info, and support files - adapt as needed
BIN_INSTALL=$(PREFIX)/bin
MAN_INSTALL=$(PREFIX)/man/man1
INFO_INSTALL=$(PREFIX)/info
SUPPORT_INSTALL=$(PREFIX)/share/latex2rtf
CFG_INSTALL=$(PREFIX)/share/latex2rtf/cfg

# Nothing to change below this line

CFLAGS:=$(CFLAGS) -g -Wall -fsigned-char

LIBS=
#LIBS=-lMallocDebug -force_flat_namespace

VERSION:="`scripts/version`"

# Locations for MacOS X packaging
PWD := $(shell pwd)
PKG_DIR := "$(PWD)/macosx/pkgdir"
PKG_CONTENTS="$(PKG_DIR)/Package_contents"
PKG_RESOURCES="$(PKG_DIR)/Install_resources"
PKG_NAME:="$(PWD)/macosx/dmg/latex2rtf-$(VERSION)/latex2rtf.pkg"
PKG_MAKER=/Developer/Applications/PackageMaker.app/Contents/MacOS/PackageMaker
DMG_DIR := "$(PWD)/macosx/dmg/latex2rtf-$(VERSION)"

SRCS=commands.c chars.c direct.c encodings.c fonts.c funct1.c tables.c ignore.c \
	main.c stack.c cfg.c utils.c parser.c lengths.c counters.c letterformat.c \
	preamble.c equations.c convert.c xrefs.c definitions.c graphics.c \
	mygetopt.c styles.c

HDRS=commands.h chars.h direct.h encodings.h fonts.h funct1.h tables.h ignore.h \
    main.h stack.h cfg.h utils.h parser.h lengths.h counters.h letterformat.h \
    preamble.h equations.h convert.h xrefs.h definitions.h graphics.h encoding_tables.h \
    version.h mygetopt.h styles.h

CFGS=cfg/fonts.cfg cfg/direct.cfg cfg/ignore.cfg cfg/style.cfg \
    cfg/afrikaans.cfg cfg/bahasa.cfg cfg/basque.cfg cfg/brazil.cfg cfg/breton.cfg \
    cfg/catalan.cfg cfg/croatian.cfg cfg/czech.cfg cfg/danish.cfg cfg/dutch.cfg \
    cfg/english.cfg cfg/esperanto.cfg cfg/estonian.cfg cfg/finnish.cfg cfg/french.cfg \
    cfg/galician.cfg cfg/german.cfg cfg/icelandic.cfg cfg/irish.cfg cfg/italian.cfg \
    cfg/latin.cfg cfg/lsorbian.cfg cfg/magyar.cfg cfg/norsk.cfg cfg/nynorsk.cfg \
    cfg/polish.cfg cfg/portuges.cfg cfg/romanian.cfg cfg/samin.cfg cfg/scottish.cfg \
    cfg/serbian.cfg cfg/slovak.cfg cfg/slovene.cfg cfg/spanish.cfg cfg/swedish.cfg \
    cfg/turkish.cfg cfg/usorbian.cfg cfg/welsh.cfg cfg/russian.cfg cfg/inc_test.tex \
    cfg/ukrainian.cfg

DOCS= doc/latex2rtf.1   doc/latex2png.1    doc/latex2rtf.texi doc/latex2rtf.pdf \
      doc/latex2rtf.txt doc/latex2rtf.info doc/latex2rtf.html doc/credits \
      doc/copying.txt   doc/Makefile       doc/latex2pn.txt  doc/latex2rt.txt

README= README README.DOS README.Mac README.OS2 README.Solaris README.VMS README.OSX \
        Copyright ChangeLog

SCRIPTS= scripts/version scripts/latex2png scripts/latex2png_1 scripts/latex2png_2 \
	scripts/latex2pn.bat scripts/README \
	scripts/Makefile scripts/test1.tex scripts/test2.tex scripts/test3.tex \
	scripts/test3a.tex scripts/test4.tex scripts/test1fig.eps

TEST=   test/Makefile test/bracecheck test/accentchars.tex test/array.tex  \
	test/eqns.tex test/fonts.tex test/fontsize.tex test/frac.tex \
	test/list.tex test/logo.tex test/misc1.tex test/misc2.tex \
	test/oddchars.tex test/tabular.tex test/percent.tex test/essential.tex test/hndout.sty \
	test/misc3.tex test/misc4.tex test/fancy.tex test/align.tex \
	test/box.tex test/ttgfsr7.tex \
	test/defs.tex test/excalibur.tex test/qualisex.tex test/include.tex \
	test/include1.tex test/include2.tex test/include3.tex test/ch.tex test/spago1.tex \
	test/theorem.tex test/picture.tex test/eqns-koi8.tex test/tabbing.tex \
	test/chem.tex test/linux.tex test/color.tex test/subsup.tex \
	test/babel_german.tex  test/babel_russian.tex test/babel_french.tex \
	test/babel_frenchb.tex test/babel_czech.tex test/babel_spanish.tex \
	test/bib_apacite.tex    test/bib_apalike.tex test/bib_apalike2.tex \
	test/bib_natbib1.tex    test/bib_natbib2.tex test/bib_natbib3.tex test/bib_apanat.tex \
	test/bib_authordate.tex test/bib_simple.tex test/bib_simple.bib\
	test/enc_applemac.tex test/enc_cp437.tex test/enc_cp865.tex test/enc_latin2.tex \
	test/enc_latin5.tex test/enc_cp1250.tex test/enc_cp850.tex test/enc_decmulti.tex  \
	test/enc_latin3.tex test/enc_latin9.tex test/enc_cp1252.tex test/enc_cp852.tex \
	test/enc_latin1.tex test/enc_latin4.tex test/enc_next.tex  \
	test/enc_cp1251.tex test/enc_cp855.tex  test/enc_cp866.tex  test/enc_koi8-r.tex \
	test/enc_koi8-u.tex test/enc_maccyr.tex test/enc_macukr.tex \
	test/fig_test.eps test/fig_testb.pdf test/fig_test.tex \
	test/fig_test2.tex test/fig_testc.ps test/fig_testc.pdf test/fig_testd.ps \
	test/fig_testd.pdf test/fig_test3.tex test/fig_size.tex \
	test/head_book.tex test/head_report.tex test/head_article.tex \
	test/endnote.tex   test/bib_harvard.tex test/report.tex \
	test/bibentry_apalike.tex test/bibentry_apalike.bib \
	test/bibentry_plain.tex   test/bibentry_plain.bib \
	test/bib_apacite_dblsp.tex test/dblspace.tex test/geotest.tex\
	test/eqns2.tex             test/ifclause.tex test/enc_utf8x.tex\
	test/geometry.tex          test/unicode.tex  test/fonttest.tex\
	test/german.tex            test/bib_harvard.bib test/bib_super.tex\
	test/fig_endfloat.tex      test/fig_test4.tex   test/overstrike.tex \
	test/eqnnumber.tex         test/eqnnumber2.tex
	
	
OBJS=fonts.o direct.o encodings.o commands.o stack.o funct1.o tables.o \
	chars.o ignore.o cfg.o main.o utils.o parser.o lengths.o counters.o \
	preamble.o letterformat.o equations.o convert.o xrefs.o definitions.o graphics.o \
	mygetopt.o styles.o

all : checkdir uptodate latex2rtf

latex2rtf: $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(LINK_FLAGS) $(OBJS)	$(LIBS) -o $(BINARY_NAME)

cfg.o: Makefile cfg.c
	$(CC) $(CFLAGS) -DCFGDIR=\"$(CFG_INSTALL)\" -c cfg.c -o cfg.o

main.o: Makefile main.c
	$(CC) $(CFLAGS) -DCFGDIR=\"$(CFG_INSTALL)\" -c main.c -o main.o

check test: latex2rtf
	cd scripts && $(MAKE)
	cd test && $(MAKE) 
	cd test && $(MAKE) check
	
fullcheck: latex2rtf
	cd scripts && $(MAKE)
	cd test && $(MAKE) clean
	cd test && $(MAKE) all
	cd test && $(MAKE) check

checkdir: $(README) $(SRCS) $(HDRS) $(CFGS) $(SCRIPTS) $(TEST) doc/latex2rtf.texi Makefile vms_make.com

clean: checkdir
	$(RM) -f $(OBJS) core $(BINARY_NAME)

depend: $(SRCS)
	$(CC) -MM $(SRCS) >makefile.depend
	@echo "***** Append makefile.depend to Makefile manually ******"

dist: checkdir releasedate latex2rtf doc $(SRCS) $(HDRS) $(CFGS) $(README) Makefile vms_make.com $(SCRIPTS) $(DOCS) $(TEST)
	$(MAKE) releasedate
	$(MKDIR) latex2rtf-$(VERSION)
	$(MKDIR) latex2rtf-$(VERSION)/cfg
	$(MKDIR) latex2rtf-$(VERSION)/doc
	$(MKDIR) latex2rtf-$(VERSION)/test
	$(MKDIR) latex2rtf-$(VERSION)/scripts
	ln $(SRCS)         latex2rtf-$(VERSION)
	ln $(HDRS)         latex2rtf-$(VERSION)
	ln $(README)       latex2rtf-$(VERSION)
	ln Makefile        latex2rtf-$(VERSION)
	ln vms_make.com    latex2rtf-$(VERSION)
	ln $(CFGS)         latex2rtf-$(VERSION)/cfg
	ln $(DOCS)         latex2rtf-$(VERSION)/doc
	ln $(SCRIPTS)      latex2rtf-$(VERSION)/scripts
	ln $(TEST)         latex2rtf-$(VERSION)/test
	$(TAR) cvf - latex2rtf-$(VERSION) | \
	    gzip > latex2rtf-$(VERSION).tar.gz
	$(RM) -rf latex2rtf-$(VERSION)

uptodate:
#	perl -pi.bak -e '$$date=scalar localtime; s/\(.*/($$date)";/' version.h
#	$(RM) version.h.bak

releasedate:
	perl -pi.bak -e '$$date=scalar localtime; s/\(.*/(released $$date)";/; s/d ..../d /;s/\d\d:\d\d:\d\d //;' version.h
	$(RM) version.h.bak

doc: doc/latex2rtf.texi doc/Makefile
	cd doc && $(MAKE) -k

install: latex2rtf doc/latex2rtf.1 $(CFGS) scripts/latex2png
	cd doc && $(MAKE)
	$(MKDIR) $(BIN_INSTALL)
	$(MKDIR) $(MAN_INSTALL)
	$(MKDIR) $(CFG_INSTALL)
	cp $(BINARY_NAME)     $(BIN_INSTALL)
	cp scripts/latex2png  $(BIN_INSTALL)
	cp doc/latex2rtf.1    $(MAN_INSTALL)
	cp doc/latex2png.1    $(MAN_INSTALL)
	cp $(CFGS)            $(CFG_INSTALL)
	cp doc/latex2rtf.html $(SUPPORT_INSTALL)
	cp doc/latex2rtf.pdf  $(SUPPORT_INSTALL)
	cp doc/latex2rtf.txt  $(SUPPORT_INSTALL)
	@echo "******************************************************************"
	@echo "*** latex2rtf successfully installed as \"$(BINARY_NAME)\""
	@echo "*** in directory \"$(BIN_INSTALL)\""
	@echo "***"
	@echo "*** \"make install-info\" will install TeXInfo files "
	@echo "***"
	@echo "*** latex2rtf was compiled to search for its configuration files in"
	@echo "***           \"$(CFG_INSTALL)\" "
	@echo "***"
	@echo "*** If the configuration files are moved then either"
	@echo "***   1) set the environment variable RTFPATH to this new location, or"
	@echo "***   2) use the command line option -P /path/to/cfg, or"
	@echo "***   3) edit the Makefile and recompile"
	@echo "******************************************************************"

install-info: doc/latex2rtf.info
	$(MKDIR) $(INFO_INSTALL)
	cp doc/latex2rtf.info $(INFO_INSTALL)
	install-info --info-dir=$(INFO_INSTALL) doc/latex2rtf.info

realclean: checkdir clean
	$(RM) -f makefile.depend latex2rtf-$(VERSION).tar.gz
	cd doc && $(MAKE) clean
	cd test && $(MAKE) clean

splint: 
	splint -weak $(SRCS) $(HDRS)

pkg:
	$(MKDIR) $(PKG_CONTENTS)/$(BIN_INSTALL)
	$(MKDIR) $(PKG_CONTENTS)/$(MAN_INSTALL)
	$(MKDIR) $(PKG_CONTENTS)/$(CFG_INSTALL)
	$(MKDIR) $(PKG_CONTENTS)/$(SUPPORT_INSTALL)
	$(MKDIR) $(PKG_RESOURCES)
	$(MKDIR) $(DMG_DIR)

	cp $(BINARY_NAME)     $(PKG_CONTENTS)/$(BIN_INSTALL)
	cp scripts/latex2png  $(PKG_CONTENTS)/$(BIN_INSTALL)
	cp doc/latex2rtf.1    $(PKG_CONTENTS)/$(MAN_INSTALL)
	cp doc/latex2png.1    $(PKG_CONTENTS)/$(MAN_INSTALL)
	cp $(CFGS)            $(PKG_CONTENTS)/$(CFG_INSTALL)
	cp doc/latex2rtf.html $(PKG_CONTENTS)/$(SUPPORT_INSTALL)
	cp doc/latex2rtf.pdf  $(PKG_CONTENTS)/$(SUPPORT_INSTALL)
	cp doc/latex2rtf.txt  $(PKG_CONTENTS)/$(SUPPORT_INSTALL)

	cp macosx/License.rtf $(PKG_RESOURCES)
	cp macosx/ReadMe.html $(PKG_RESOURCES)
	cp macosx/Welcome.html $(PKG_RESOURCES)
	
	-$(PKG_MAKER) -build -p $(PKG_NAME) -r $(PKG_RESOURCES) -f $(PKG_CONTENTS)
	mkdmg $(DMG_DIR)
	
	$(RM) -rf $(PKG_DIR)
	
	mkdmg 
	
	
	
.PHONY: all check checkdir clean depend dist doc install install_info realclean latex2rtf uptodate splint fullcheck

# created using "make depend"
commands.o: commands.c cfg.h main.h convert.h chars.h fonts.h preamble.h \
  funct1.h tables.h equations.h letterformat.h commands.h parser.h \
  xrefs.h ignore.h lengths.h definitions.h graphics.h
chars.o: chars.c main.h commands.h fonts.h cfg.h ignore.h encodings.h \
  parser.h chars.h funct1.h convert.h
direct.o: direct.c main.h direct.h fonts.h cfg.h utils.h
encodings.o: encodings.c main.h fonts.h funct1.h encodings.h \
  encoding_tables.h chars.h
fonts.o: fonts.c main.h convert.h fonts.h funct1.h commands.h cfg.h \
  parser.h stack.h
funct1.o: funct1.c main.h convert.h funct1.h commands.h stack.h fonts.h \
  cfg.h ignore.h utils.h encodings.h parser.h counters.h lengths.h \
  definitions.h preamble.h xrefs.h equations.h direct.h styles.h
tables.o: tables.c main.h convert.h fonts.h commands.h funct1.h tables.h \
  stack.h cfg.h parser.h counters.h utils.h lengths.h preamble.h
ignore.o: ignore.c main.h direct.h fonts.h cfg.h ignore.h funct1.h \
  commands.h parser.h convert.h
main.o: main.c main.h mygetopt.h convert.h commands.h chars.h fonts.h \
  stack.h direct.h ignore.h version.h funct1.h cfg.h encodings.h utils.h \
  parser.h lengths.h counters.h preamble.h xrefs.h
stack.o: stack.c main.h stack.h
cfg.o: cfg.c main.h convert.h funct1.h cfg.h utils.h
utils.o: utils.c main.h utils.h parser.h
parser.o: parser.c main.h commands.h cfg.h stack.h utils.h parser.h \
  fonts.h lengths.h definitions.h funct1.h
lengths.o: lengths.c main.h utils.h lengths.h parser.h
counters.o: counters.c main.h counters.h
letterformat.o: letterformat.c main.h parser.h letterformat.h cfg.h \
  commands.h funct1.h convert.h
preamble.o: preamble.c main.h convert.h utils.h preamble.h fonts.h cfg.h \
  encodings.h parser.h funct1.h lengths.h ignore.h commands.h counters.h \
  xrefs.h direct.h styles.h
equations.o: equations.c main.h convert.h commands.h stack.h fonts.h \
  cfg.h ignore.h parser.h equations.h counters.h funct1.h lengths.h \
  utils.h graphics.h xrefs.h chars.h preamble.h
convert.o: convert.c main.h convert.h commands.h chars.h funct1.h fonts.h \
  stack.h tables.h equations.h direct.h ignore.h cfg.h encodings.h \
  utils.h parser.h lengths.h counters.h preamble.h
xrefs.o: xrefs.c main.h utils.h convert.h funct1.h commands.h cfg.h \
  xrefs.h parser.h preamble.h lengths.h fonts.h styles.h definitions.h \
  equations.h
definitions.o: definitions.c main.h convert.h definitions.h parser.h \
  funct1.h utils.h cfg.h counters.h
graphics.o: graphics.c cfg.h main.h graphics.h parser.h utils.h \
  commands.h convert.h equations.h funct1.h preamble.h
mygetopt.o: mygetopt.c main.h mygetopt.h
styles.o: styles.c main.h direct.h fonts.h cfg.h utils.h parser.h
