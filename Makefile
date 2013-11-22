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


.PHONY: clean
clean:
	-rm -r bin obj

