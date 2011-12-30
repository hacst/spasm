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
#include "spasm_writer.h"
#include "spasm_commands.h"
#include "helpers/elfwrite.h"

#include <memory.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


const unsigned char *SPASM_COMMANDTYPE_TO_COMMAND[] = {
        spasm_add,
        spasm_mul,
        spasm_sub,
        spasm_div,

        spasm_les,
        spasm_and,
        spasm_equ,
        spasm_not,

        spasm_la,
        spasm_lc,
        spasm_lv,
        spasm_str,

        spasm_pri,
        spasm_rea,

        spasm_jmp,
        spasm_jin,
        spasm_nop,
        spasm_stp,

        0,
        0
};

const size_t SPASM_COMMANDTYPE_TO_COMMAND_SIZE[] = {
        sizeof(spasm_add),
        sizeof(spasm_mul),
        sizeof(spasm_sub),
        sizeof(spasm_div),

        sizeof(spasm_les),
        sizeof(spasm_and),
        sizeof(spasm_equ),
        sizeof(spasm_not),

        sizeof(spasm_la),
        sizeof(spasm_lc),
        sizeof(spasm_lv),
        sizeof(spasm_str),

        sizeof(spasm_pri),
        sizeof(spasm_rea),

        sizeof(spasm_jmp),
        sizeof(spasm_jin),
        sizeof(spasm_nop),
        sizeof(spasm_stp),

        0,
        0
};

Errc write_raw_command_with_uint32_data(
        const unsigned char command[],
        const size_t command_size,
        const size_t command_data_offset,
        const uint32_t data,
        unsigned char **buffer)
{
    const size_t behind_data_offset = command_data_offset + sizeof(data);

    assert(command_size >= command_data_offset + sizeof(data));

    memcpy(*buffer, command, command_data_offset);
    *buffer += command_data_offset;

    memcpy(*buffer, &data, sizeof(data));
    *buffer += sizeof(data);

    memcpy(*buffer, command + behind_data_offset, command_size - behind_data_offset);
    *buffer += command_size - behind_data_offset;

    return ERR_SUCCESS;
}

Errc write_command(const Command *command, unsigned char **buffer)
{
    switch (command->type)
    {
    case SPASM_REA:
        return write_raw_command_with_uint32_data(
                spasm_rea, sizeof(spasm_rea), 1,
                (uint32_t)((int32_t)0x08048080 - (int32_t)(command->vaddr + 1 + 4)),
                buffer);
    case SPASM_PRI:
        return write_raw_command_with_uint32_data(
                spasm_pri, sizeof(spasm_pri), 2,
                (uint32_t)((int32_t)(0x08048080 + sizeof(spasm_readint32)) - (int32_t)(command->vaddr + 2 + 4)),
                buffer);
    case SPASM_JMP:
        return write_raw_command_with_uint32_data(
                spasm_jmp, sizeof(spasm_jmp), 1,
                (uint32_t)((int32_t)command->argument.label_arg->command->vaddr - (int32_t)(command->vaddr + 1 + 4)),
                buffer);
    case SPASM_JIN:
        return write_raw_command_with_uint32_data(
                spasm_jin, sizeof(spasm_jin), 5,
                (uint32_t)((int32_t)command->argument.label_arg->command->vaddr - (int32_t)(command->vaddr + 5 + 4)),
                buffer);
    case SPASM_LC:
        return write_raw_command_with_uint32_data(spasm_lc, sizeof(spasm_lc), 1, command->argument.constant_arg, buffer);
    case SPASM_LA:
        assert(command->argument.memory_arg->vaddr % 4 == 0);
        return write_raw_command_with_uint32_data(spasm_la, sizeof(spasm_la), 1, command->argument.memory_arg->vaddr / 4, buffer);
    default:
        memcpy(*buffer,
                SPASM_COMMANDTYPE_TO_COMMAND[command->type],
                SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type]);

        *buffer += SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type];

        return ERR_SUCCESS;
    }
}

Errc write_text(const ParserState *parser, unsigned char *buffer)
{
    unsigned char *current = buffer;
    const Command *command = parser->command_first;
    while (command != 0)
    {
        Errc error = write_command(command, &current);
        if (error != ERR_SUCCESS)
            return error;

        command = command->next;
    }

    return ERR_SUCCESS;
}

Errc write_xdata(const ParserState *parser, unsigned char *data_buffer, unsigned char *rodata_buffer)
{
    MemoryLocation *location = parser->memory_location_first;

    while (location)
    {
        switch (location->type)
        {
        case SPASM_RODATA:
            memcpy(rodata_buffer, location->content, location->size);
            rodata_buffer += location->size;
            break;
        case SPASM_DATA:
            memcpy(data_buffer, location->content, location->size);
            data_buffer += location->size;
            break;
        default:
            break;
        }
        location = location->next;
    }

    return ERR_SUCCESS;
}

size_t update_parser_state_vaddr_info(
        ParserState *parser,
        uint32_t text_vaddr,
        uint32_t bss_vaddr,
        uint32_t rodata_vaddr,
        uint32_t data_vaddr)
{
    MemoryLocation *location = parser->memory_location_first;
    Command *command = parser->command_first;
    const uint32_t text_vaddr_first = text_vaddr;

    while (location)
    {
        switch(location->type)
        {
        case SPASM_BSS:
            location->vaddr = bss_vaddr;
            bss_vaddr += location->size;
            break;
        case SPASM_RODATA:
            location->vaddr = rodata_vaddr;
            rodata_vaddr += location->size;
            break;
        case SPASM_DATA:
            location->vaddr = data_vaddr;
            data_vaddr += location->size;
            break;
        default:
            break;
        }

        location = location->next;
    }

    while (command)
    {
        command->vaddr = text_vaddr;
        text_vaddr += SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type];

        command = command->next;
    }

    return text_vaddr - text_vaddr_first;
}

Errc write_program(ParserState *parser, FILE *file)
{
    const uint32_t text_vaddr_base = 0x08048080;
    const uint32_t rodata_vaddr_base = 0x15000000;
    const uint32_t data_vaddr_base = 0x20000000;
    const uint32_t bss_vaddr_base = 0x25000000;

    const size_t text_size = update_parser_state_vaddr_info(parser,
            text_vaddr_base +  sizeof(spasm_readint32) + sizeof(spasm_writeint32),
            bss_vaddr_base + spasm_bss_usage,
            rodata_vaddr_base + sizeof(spasm_rodata),
            data_vaddr_base)
            + sizeof(spasm_readint32)
            + sizeof(spasm_writeint32);

    unsigned char *text_buffer = malloc(text_size);
    unsigned char *rodata_buffer = malloc(parser->rodata_used + sizeof(spasm_rodata));
    unsigned char *data_buffer = malloc(parser->data_used);

    Errc result = ERR_SUCCESS;

    if (   !text_buffer
        || !rodata_buffer
        || (!data_buffer && !parser->data_used))
        return ERR_ALLOC;


    memcpy(text_buffer, spasm_readint32, sizeof(spasm_readint32));
    memcpy(text_buffer + sizeof(spasm_readint32), spasm_writeint32, sizeof(spasm_writeint32));

    result = write_text(parser, text_buffer + sizeof(spasm_readint32) + sizeof(spasm_writeint32));
    if (result != ERR_SUCCESS)
        goto cleanup;

    memcpy(rodata_buffer, spasm_rodata, sizeof(spasm_rodata));

    result = write_xdata(parser, data_buffer, rodata_buffer + sizeof(spasm_rodata));
    if (result != ERR_SUCCESS)
        goto cleanup;

    elf_write(file,
            text_vaddr_base  +  sizeof(spasm_readint32) + sizeof(spasm_writeint32),
            text_vaddr_base, text_buffer, text_size,
            rodata_vaddr_base, rodata_buffer, parser->rodata_used + sizeof(spasm_rodata),
            data_vaddr_base, data_buffer, parser->data_used,
            bss_vaddr_base, parser->bss_used + spasm_bss_usage);

cleanup:
    free(data_buffer);
    free(rodata_buffer);
    free(text_buffer);

    return result;
}

