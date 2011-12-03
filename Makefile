C = gcc
ifeq ($(mode),release)
	CFLAGS = -O3 -pedantic -pedantic-errors -Wall -std=c89
else
	mode = debug
	CFLAGS = -O0 -g3 -pedantic -pedantic-errors -Wall -std=c89
endif

MODULES = spasm

all : $(MODULES)

spasm: spasm.c helpers/elfwrite.c
	$(C) $(CFLAGS) -o $@ $^

clean:
	rm -f $(MODULES)

.PHONY: all
.PHONY: clean
