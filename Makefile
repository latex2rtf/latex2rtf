# The original make file was by Ralf Schlatterbeck
# and enhanced by Georg Lehner
# The Debian-specific parts of this Makefile are created by 
# Erick Branderhorst. Parts are written by Ian Jackson and Ian Murdock.
# Recent changes by Scott Prahl

CC=gcc
COPY=cp
INSTALL=install

# Additional system libraries if needed
# LIBS=libdebug_malloc.a
LIBS=

CFLAGS:=-DUNIX
#CFLAGS:=-DMSDOS
#CFLAGS:=-DMACINTOSH

#Uncomment if MS Word uses ',' as a decimal point 
#CFLAGS:=$(CFLAGS) -DSEMICOLONSEP

#Uncomment if strdup() is not defined in string.h
#CFLAGS:=$(CFLAGS) -DHAS_NO_STRDUP

#Uncomment if getopt() is not available
#CFLAGS:=$(CFLAGS) -DHAS_NO_GETOPT

#Comment out if you don't want warnings
#CFLAGS:=$(CFLAGS) -g -Wall -ansi -pedantic

#Base directory
PREFIX=/usr/local

# Location of .cfg files needed by executable
LIBDIR=$(PREFIX)/lib/latex2rtf

# Location of executable
BININSTALL=$(PREFIX)/bin

# Location of man files
MANINSTALL=$(PREFIX)/man/man1

# Multiple .cfg locations.  Usually just $(LIBDIR), but handy for mirrored images
#LIBINSTALL=/quasi/local/lib/latex2rtf:/oberon/local/lib/latex2rtf
LIBINSTALL=$(LIBDIR)

DIR_MODE=755
BIN_MODE=755
DAT_MODE=644
DIR_USER=root
BIN_USER=root
DAT_USER=root
DIR_GROUP=wheel
BIN_GROUP=wheel
DAT_GROUP=wheel
# If you have the program install, use the following definitions
INST_DIR=$(INSTALL) -g $(DIR_GROUP) -o $(DIR_USER) -d -m $(BIN_MODE)
INST_BIN=$(INSTALL) -g $(BIN_GROUP) -o $(BIN_USER) -m $(DIR_MODE)
INST_DAT=$(INSTALL) -c -g $(DAT_GROUP) -o $(DAT_USER) -m $(DAT_MODE)
CHOWN_DIR=true
CHOWN_BIN=true
CHOWN_DAT=true
CHMOD_DIR=true
CHMOD_BIN=true
CHMOD_DAT=true
# If you do not have install, comment the definitions above and
# uncomment the following definitions
# If your mkdir does not support the -p option, you may want to create
# directories by hand and define INST_DIR=true or use the target
# simple_install instead.
# INST_BIN=$(COPY)
# INST_DIR=mkdir -p
# INST_DAT=$(COPY)
# CHOWN_DIR=chown $(DIR_USER).$(DIR_GROUP)
# CHOWN_BIN=chown $(BIN_USER).$(BIN_GROUP)
# CHOWN_DAT=chown $(DAT_USER).$(DAT_GROUP)
# CHMOD_DIR=chmod $(DIR_MODE)
# CHMOD_BIN=chmod $(DIR_MODE)
# CHMOD_DAT=chmod $(DIR_MODE)
#
# Note: If install doesn't work for you, use simple_install instead.
#

# Nothing to change below this line
SOURCES=commands.c commands.h chars.c chars.h direct.c direct.h encode.c encode.h l2r_fonts.c \
    l2r_fonts.h funct1.c funct1.h tables.c tables.h ignore.c ignore.h main.c \
    main.h stack.c stack.h version.h cfg.c cfg.h util.c util.h parser.c parser.h \
    lengths.c lengths.h counters.c counters.h letterformat.c letterformat.h \
    preamble.c preamble.h equation.c equation.h convert.c convert.h xref.c xref.h\
    Makefile README README.DOS README.Mac Copyright\
    mygetopt.c optind.c version \
    debian.README debian.control debian.rules ChangeLog l2r.bat\
    encode_tables.h definitions.c definitions.h

SUPPORT=cfg/fonts.cfg     cfg/direct.cfg   cfg/ignore.cfg \
    cfg/afrikaans.cfg cfg/bahasa.cfg cfg/basque.cfg cfg/brazil.cfg cfg/breton.cfg \
    cfg/catalan.cfg cfg/croatian.cfg cfg/czech.cfg cfg/danish.cfg cfg/dutch.cfg \
    cfg/english.cfg cfg/esperanto.cfg cfg/estonian.cfg cfg/finnish.cfg cfg/french.cfg \
    cfg/galician.cfg cfg/german.cfg cfg/icelandic.cfg cfg/irish.cfg cfg/italian.cfg \
    cfg/latin.cfg cfg/lsorbian.cfg cfg/magyar.cfg cfg/norsk.cfg cfg/nynorsk.cfg \
    cfg/polish.cfg cfg/portuges.cfg cfg/romanian.cfg cfg/samin.cfg cfg/scottish.cfg \
    cfg/serbian.cfg cfg/slovak.cfg cfg/slovene.cfg cfg/spanish.cfg cfg/swedish.cfg \
    cfg/turkish.cfg cfg/usorbian.cfg cfg/welsh.cfg 

MANUALS=latex2rtf.1

MSDOS=l2r.bat l2r.exe

DOCS= doc/latex2rtf.texi doc/latex2rtf.html doc/latex2rtf.pdf doc/latex2rtf.txt \
	  doc/latex2rtf.info doc/credits doc/copying.txt doc/Makefile

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
	test/include1.tex test/include2.tex test/include3.tex test/ch.tex

OBJS=l2r_fonts.o direct.o encode.o commands.o stack.o funct1.o tables.o \
	chars.o ignore.o cfg.o main.o util.o parser.o mygetopt.o lengths.o counters.o \
	preamble.o letterformat.o equation.o convert.o xref.o definitions.o

ARCH="`dpkg --print-architecture`"

# Some defines for versions
VERSION="`./version`"

all build stamp-build: checkdir latex2rtf doc
	touch stamp-build

latex2rtf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)	$(LIBS) -o latex2rtf

cfg.o: cfg.c cfg.h util.h
	$(CC) $(CFLAGS) -DLIBDIR=\"$(LIBDIR)\" -c cfg.c -o cfg.o

doc:	checkdir doc/latex2rtf.texi
	cd doc && $(MAKE) -k

test: latex2rtf
	cd test && $(MAKE) 

clean: checkdir
	rm -f $(OBJS) core latex2rtf.tar.gz \
	    *~ ./#* stamp-build latex2rtf-$(VERSION).tar.gz __tmp__ \
	    *.deb
	rm -rf latex2rtf latex2rtf-$(VERSION)
	rm -rf debian-tmp
	cd doc && $(MAKE) clean
	cd test && $(MAKE) clean

checkout checkdir: $(SOURCES) $(SUPPORT) $(MANUALS) $(TEST)

dist source: $(SOURCES) $(SUPPORT) $(MANUALS) $(DOCS) $(TEST)
	mkdir latex2rtf-$(VERSION)
	mkdir latex2rtf-$(VERSION)/cfg
	mkdir latex2rtf-$(VERSION)/doc
	mkdir latex2rtf-$(VERSION)/test
	ln $(SOURCES) $(MANUALS) latex2rtf-$(VERSION)
	ln $(SUPPORT) latex2rtf-$(VERSION)/cfg
	ln $(DOCS) latex2rtf-$(VERSION)/doc
	ln $(TEST) latex2rtf-$(VERSION)/test
	tar cvf - latex2rtf-$(VERSION) | \
	    gzip -best > latex2rtf-$(VERSION).tar.gz
	rm -rf latex2rtf-$(VERSION)

# deleted rm -f $$i/$$j; \ from just after for statement and changed name
install_cfg: $(SUPPORT)
	IFS=: ; for i in $(LIBINSTALL) ; do \
	    $(INST_DIR) $$i; \
	    $(CHOWN_DIR) $$i; \
	    $(CHMOD_DIR) $$i; \
	    for j in $(SUPPORT) ; do \
		$(INST_DAT) $$j $$i; \
		$(CHOWN_DAT) $$i/$$j; \
		$(CHMOD_DAT) $$i/$$j; \
	    done ;\
	done

complex_install: latex2rtf install.man install_cfg
	IFS=: ; for i in $(BININSTALL) ; do \
	    $(INST_DIR) $$i; \
	    $(CHOWN_DIR) $$i; \
	    $(CHMOD_DIR) $$i; \
	    $(INST_BIN) latex2rtf $$i; \
	    $(CHOWN_BIN) $$i/latex2rtf; \
	    $(CHMOD_BIN) $$i/latex2rtf; \
	done

# do not delete rtf2latex.1 since we don't have the original source files
install.man: $(MANUALS)
	IFS=: ; for i in $(MANINSTALL) ; do \
	    $(INST_DIR) $$i; \
	    $(CHOWN_DIR) $$i; \
	    $(CHMOD_DIR) $$i; \
	    for j in $(MANUALS) ; do \
		$(INST_DAT) $$j $$i; \
		$(CHOWN_DAT) $$i/$$j; \
		$(CHMOD_DAT) $$i/$$j; \
	    done ;\
	done

simple_cfg_install: $(SUPPORT)
	$(INST_DAT) $(SUPPORT) $(LIBINSTALL)

simple_install: $(MANUALS) latex2rtf
	-mkdir $(LIBINSTALL)
	$(CHOWN_DIR) $(LIBINSTALL)
	$(CHMOD_DIR) $(LIBINSTALL)
	$(INST_BIN) latex2rtf $(BININSTALL)
	$(CHOWN_BIN) $(BININSTALL)/latex2rtf
	$(CHMOD_BIN) $(BININSTALL)/latex2rtf
	$(INST_DAT) $(MANUALS) $(MANINSTALL)

install: complex_install

.PHONY: install complex_install simple_install simple_cfg_install \
	install.man complex_install install_and_delete_old_cfg dist \
	all clean checkout build checkdir diff checkroot binary source test

# Debian-specific targets:

checkroot:
	test root = "`whoami`"

diff:
	echo "Debian release is maintained by source maintainer. No changes" | \
	gzip -best > latex2rtf-$(VERSION).diff

binary: checkroot debian.README install install_and_delete_old_cfg
	$(INST_DIR) debian-tmp/DEBIAN
	sed -e '2s/=/'$(VERSION)'/; 3s/=/'$(ARCH)/ debian.control > __tmp__
	$(INST_DAT) __tmp__ debian-tmp/DEBIAN/control
	rm -f __tmp__
	$(INST_DIR) debian-tmp/usr/doc/copyright
	cat debian.README Copyright > __tmp__
	$(INST_DAT) __tmp__ debian-tmp/usr/doc/copyright/latex2rtf
	rm -f __tmp__
	dpkg --build debian-tmp
	mv debian-tmp.deb latex2rtf-$(VERSION).$(ARCH).deb
	rm -rf debian-tmp	

# DO NOT DELETE THIS LINE -- make depend depends on it.
cfg.o: cfg.h convert.h funct1.h main.h util.h
chars.o: cfg.h chars.h commands.h convert.h encode.h funct1.h ignore.h
chars.o: l2r_fonts.h main.h parser.h
commands.o: cfg.h chars.h commands.h convert.h definitions.h equation.h
commands.o: funct1.h ignore.h l2r_fonts.h lengths.h letterformat.h main.h
commands.o: parser.h preamble.h tables.h xref.h
convert.o: cfg.h chars.h commands.h convert.h counters.h direct.h encode.h
convert.o: equation.h funct1.h ignore.h l2r_fonts.h lengths.h main.h parser.h
convert.o: preamble.h stack.h tables.h util.h
counters.o: counters.h main.h util.h
definitions.o: convert.h definitions.h funct1.h main.h parser.h util.h
direct.o: cfg.h direct.h l2r_fonts.h main.h
encode.o: encode.h encode_tables.h funct1.h l2r_fonts.h main.h
equation.o: cfg.h commands.h convert.h counters.h equation.h funct1.h
equation.o: ignore.h l2r_fonts.h lengths.h main.h parser.h stack.h
funct1.o: cfg.h commands.h convert.h counters.h definitions.h encode.h
funct1.o: funct1.h ignore.h l2r_fonts.h lengths.h main.h parser.h preamble.h
funct1.o: stack.h util.h
ignore.o: cfg.h commands.h direct.h funct1.h ignore.h l2r_fonts.h main.h
ignore.o: parser.h
l2r_fonts.o: cfg.h commands.h convert.h funct1.h l2r_fonts.h main.h parser.h
l2r_fonts.o: stack.h
lengths.o: lengths.h main.h parser.h util.h
letterformat.o: cfg.h commands.h convert.h funct1.h letterformat.h main.h
letterformat.o: parser.h
main.o: cfg.h chars.h commands.h convert.h counters.h direct.h encode.h
main.o: funct1.h ignore.h l2r_fonts.h lengths.h main.h parser.h preamble.h
main.o: stack.h version.h xref.h
mygetopt.o: main.h
parser.o: cfg.h l2r_fonts.h lengths.h main.h parser.h stack.h util.h
preamble.o: cfg.h commands.h convert.h counters.h encode.h funct1.h ignore.h
preamble.o: l2r_fonts.h lengths.h main.h parser.h preamble.h util.h
stack.o: main.h stack.h
tables.o: cfg.h commands.h convert.h counters.h funct1.h l2r_fonts.h main.h
tables.o: parser.h stack.h tables.h util.h
util.o: main.h parser.h util.h
xref.o: cfg.h commands.h convert.h funct1.h l2r_fonts.h lengths.h main.h
xref.o: parser.h preamble.h util.h xref.h
