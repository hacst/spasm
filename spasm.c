/*
 * Copyright (C) 2011, Stefan Hacker <dd0t@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spasm_parser.h"
#include "spasm_writer.h"
#include "helpers/elfwrite.h"

void print_cmd(Command *cmd)
{
    printf("0x%x l.%u ", cmd->vaddr, cmd->source_line);

    if (cmd->label)
    {
        printf("#%s ", cmd->label->name);
    }

    printf("%s", SPASM_MNEMONICS[cmd->type]);

    switch (cmd->type)
    {
    case SPASM_LC:
        printf(" %d", cmd->argument.constant_arg);
        break;
    case SPASM_JMP:
    case SPASM_JIN:
        printf(" #%s -> [", cmd->argument.label_arg->name);
        print_cmd(cmd->argument.label_arg->command);
        printf("]");
        break;
    case SPASM_LA:
        printf(" $%s [0x%x]",
                cmd->argument.memory_arg->name,
                cmd->argument.memory_arg->vaddr);
        break;
    default: break;
    }
}


void print_info(const ParserState *parser)
{
    Command *cmd;
    MemoryLocation *mem;
    Label *lbl;

    printf("===INFO===\n");
    printf("Variables:\n");
    mem = parser->memory_location_first;
    while (mem)
    {
        printf("0x%x - 0x%x l.%u $%s (%i bytes)\n", mem->vaddr, mem->vaddr + mem->size - 1, mem->source_line, mem->name, mem->size);
        mem = mem->next;
    }
    printf("\nCommands:\n");
    cmd = parser->command_first;
    while (cmd)
    {
        print_cmd(cmd);
        printf("\n");
        cmd = cmd->next;
    }

    printf("\nLabels:\n");
    lbl = parser->label_first;
    while (lbl)
    {
        printf("l.%u #%s -> ", lbl->command->source_line, lbl->name);
        print_cmd(lbl->command);
        printf("\n");
        lbl = lbl->next;
    }
    printf("===ENDOFINFO===\n\n");
}

void print_usage(const char *name)
{
    fprintf(stderr, "Usage:\n"
           "    %s <source> <target> [-i/--info]\n", name);
}

int main(int argn, char **argv)
{
    FILE *source;
    FILE *target;
    ParserState parser;
    Errc result;
    int verbose = 0;

    if (argn < 3)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    else if (argn == 4)
    {
        verbose = strcmp(argv[3], "--info") == 0 || strcmp(argv[3], "-i") == 0;
        if (!verbose)
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    source = fopen(argv[1], "r");
    if (!source)
    {
        fprintf(stderr, "Failed to open source file \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Parsing input [%s]...", argv[1]);
    init_parser(&parser);
    result = parse_file(&parser, source);
    if (result != ERR_SUCCESS)
    {
        printf("FAILED\n");
        fprintf(stderr, "Failed to parse source file, reason: %s line %u\n",
                SPASM_ERR_STR[result], parser.last_line);

        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

    fclose(source);
    printf("DONE\n");

    printf("Writing binary [%s]....", argv[2]);
    target = fopen(argv[2], "wb");
    if (!target)
    {
        printf("FAILED\n");
        fprintf(stderr, "Failed to open target file \"%s\"\n", argv[2]);
        return EXIT_FAILURE;
    }

    result = write_program(&parser, target);
    if (result != ERR_SUCCESS)
    {
        printf("FAILED\n");
        fprintf(stderr, "Failed to write program file, reason: %s", SPASM_ERR_STR[result]);
        cleanup_parser(&parser);
        fclose(target);
        return EXIT_FAILURE;
    }

    fclose(target);
    printf("DONE\n");

    if (verbose)
    {
        printf("\n");
        print_info(&parser);
        printf("\n");
    }

    printf("Cleanup...");
    cleanup_parser(&parser);
    printf("DONE\n");

    return EXIT_SUCCESS;
}
