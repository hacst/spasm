
ifeq ($(tool), clang)
	C = clang
else
	C = gcc
endif

ifeq ($(arch), 32)
	ARCHFLAG = -m32
else ifeq ($(arch), 64)
	ARCHFLAG = -m64
else
	ARCHFLAG =
endif

ifeq ($(mode),release)
	CFLAGS = -O3 -pedantic -pedantic-errors -Wall -std=c89 $(ARCHFLAG)
else
	mode = debug
	CFLAGS = -O0 -g3 -pedantic -pedantic-errors -Wall -std=c89 $(ARCHFLAG)
endif

MODULES = spasm

all : $(MODULES)

spasm: spasm_types.c spasm_writer.c spasm_parser.c spasm_commands.c helpers/elfwrite.c spasm.c
	$(C) $(CFLAGS) -o $@ $^

clean:
	rm -f $(MODULES)

.PHONY: all
.PHONY: clean
