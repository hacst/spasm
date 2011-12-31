C = gcc
#C = clang
ifeq ($(mode),release)
	CFLAGS = -O3 -pedantic -pedantic-errors -Wall -std=c89
else
	mode = debug
	CFLAGS = -O0 -g3 -pedantic -pedantic-errors -Wall -std=c89
endif

MODULES = spasm

all : $(MODULES)

spasm: spasm_types.c spasm_writer.c spasm_parser.c spasm_commands.c helpers/elfwrite.c spasm.c
	$(C) $(CFLAGS) -o $@ $^

clean:
	rm -f $(MODULES)

.PHONY: all
.PHONY: clean
