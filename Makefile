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
WARN = -Wall -Wextra -pedantic -Wdouble-promotion -Wformat=2 -Winit-self -Wmissing-include-dirs \
       -Wfloat-equal -Wmissing-prototypes -Wmissing-declarations -Wtrampolines -Wnested-externs \
       -Wno-variadic-macros -Wdeclaration-after-statement -Wundef -Wpacked -Wunsafe-loop-optimizations \
       -Wbad-function-cast -Wwrite-strings -Wlogical-op -Wstrict-prototypes -Wold-style-definition \
       -Wvector-operation-performance -Wstack-protector -Wunsuffixed-float-constants -Wcast-align \
       -Wsync-nand -Wshadow -Wredundant-decls -Winline -Wcast-qual -Wsign-conversion -Wstrict-overflow
# The C standard for C code compilation
STD = gnu99
# C preprocessor flags
CPPFLAGS_ = $(foreach D, $(OPTIONS), -D'$(D)=1') $(CPPFLAGS)
# C compiling flags
CFLAGS_ = -std=$(STD) $(WARN) -fPIC $(CFLAGS)
# Linking flags
LDFLAGS_ = -shared $(LDFLAGS)

# Flags to use when compiling and assembling
CC_FLAGS = $(CPPFLAGS_) $(CFLAGS_) $(OPTIMISE)
# Flags to use when linking
LD_FLAGS = $(LDFLAGS_) $(CFLAGS_) $(OPTIMISE)

SRC = passphrase
OBJ = $(foreach S, $(SRC), obj/$(S).o)


.PHONY: default
default: libpassphrase info

.PHONY: all
all: libpassphrase doc

.PHONY: doc
doc: info pdf ps dvi

.PHONY: libpassphrase
libpassphrase: bin/libpassphrase.so

bin/libpassphrase.so: $(OBJ)
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -o "$@" $^

obj/%.o: src/%.c src/%.h
	@mkdir -p "$(shell dirname "$@")"
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"

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
install-base: install-lib install-license

.PHONY: install-lib
install-lib: bin/libpassphrase.so libpassphrase.info
	install -dm755 -- "$(DESTDIR)$(LIBDIR)"
	install -dm755 -- "$(DESTDIR)$(INCLUDEDIR)"
	install  -m755 -- bin/libpassphrase.so "$(DESTDIR)$(LIBDIR)"
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
	-rm -r bin obj libpassphrase.{info,pdf,ps,dvi}

