# The original make file was by Ralf Schlatterbeck
# and enhanced by Georg Lehner
# The Debian-specific parts of this Makefile are created by 
# Erick Branderhorst. Parts are written by Ian Jackson and Ian Murdock.
# Recent changes by Scott Prahl

CC=gcc    # C-Compiler 
#CFLAGS=-g -Wall -ansi -pedantic $(XCFLAGS)
#CFLAGS=$(XCFLAGS) # Use -O here if you want it optimized
CFLAGS=
COPY=cp
INSTALL=install
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
# Where support files are searched for by the executable
# prefix defaults to /usr/local, but may be set on the command line
prefix=/usr/local
LIBDIR=$(prefix)/lib/latex2rtf
# You can give several Directories separated by ':' for the following
# install targets
#
# Where supportfiles are installed should normally be the same as LIBDIR
# If you specify SEVERAL directories here, the files will get installed
# into EVERY directory. This is rather useful if you have mirrored images
# that you want to update, but is normally not necessary for a normal
# installation.
#LIBINSTALL=/quasi/local/lib/latex2rtf:/oberon/local/lib/latex2rtf
LIBINSTALL=$(LIBDIR)
# Where Binaries are installed
#BININSTALL=/quasi/local/bin:/oberon/local/bin
BININSTALL=$(prefix)/bin
MANINSTALL=$(prefix)/man/man1

# It seems that MS Word allows different forms for the separator
# used in the formula commands.  For US systems the default is
# for FORMULASEP should be ',' in Germany and in perhaps other
# European systems, it should be ';' 

# The following should fix compatibility problems on some machines, you
# may add the following option to XCFLAGS
# -DHAS_NO_FPOS for SunOs 4.1.3 (Thanks to Ulrich Schmid schmid@dkrz.d400.de)
#
# If you are using MSDOS, the environment separator ENVSEP shoud be
# ';' and PATHSEP '\'.
# If not specified it defaults to ':' and '/' (UNIX standard)
#
# It seems that MS Word allows different forms for the separator
# used in the formula commands.  For US systems the default is
# The default for FORMULASEP is ',' 
# In Germany at least SEMICOLONSEP should be defined so that 
# FORMULASEP becomes ';'

# If your target/system has no getopt() function, use Vladimir Menkov's
# instead, found in mygetopt.c.
# Add -DHAS_NO_FPOS

# If your system does not have strdup() defined in string.h
# then you should add -DHAS_NO_STRDUP

XCFLAGS=
#XCFLAGS=-DENVSEP="';'"
#XCFLAGS=-DSEMICOLONSEP
#XCFLAGS=-DHAS_NO_FPOS
#XCFLAGS=-DHAS_NO_GETOPT
#XCFLAGS=-DHAS_NO_STRDUP

# Sometimes additional system libraries are needed, they can be defined
# here
# LIBS=libdebug_malloc.a
LIBS=

# Nothing to change below this line
SOURCES=commands.c commands.h chars.c chars.h direct.c direct.h encode.c encode.h l2r_fonts.c \
    l2r_fonts.h funct1.c funct1.h tables.c tables.h ignore.c ignore.h main.c \
    main.h stack.c stack.h version.h cfg.c cfg.h util.c util.h parser.c parser.h \
    lengths.c lengths.h counters.c counters.h letterformat.c letterformat.h \
    preamble.c preamble.h equation.c equation.h convert.c convert.h biblio.c biblio.h\
    Makefile README README.DOS README.Mac Copyright\
    mygetopt.c optind.c version \
    debian.README debian.control debian.rules ChangeLog l2r.bat

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
DOCS= doc/l2r.html doc/credits doc/copying.txt doc/Makefile
TEST=   test/Makefile \
	test/accentchars.tex test/array.tex test/cite.tex test/cite.bib \
	test/eqns.tex test/fonts.tex test/fontsize.tex test/frac.tex \
	test/list.tex test/logo.tex test/misc1.tex test/misc2.tex \
	test/oddchars.tex test/tabular.tex test/percent.tex test/essential.tex test/hndout.sty \
	test/misc3.tex test/misc4.tex test/fancy.tex test/align.tex \
	test/german.tex test/box.tex

OBJS=l2r_fonts.o direct.o encode.o commands.o stack.o funct1.o tables.o \
	chars.o ignore.o cfg.o main.o util.o parser.o mygetopt.o lengths.o counters.o \
	preamble.o letterformat.o equation.o convert.o biblio.o

ARCH="`dpkg --print-architecture`"

# Some defines for versions
VERSION="`./version`"

all build stamp-build: checkdir latex2rtf
	touch stamp-build

latex2rtf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)	$(LIBS) -o latex2rtf

l2r_fonts.o: l2r_fonts.c main.h l2r_fonts.h cfg.h
	$(CC) $(CFLAGS) -c l2r_fonts.c -o l2r_fonts.o

direct.o: direct.c main.h direct.h l2r_fonts.h cfg.h
	$(CC) $(CFLAGS) -c direct.c -o direct.o

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c -o stack.o

funct1.o: funct1.c main.h funct1.h tables.h commands.h stack.h l2r_fonts.h cfg.h ignore.h util.h encode.h
	$(CC) $(CFLAGS) -c funct1.c -o funct1.o

ignore.o: ignore.c main.h direct.h l2r_fonts.h cfg.h ignore.h util.h
	$(CC) $(CFLAGS) -c ignore.c -o ignore.o

encode.o: encode.c encode.h main.h funct1.h l2r_fonts.h
	$(CC) $(CFLAGS) -c encode.c -o encode.o

cfg.o: cfg.c cfg.h util.h
	$(CC) $(CFLAGS) -DLIBDIR=\"$(LIBDIR)\" -c cfg.c -o cfg.o

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -o util.o

main.o: main.c main.h commands.h funct1.h l2r_fonts.h stack.h \
	direct.h ignore.h version.h cfg.h encode.h util.h
	$(CC) $(CFLAGS) -c main.c -o main.o

commands.o: commands.c main.h funct1.h commands.h 
	$(CC) $(CFLAGS) -c commands.c -o commands.o

parser.o: parser.c parser.h main.h
	$(CC) $(CFLAGS) -c parser.c  -o parser.o

mygetopt.o: mygetopt.c main.h
	$(CC) $(CFLAGS) -c mygetopt.c  -o mygetopt.o

change.log: ChangeLog
	cp ChangeLog change.log

doc:	checkdir change.log
	cd doc && $(MAKE) -k

test: latex2rtf
	cd test && $(MAKE) 

clean: checkdir
	rm -f $(OBJS) core latex2rtf.tar.gz \
	    *~ ./#* stamp-build latex2rtf-$(VERSION).tar.gz __tmp__ \
	    *.deb
	rm -rf latex2rtf latex2rtf-$(VERSION)
	rm -rf debian-tmp
	cd doc && $(MAKE) almostclean
	cd test && $(MAKE) clean

#$(SOURCES) $(SUPPORT) $(MANUALS):
#	co $@

checkout checkdir: $(SOURCES) $(SUPPORT) $(MANUALS) $(TEST)

dist source: $(SOURCES) $(SUPPORT) $(MANUALS) $(DOCS) $(TEST) clean
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
