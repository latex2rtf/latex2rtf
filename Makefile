# $Id: Makefile,v 1.50 2002/04/03 15:44:18 prahl Exp $

CC=gcc
MKDIR=mkdir -p
LIBS=

CFLAGS:=-DUNIX
#CFLAGS:=-DMSDOS
#CFLAGS:=-DMACINTOSH

#Uncomment if getopt() is not available
#CFLAGS:=$(CFLAGS) -DHAS_NO_GETOPT

#Comment out if you don't want compiler warnings
CFLAGS:=$(CFLAGS) -g -Wall -ansi -pedantic

#Base directory
PREFIX=/usr/local

# Location of binary, man, info, and support files
BIN_INSTALL=$(PREFIX)/bin
MAN_INSTALL=$(PREFIX)/man/man1
INFO_INSTALL=$(PREFIX)/info
SUPPORT_INSTALL=$(PREFIX)/share/latex2rtf
CFG_INSTALL=$(PREFIX)/share/latex2rtf/cfg

# Nothing to change below this line

VERSION="`scripts/version`"

SRCS=commands.c chars.c direct.c encode.c l2r_fonts.c funct1.c tables.c ignore.c \
	main.c stack.c cfg.c util.c parser.c lengths.c counters.c letterformat.c \
	preamble.c equation.c convert.c xref.c definitions.c graphics.c \
	mygetopt.c optind.c

HDRS=commands.h chars.h direct.h encode.h l2r_fonts.h funct1.h tables.h ignore.h \
    main.h stack.h cfg.h util.h parser.h lengths.h counters.h letterformat.h \
    preamble.h equation.h convert.h xref.h definitions.h graphics.h encode_tables.h \
    version.h

CFGS=cfg/fonts.cfg cfg/direct.cfg cfg/ignore.cfg \
    cfg/afrikaans.cfg cfg/bahasa.cfg cfg/basque.cfg cfg/brazil.cfg cfg/breton.cfg \
    cfg/catalan.cfg cfg/croatian.cfg cfg/czech.cfg cfg/danish.cfg cfg/dutch.cfg \
    cfg/english.cfg cfg/esperanto.cfg cfg/estonian.cfg cfg/finnish.cfg cfg/french.cfg \
    cfg/galician.cfg cfg/german.cfg cfg/icelandic.cfg cfg/irish.cfg cfg/italian.cfg \
    cfg/latin.cfg cfg/lsorbian.cfg cfg/magyar.cfg cfg/norsk.cfg cfg/nynorsk.cfg \
    cfg/polish.cfg cfg/portuges.cfg cfg/romanian.cfg cfg/samin.cfg cfg/scottish.cfg \
    cfg/serbian.cfg cfg/slovak.cfg cfg/slovene.cfg cfg/spanish.cfg cfg/swedish.cfg \
    cfg/turkish.cfg cfg/usorbian.cfg cfg/welsh.cfg 

DOCS= doc/latex2rtf.1 doc/latex2rtf.texi doc/latex2rtf.pdf doc/latex2rtf.txt \
	doc/latex2rtf.info doc/latex2rtf.html doc/credits doc/copying.txt doc/Makefile

README= README README.DOS README.Mac Copyright ChangeLog 

SCRIPTS= scripts/version scripts/latex2png scripts/latex2png_1 scripts/latex2png_2 \
         scripts/latex2png_3 scripts/latex2png_4

TEST=   test/Makefile test/bracecheck \
	test/accentchars.tex test/array.tex test/cite.tex test/cite.bib \
	test/eqns.tex test/fonts.tex test/fontsize.tex test/frac.tex \
	test/list.tex test/logo.tex test/misc1.tex test/misc2.tex \
	test/oddchars.tex test/tabular.tex test/percent.tex test/essential.tex test/hndout.sty \
	test/misc3.tex test/misc4.tex test/fancy.tex test/align.tex \
	test/german.tex test/box.tex \
	test/enc_applemac.tex test/enc_cp437.tex test/enc_cp865.tex test/enc_latin2.tex \
	test/enc_latin5.tex test/enc_cp1250.tex test/enc_cp850.tex test/enc_decmulti.tex  \
	test/enc_latin3.tex test/enc_latin9.tex test/enc_cp1252.tex test/enc_cp852.tex \
	test/enc_latin1.tex test/enc_latin4.tex test/enc_next.tex test/ttgfsr7.tex \
	test/defs.tex test/proffois.tex test/excalibur.tex test/qualisex.tex test/include.tex \
	test/include1.tex test/include2.tex test/include3.tex test/ch.tex test/spago1.tex \
	test/theorem.tex test/picture.tex

OBJS=l2r_fonts.o direct.o encode.o commands.o stack.o funct1.o tables.o \
	chars.o ignore.o cfg.o main.o util.o parser.o lengths.o counters.o \
	preamble.o letterformat.o equation.o convert.o xref.o definitions.o graphics.o \
	optind.o mygetopt.o

all : checkdir latex2rtf
	touch stamp-build

latex2rtf: $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(OBJS)	$(LIBS) -o latex2rtf

cfg.o: Makefile
	$(CC) $(CFLAGS) -DLIBDIR=\"$(CFG_INSTALL)\" -c cfg.c -o cfg.o

check test: latex2rtf
	cd test && $(MAKE) 

checkdir: $(README) $(SRCS) $(HDRS) $(CFGS) $(SCRIPTS) $(TEST) doc/latex2rtf.texi

clean: checkdir
	rm -f $(OBJS) core latex2rtf

depend: $(SRCS)
	$(CC) -MM $(SRCS) >makefile.depend
	@echo "***** Append makefile.depend to Makefile manually ******"

dist: $(SRCS) $(HDRS) $(CFGS) $(README) Makefile $(SCRIPTS) $(DOCS) $(TEST)
	$(MKDIR) latex2rtf-$(VERSION)
	$(MKDIR) latex2rtf-$(VERSION)/cfg
	$(MKDIR) latex2rtf-$(VERSION)/doc
	$(MKDIR) latex2rtf-$(VERSION)/doc/latex2rtf
	$(MKDIR) latex2rtf-$(VERSION)/test
	$(MKDIR) latex2rtf-$(VERSION)/scripts
	ln $(SRCS)         latex2rtf-$(VERSION)
	ln $(HDRS)         latex2rtf-$(VERSION)
	ln $(README)       latex2rtf-$(VERSION)
	ln Makefile        latex2rtf-$(VERSION)
	ln $(CFGS)         latex2rtf-$(VERSION)/cfg
	ln $(DOCS)         latex2rtf-$(VERSION)/doc
	ln $(SCRIPTS)      latex2rtf-$(VERSION)/scripts
	ln $(TEST)         latex2rtf-$(VERSION)/test
	tar cvf - latex2rtf-$(VERSION) | \
	    gzip -best > latex2rtf-$(VERSION).tar.gz
	rm -rf latex2rtf-$(VERSION)

doc: doc/latex2rtf.texi doc/Makefile
	cd doc && $(MAKE) -k

install: latex2rtf doc/latex2rtf.1 $(CFGS) scripts/latex2png
	$(MKDIR) $(BIN_INSTALL)
	$(MKDIR) $(MAN_INSTALL)
	$(MKDIR) $(CFG_INSTALL)
	cp latex2rtf          $(BIN_INSTALL)
	cp scripts/latex2png  $(BIN_INSTALL)
	cp doc/latex2rtf.1    $(MAN_INSTALL)
	cp $(CFGS)            $(CFG_INSTALL)
	cp doc/latex2rtf.html $(SUPPORT_INSTALL)
	cp doc/latex2rtf.pdf  $(SUPPORT_INSTALL)
	cp doc/latex2rtf.txt  $(SUPPORT_INSTALL)
	@echo "******************************************************************"
	@echo "*** latex2rtf successfully installed"
	@echo "***"
	@echo "*** \"make install-info\" will install TeXInfo files "
	@echo "***"
	@echo "*** latex2rtf was compiled to search for its configuration files in"
	@echo "***           \"$(CFG_INSTALL)\" "
	@echo "*** If the configuration files are moved then either"
	@echo "***   1) set the environment variable RTFPATH to this new location, or"
	@echo "***   2) use the command line option -P /path/to/cfg, or"
	@echo "***   3) edit the Makefile and recompile"
	@echo "******************************************************************"

install-info: doc/latex2rtf.info
	$(MKDIR) $(INFO_INSTALL)
	cp doc/latex2rtf.info $(BIN_INSTALL)
	install-info --info-dir=$(INFO_INSTALL) doc/latex2rtf.info

realclean: checkdir clean
	rm -f stamp-build makefile.depend latex2rtf-$(VERSION).tar.gz
	cd doc && $(MAKE) clean
	cd test && $(MAKE) clean

.PHONY: all check checkdir clean depend dist doc install install_info realclean

# created using "make depend"
commands.o : cfg.h main.h convert.h chars.h l2r_fonts.h preamble.h funct1.h \
  tables.h equation.h letterformat.h commands.h parser.h xref.h ignore.h \
  lengths.h definitions.h graphics.h 
chars.o : main.h commands.h l2r_fonts.h cfg.h ignore.h encode.h parser.h \
  chars.h funct1.h convert.h 
direct.o : main.h direct.h l2r_fonts.h cfg.h 
encode.o : main.h l2r_fonts.h funct1.h encode.h encode_tables.h 
l2r_fonts.o : main.h convert.h l2r_fonts.h funct1.h commands.h cfg.h \
  parser.h stack.h 
funct1.o : main.h convert.h funct1.h commands.h stack.h l2r_fonts.h cfg.h \
  ignore.h util.h encode.h parser.h counters.h lengths.h definitions.h \
  preamble.h 
tables.o : main.h convert.h l2r_fonts.h commands.h funct1.h tables.h \
  stack.h cfg.h parser.h counters.h util.h 
ignore.o : main.h direct.h l2r_fonts.h cfg.h ignore.h funct1.h commands.h \
  parser.h convert.h 
main.o : main.h convert.h commands.h chars.h l2r_fonts.h stack.h direct.h \
  ignore.h version.h funct1.h cfg.h encode.h util.h parser.h lengths.h \
  counters.h preamble.h xref.h 
stack.o : main.h stack.h 
cfg.o : main.h convert.h funct1.h cfg.h util.h 
util.o : main.h util.h parser.h 
parser.o : main.h commands.h cfg.h stack.h util.h parser.h l2r_fonts.h \
  lengths.h definitions.h funct1.h 
lengths.o : main.h util.h lengths.h parser.h 
counters.o : main.h util.h counters.h 
letterformat.o : main.h parser.h letterformat.h cfg.h commands.h funct1.h \
  convert.h 
preamble.o : main.h convert.h util.h preamble.h l2r_fonts.h cfg.h encode.h \
  parser.h funct1.h lengths.h ignore.h commands.h counters.h 
equation.o : main.h convert.h commands.h stack.h l2r_fonts.h cfg.h ignore.h \
  parser.h equation.h counters.h funct1.h lengths.h util.h graphics.h 
convert.o : main.h convert.h commands.h chars.h funct1.h l2r_fonts.h \
  stack.h tables.h equation.h direct.h ignore.h cfg.h encode.h util.h \
  parser.h lengths.h counters.h preamble.h 
xref.o : main.h util.h convert.h funct1.h commands.h cfg.h xref.h parser.h \
  preamble.h lengths.h l2r_fonts.h 
mygetopt.o : 
optind.o : 
definitions.o : main.h convert.h definitions.h parser.h funct1.h util.h \
  cfg.h counters.h 
graphics.o : main.h graphics.h parser.h util.h 
