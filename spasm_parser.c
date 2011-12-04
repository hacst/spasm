/*
 * Copyright (C) 2011, Stefan Hacker
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

#include "spasm_parser.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>

Label *get_label(ParserState *parser, const char *name, const size_t len)
{
    Label *cur = parser->label_first;

    assert(len <= MAX_SYMBOL_NAME_LENGTH);

    while (cur)
    {
        if (strncmp(cur->name, name, len) == 0)
            return cur;

        cur = cur->next;
    }

    return 0;
}

Label *get_or_insert_label(ParserState *parser, const char *name, const size_t len)
{
    Label *cur = get_label(parser, name, len);

    if (cur)
        return cur;

    cur = (Label*)calloc(1, sizeof(Label));
    if (!cur)
        return 0;

    strncpy(cur->name, name, len);

    if (parser->label_first == 0)
        parser->label_first = cur;
    else
        parser->label_last->next = cur;

    parser->label_last = cur;

    return cur;
}

MemoryLocation *get_bss_variable(ParserState *parser, const char *name, const size_t len)
{
    MemoryLocation *cur = parser->memory_location_first;

    assert(len <= MAX_SYMBOL_NAME_LENGTH);

    while (cur)
    {
        if (cur->type == SPASM_BSS && strncmp(cur->name, name, len) == 0)
            return cur;

        cur = cur->next;
    }
    return 0;
}

MemoryLocation *insert_bss_variable(ParserState *parser, const char *name, const size_t len, const uint32_t size, const uint32_t line_num)
{
    MemoryLocation *mem = (MemoryLocation*)calloc(1, sizeof(MemoryLocation));

    assert(get_bss_variable(parser, name, len) == 0);

    if (!mem)
        return 0;

    strncpy(mem->name, name, len);
    mem->size = size;
    mem->type = SPASM_BSS;
    mem->source_line = line_num;

    if (parser->memory_location_first == 0)
        parser->memory_location_first = mem;
    else
        parser->memory_location_last->next = mem;

    parser->memory_location_last = mem;

    parser->bss_used += size;

    return mem;
}

Errc check_result(ParserState *parser)
{
    Command *cmd = parser->command_first;

    /*
     * Check if empty
     */
    if (!cmd)
    {
        return ERR_NO_COMMANDS;
    }

    /*
     *  Check for undefined labels
     */

    while(cmd)
    {
        if (cmd->type == SPASM_JMP
         || cmd->type == SPASM_JIN)
        {
            if (cmd->argument.label_arg == 0)
            {
                return ERR_UNDEFINED_LABEL;
            }
        }
        cmd = cmd->next;
    }

    return ERR_SUCCESS;
}

const char* skip_spaces(const char *buffer)
{
    const char *ret = buffer;
    while (*ret == ' ' || *ret == '\t')
        ++ret;

    return ret;
}

const char* skip_alnums(const char *buffer)
{
    const char *ret = buffer;

    while (*ret != '\0' && isalnum(*ret))
        ++ret;

    return ret;
}

Errc read_to_end_of_line(const char *buffer)
{
    buffer = skip_spaces(buffer);
    if (*buffer != '\0'         /* End of buffer without newline (e.g. last line) */
         && *buffer != ';'      /* Comment to end of line */
         && *buffer != '\n'     /* *nix style newline */
         && (buffer[0] != '\r' || buffer[1] != '\n')) /* win style newline */
        return ERR_SYNTAX;

    return ERR_SUCCESS;
}

Command* insert_command(ParserState *parser, CommandType type, uint32_t line_num, Label *label)
{
    Command *cmd = (Command*)calloc(1, sizeof(Command));
    if (!cmd)
        return 0;

    if (label)
        label->command = cmd;

    cmd->type = type;
    cmd->source_line = line_num;
    cmd->label = label;

    if (parser->command_first == 0)
        parser->command_first = cmd;
    else
        parser->command_last->next = cmd;

    parser->command_last = cmd;

    return cmd;
}

Errc parse_command(ParserState *parser, const uint32_t line_num, const char *buffer, Label *label)
{
    CommandType type = 0;
    const char *tmp;

    Command *cmd;
    Label *label_arg;
    MemoryLocation *memory_arg;
    unsigned long int constant_arg;
    size_t mmlen;

    while (type < SPASM_RUNTIME_COMMAND_COUNT)
    {
        mmlen = strlen(SPASM_MNEMONICS[type]);
        if (strncmp(buffer, SPASM_MNEMONICS[type], mmlen) == 0)
        {
            buffer = skip_spaces(buffer + mmlen);
            switch(type)
            {
            case SPASM_LA:
                /*
                 * Commands expecting a memory location as argument
                 */

                if (*(buffer++) != '$')
                    return ERR_SYNTAX;

                tmp = skip_alnums(buffer);
                memory_arg = get_bss_variable(parser, buffer, tmp - buffer);
                if (!memory_arg)
                    return ERR_UNDEFINED_VARIABLE;

                buffer = tmp;
                if (read_to_end_of_line(buffer) != ERR_SUCCESS)
                    return ERR_SYNTAX;

                cmd = insert_command(parser, type, line_num, label);
                if (!cmd)
                    return ERR_ALLOC;

                cmd->argument.memory_arg = memory_arg;

                return ERR_SUCCESS;

            case SPASM_LC:
                /*
                 * Commands expecting a constant unsigned integer as argument
                 */

                if (!isdigit(*buffer))
                    return ERR_SYNTAX;

                errno = 0;
                constant_arg = strtoul(buffer, (char**)&buffer, 10);
                if (errno != 0)
                    return ERR_SYNTAX;

                if (constant_arg > UINT_MAX)
                    return ERR_CONSTANT_RANGE;

                if (read_to_end_of_line(buffer) != ERR_SUCCESS)
                    return ERR_SYNTAX;

                cmd = insert_command(parser, type, line_num, label);
                if (!cmd)
                    return ERR_ALLOC;

                cmd->argument.constant_arg = (uint32_t) constant_arg;

                return ERR_SUCCESS;

            case SPASM_JMP:
            case SPASM_JIN:
                /*
                 * Commands expecting a label as argument
                 */

                if (*(buffer++) != '#')
                    return ERR_SYNTAX;

                tmp = skip_alnums(buffer);
                label_arg = get_or_insert_label(parser, buffer, tmp - buffer);
                if (!label_arg)
                    return ERR_ALLOC;

                buffer = tmp;
                if (read_to_end_of_line(buffer) != ERR_SUCCESS)
                    return ERR_SYNTAX;

                cmd = insert_command(parser, type, line_num, label);
                if (!cmd)
                    return ERR_ALLOC;

                cmd->argument.label_arg = label_arg;

                return ERR_SUCCESS;

            default:

                /*
                 * Commands without further parameters
                 */
                if (read_to_end_of_line(buffer) != ERR_SUCCESS)
                    return ERR_SYNTAX;

                if (!insert_command(parser, type, line_num, label))
                    return ERR_ALLOC;

                return ERR_SUCCESS;
            }
        }
        ++type;
    }
    return ERR_INVALID_MNEMONIC;
}

Errc parse_line(ParserState *parser, const uint32_t line_num, const char *line)
{
    const char *cur = line;
    const char *tmp;
    const char *tmp_end;
    unsigned long int size;
    Label *label = 0;
    const size_t DS_LEN = strlen(SPASM_MNEMONICS[SPASM_DS]);

    cur = skip_spaces(cur);

    if (read_to_end_of_line(cur) == ERR_SUCCESS)
    {
        /* Enable skipping of empty or comment only lines */
        return ERR_SUCCESS;
    }

    if (strncmp(cur, SPASM_MNEMONICS[SPASM_DS], DS_LEN) == 0)
    {
        /* Variable declaration:
         *     DS $variablename 42
         */
        cur += DS_LEN;
        cur = skip_spaces(cur);

        if (*(cur++) != '$')
            return ERR_SYNTAX;

        tmp = cur;
        cur = skip_alnums(cur);
        if (get_bss_variable(parser, tmp, cur - tmp))
            return ERR_VARIABLE_REDEFINITION;

        tmp_end = cur;

        cur = skip_spaces(cur);
        if (!isdigit(*cur))
            return ERR_SYNTAX;

        errno = 0;
        size = strtoul(cur, (char**)&cur, 10);
        if (errno != 0)
            return ERR_SYNTAX;

        if (size > UINT_MAX)
            return ERR_CONSTANT_RANGE;

        if (read_to_end_of_line(cur) != ERR_SUCCESS)
            return ERR_SYNTAX;

        if (insert_bss_variable(parser, tmp, tmp_end - tmp, (uint32_t) size, line_num) == 0)
            return ERR_ALLOC;

        return ERR_SUCCESS;
    }

    if (*cur == '#')
    {
        /* Label */
        ++cur;
        tmp = skip_alnums(cur);

        label = get_or_insert_label(parser, cur, tmp - cur);
        if (!label)
            return ERR_ALLOC;

        cur = skip_spaces(tmp);
    }

    return parse_command(parser, line_num, cur, label);
}

Errc parse_file(ParserState *parser, FILE *file)
{
    char buffer[MAX_LINE_LENGTH + 1];
    char *cur = buffer;
    const char *LAST = buffer + MAX_LINE_LENGTH;
    int chr;
    uint32_t line = 1;
    Errc result;

    while ((chr = fgetc(file)) != EOF)
    {
        parser->last_line = line;
        *cur = (char)chr;

        if (*cur == '\n')
        {
            *(cur + 1) = '\0';
            result = parse_line(parser, line, buffer);

            if (result != ERR_SUCCESS)
                return result;

            ++line;
            cur = buffer;
            continue;
        }

        ++cur;
        if (cur > LAST)
            return ERR_LINE_TO_LONG;
    }

    parser->last_line = line;

    if (cur != buffer)
    {
        *(cur + 1) = '\0';
        result = parse_line(parser, line, buffer);

        if (result != ERR_SUCCESS)
            return result;
    }

    return check_result(parser);
}

void init_parser(ParserState *parser)
{
    memset(parser, 0, sizeof(ParserState));
}

void cleanup_parser(ParserState *parser)
{
    Command *cmd = parser->command_first;
    Command *cmd_del;

    MemoryLocation *mem = parser->memory_location_first;
    MemoryLocation *mem_del;

    Label *lbl = parser->label_first;
    Label *lbl_del;

    while(cmd)
    {
        cmd_del = cmd;
        cmd = cmd->next;
        free(cmd_del);
    }

    while(mem)
    {
        mem_del = mem;
        mem = mem->next;
        free(mem_del->content);
        free(mem_del);
    }

    while(lbl)
    {
        lbl_del = lbl;
        lbl = lbl->next;
        free(lbl_del);
    }

    memset(parser, 0, sizeof(ParserState));
}

