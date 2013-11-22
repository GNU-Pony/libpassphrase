PREFIX = /usr
LIB = /lib
INCLUDE = /include

OPTIMISE = -Os
CPPFLAGS = 
CFLAGS = -std=c90 -Wall -Wextra -fPIC
LDFLAGS = -shared

CC_FLAGS = $(CPPFLAGS) $(CFLAGS) $(OPTIMISE)
LD_FLAGS = $(LDFLAGS) $(CFLAGS) $(OPTIMISE)

SRC = passphrase
OBJ = $(foreach S, $(SRC), obj/$(S).o)


.PHONY: all
all: bin/libpassphrase.so


bin/libpassphrase.so: $(OBJ)
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -o "$@" $^

obj/%.o: src/%.c src/%.h
	@mkdir -p "$(shell dirname "$@")"
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"


.PHONY: install
install: bin/libpassphrase.so
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(LIB)"
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(INCLUDE)"
	install -m755 -- bin/libpassphrase.so "$(DESTDIR)$(PREFIX)$(LIB)"
	install -m755 -- src/passphrase.h "$(DESTDIR)$(PREFIX)$(INCLUDE)"


.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)$(LIB)/libpassphrase.so"
	-rm -- "$(DESTDIR)$(PREFIX)$(INCLUDE)/passphrase.h"


.PHONY: clean
clean:
	-rm -r bin obj

