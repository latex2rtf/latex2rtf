# $Id: Makefile,v 1.2 2001/08/12 15:47:04 prahl Exp $
# History:
# $Log: Makefile,v $
# Revision 1.2  2001/08/12 15:47:04  prahl
# latex2rtf version 1.1 by Ralf Schlatterbeck
#
# Revision 1.9  1994/07/13  09:27:31  ralf
# Corrected fpos/SEEK_SET bug for SunOs 4.3.1 reported by Ulrich Schmid
# <schmid@dkrz.d400.de>
#
# Revision 1.8  1994/06/21  08:13:57  ralf
# Added CFLAGS
#
# Revision 1.7  1994/06/17  15:13:44  ralf
# Added README and Copyright
#
# Revision 1.6  1994/06/17  14:35:46  ralf
# Added Makefile to SOURCES
#
# Revision 1.5  1994/06/17  14:32:29  ralf
# Added latex2rtf.tar.gz to clean list.
#
# Revision 1.4  1994/06/17  14:29:42  ralf
# Added version.h to dependecy list of main.o and to SOURCES
# Added dist target
#
# Revision 1.3  1994/06/17  14:19:41  ralf
# Corrected various bugs, for example interactive read of arguments
#
# Revision 1.2  1994/06/17  12:11:57  ralf
# Added intall target and rcs rules.
#
# Revision 1.1  1994/06/17  11:30:33  ralf
# Initial revision
#
CC=gcc    # C-Compiler 
CFLAGS=-g $(XCFLAGS) # Use -O here if you want it optimized
COPY=cp
# Note: If install doesn't work for you, use simple_install instead.
#
# Where support files are searched for by the executable
LIBDIR=/usr/local/lib/latex2rtf
# You can give several Directories separated by ':' for the following
# install targets
#
# Where supportfiles are installed should normally be the same as LIBDIR
LIBINSTALL=/quasi/local/lib/latex2rtf:/oberon/local/lib/latex2rtf
#LIBINSTALL=$(LIBDIR)
# Where Binaries are installed
BININSTALL=/quasi/local/bin:/oberon/local/bin
#BININSTALL=/usr/local/bin
MANINSTALL=/usr/local/man/man1

# The following should fix compatibility problems on some machines, you
# may add the following option to XCFLAGS
# -DHAS_NO_FPOS for SunOs 4.1.3 (Thanks to Ulrich Schmid schmid@dkrz.d400.de)

XCFLAGS=
#XCFLAGS=-DHAS_NO_FPOS

# Nothing to change below this line
SOURCES=commands.c commands.h direct.c direct.h fonts.c fonts.h \
    funct1.c funct1.h funct2.c funct2.h ignore.c ignore.h main.c main.h \
    stack.c stack.h version.h Makefile README Copyright
SUPPORT=direct.cfg fonts.cfg ignore.cfg
MANUALS=latex2rtf.1

all: latex2rtf

latex2rtf: fonts.o direct.o commands.o stack.o funct1.o funct2.o \
	ignore.o main.o
	$(CC) $(CFLAGS) fonts.o direct.o commands.o stack.o funct1.o funct2.o \
	main.o ignore.o -o latex2rtf

fonts.o: fonts.c main.h fonts.h
	$(CC) $(CFLAGS) -c fonts.c -o fonts.o

direct.o: direct.c main.h direct.h fonts.h
	$(CC) $(CFLAGS) -c direct.c -o direct.o

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c -o stack.o

funct1.o: funct1.c main.h funct1.h funct2.h commands.h stack.h fonts.h
	$(CC) $(CFLAGS) -c funct1.c -o funct1.o

funct2.o: funct2.c main.h funct1.h commands.h funct2.h stack.h
	$(CC) $(CFLAGS) -c funct2.c -o funct2.o

ignore.o: ignore.c main.h direct.h fonts.h
	$(CC) $(CFLAGS) -c ignore.c -o ignore.o

main.o: main.c main.h commands.h funct1.h fonts.h stack.h funct2.h \
	direct.h ignore.h version.h
	$(CC) $(CFLAGS) -DLIBDIR=\"$(LIBDIR)\" -c main.c -o main.o

commands.o: commands.c main.h funct1.h commands.h funct2.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o

clean:
	rm -f stack.o main.o funct1.o funct2.o ignore.o commands.o \
	direct.o fonts.o core latex2rtf.tar.gz
	rm -rf latex2rtf

$(SOURCES) $(SUPPORT) $(MANUALS):
	co $@

dist: $(SOURCES) $(SUPPORT) $(MANUALS) clean
	mkdir latex2rtf
	ln $(SOURCES) $(SUPPORT) $(MANUALS) latex2rtf
	tar cvf - latex2rtf | gzip -best > latex2rtf.tar.gz
	rm -rf latex2rtf

install: $(SUPPORT) latex2rtf install.man
	IFS=: ; for i in $(LIBINSTALL) ; do \
	    mkdir -p $$i; \
	    for j in $(SUPPORT) ; do \
		rm -f $$i/$$j; \
	    done ;\
	    $(COPY) $(SUPPORT) $$i; \
	done
	IFS=: ; for i in $(BININSTALL) ; do \
	    mkdir -p $$i; \
	    $(COPY) latex2rtf $$i; \
	done

install.man: $(MANUALS)
	IFS=: ; for i in $(MANINSTALL) ; do \
	    mkdir -p $$i; \
	    for j in $(MANUALS) ; do \
		rm -f $$i/$$j; \
		$(COPY) $$j $$i; \
	    done ;\
	done

simple_install: $(MANUALS) $(SUPPORT) latex2rtf
	-mkdir $(LIBINSTALL)
	$(COPY) $(SUPPORT) $(LIBINSTALL)
	$(COPY) latex2rtf $(BININSTALL)
	$(COPY) $(MANUALS) $(MANINSTALL)
