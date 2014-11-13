# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.


# The package path prefix, if you want to install to another root, set DESTDIR to that root
PREFIX ?= /usr
# The library path excluding prefix
LIB ?= /lib
# The resource path excluding prefix
DATA ?= /share
# The library header path excluding prefix
INCLUDE ?= /include
# The library path including prefix
LIBDIR ?= $(PREFIX)$(LIB)
# The resource path including prefix
DATADIR ?= $(PREFIX)$(DATA)
# The library header path including prefix
INCLUDEDIR ?= $(PREFIX)$(INCLUDE)
# The generic documentation path including prefix
DOCDIR ?= $(DATADIR)/doc
# The info manual documentation path including prefix
INFODIR ?= $(DATADIR)/info
# The license base path including prefix
LICENSEDIR ?= $(DATADIR)/licenses

# The name of the package as it should be installed
PKGNAME ?= libpassphrase

# Options with which to compile the library
OPTIONS ?= 
# PASSPHRASE_ECHO:      Do not hide the passphrase
# PASSPHRASE_STAR:      Use '*' for each character instead of no echo
# PASSPHRASE_REALLOC:   Soften security by using `realloc`
# PASSPHRASE_MOVE:      Enable move of point
# PASSPHRASE_INSERT:    Enable insert mode
# PASSPHRASE_OVERRIDE:  Enable override mode
# PASSPHRASE_DELETE:    Enable reversed erase command
# PASSPHRASE_CONTROL:   Enable use of control key combinations
# PASSPHRASE_DEDICATED: Enable use of dedicated keys
# DEFAULT_INSERT:       Use insert mode as default
# PASSPHRASE_INVALID:   Prevent duplication of non-initialised memory


# Optimisation settings for C code compilation
OPTIMISE ?= -Os
# Warnings settings for C code compilation
WARN = -Wall -Wextra -pedantic -Wdouble-promotion -Wformat=2 -Winit-self -Wmissing-include-dirs  \
       -Wtrampolines -Wfloat-equal -Wshadow -Wmissing-prototypes -Wmissing-declarations          \
       -Wredundant-decls -Wnested-externs -Winline -Wno-variadic-macros -Wsync-nand              \
       -Wunsafe-loop-optimizations -Wcast-align -Wstrict-overflow -Wdeclaration-after-statement  \
       -Wundef -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wlogical-op -Waggregate-return   \
       -Wstrict-prototypes -Wold-style-definition -Wpacked -Wvector-operation-performance        \
       -Wunsuffixed-float-constants -Wsuggest-attribute=const -Wsuggest-attribute=noreturn       \
       -Wsuggest-attribute=pure -Wsuggest-attribute=format -Wnormalized=nfkc -Wconversion        \
       -fstrict-aliasing -fstrict-overflow -fipa-pure-const -ftree-vrp -fstack-usage             \
       -funsafe-loop-optimizations

# The C standard for C code compilation
STD = gnu99
# C preprocessor flags
CPPFLAGS_ = $(foreach D, $(OPTIONS), -D'$(D)=1')
# C compiling flags
CFLAGS_ = -std=$(STD) $(WARN)
# Linking flags
LDFLAGS_ = 

# Flags to use when compiling and assembling
CC_FLAGS = $(CPPFLAGS_) $(CFLAGS_) $(OPTIMISE)
# Flags to use when linking
LD_FLAGS = $(LDFLAGS_) $(CFLAGS_) $(OPTIMISE)



.PHONY: default
default: lib info

.PHONY: all
all: lib test doc

.PHONY: doc
doc: info pdf ps dvi

.PHONY: lib
lib: libpassphrase

.PHONY: libpassphrase
libpassphrase: so a

.PHONY: so
so: bin/libpassphrase.so

.PHONY: a
a: bin/libpassphrase.a

.PHONY: test
test: bin/test

bin/libpassphrase.so: obj/passphrase.o
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -shared -Wl,-soname,libpassphrase.so -o "$@" $^ $(LDFLAGS)

bin/libpassphrase.a: obj/passphrase.o
	@mkdir -p bin
	ar rcs "$@" $^

bin/test: bin/libpassphrase.so obj/test.o
	$(CC) $(LD_FLAGS) -Lbin -lpassphrase -o "$@" obj/test.o $(LDFLAGS)

obj/passphrase.o: src/passphrase.c src/passphrase.h
	@mkdir -p "$(shell dirname "$@")"
	$(CC) $(CC_FLAGS) -fPIC -o "$@" -c "$<" $(CFLAGS) $(CPPFLAGS)

obj/test.o: src/test.c src/test.h
	@mkdir -p "$(shell dirname "$@")"
	$(CC) $(CC_FLAGS) -o "$@" -c "$<" $(CFLAGS) $(CPPFLAGS)

.PHONY: info
info: libpassphrase.info
%.info: info/%.texinfo
	makeinfo "$<"

.PHONY: pdf
pdf: libpassphrase.pdf
%.pdf: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | texi2pdf ../$<
	mv obj/$@ $@

.PHONY: dvi
dvi: libpassphrase.dvi
%.dvi: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | $(TEXI2DVI) ../$<
	mv obj/$@ $@

.PHONY: ps
ps: libpassphrase.ps
%.ps: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | texi2pdf --ps ../$<
	mv obj/$@ $@


.PHONY: install
install: install-base install-info

.PHONY: install
install-all: install-base install-doc

.PHONY: install-base
install-base: install-so install-a install-header install-license

.PHONY: install-lib
install-lib: install-so install-a install-header

.PHONY: install-so
install-so: bin/libpassphrase.so
	install -dm755 -- "$(DESTDIR)$(LIBDIR)"
	install  -m755 -- bin/libpassphrase.so "$(DESTDIR)$(LIBDIR)"

.PHONY: install-a
install-a: bin/libpassphrase.a
	install -dm755 -- "$(DESTDIR)$(LIBDIR)"
	install  -m644 -- bin/libpassphrase.a "$(DESTDIR)$(LIBDIR)"

.PHONY: install-header
install-header:
	install -dm755 -- "$(DESTDIR)$(INCLUDEDIR)"
	install  -m755 -- src/passphrase.h "$(DESTDIR)$(INCLUDEDIR)"

.PHONY: install-license
install-license:
	install -dm755 -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"
	install  -m644 -- COPYING LICENSE "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"

.PHONY: install-doc
install-doc: install-info install-pdf install-ps install-dvi

.PHONY: install-info
install-info: libpassphrase.info
	install -dm755 -- "$(DESTDIR)$(INFODIR)"
	install  -m644 -- "$<" "$(DESTDIR)$(INFODIR)/$(PKGNAME).info"

.PHONY: install-pdf
install-pdf: libpassphrase.pdf
	install -dm755 -- "$(DESTDIR)$(DOCDIR)"
	install  -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).pdf"

.PHONY: install-ps
install-ps: libpassphrase.ps
	install -dm755 -- "$(DESTDIR)$(DOCDIR)"
	install  -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).ps"

.PHONY: install-dvi
install-dvi: libpassphrase.dvi
	install -dm755 -- "$(DESTDIR)$(DOCDIR)"
	install  -m644 -- "$<" "$(DESTDIR)$(DOCDIR)/$(PKGNAME).dvi"


.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(LIBDIR)/libpassphrase.so"
	-rm -- "$(DESTDIR)$(LIBDIR)/libpassphrase.a"
	-rm -- "$(DESTDIR)$(INCLUDEDIR)/passphrase.h"
	-rm -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)/COPYING"
	-rm -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)/LICENSE"
	-rmdir -- "$(DESTDIR)$(LICENSEDIR)/$(PKGNAME)"
	-rm -- "$(DESTDIR)$(INFODIR)/$(PKGNAME).info"
	-rm -- "$(DESTDIR)$(INFODIR)/$(PKGNAME).pdf"
	-rm -- "$(DESTDIR)$(INFODIR)/$(PKGNAME).ps"
	-rm -- "$(DESTDIR)$(INFODIR)/$(PKGNAME).dvi"


.PHONY: clean
clean:
	-rm -r bin obj *.info *.pdf *.ps *.dvi

