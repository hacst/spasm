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

typedef struct SpasmBuiltins SpasmBuiltins;

/**
 * @brief Structure for passing builtin function virtual addresses
 *        around.
 */
struct SpasmBuiltins
{
    uint32_t readint32_vaddr; /* readint32 function vaddr */
    uint32_t printint32_vaddr; /* printint32 function vaddr */
};


/**
 * @brief CommandType to Command implementation mapper
 */
const unsigned char *SPASM_COMMANDTYPE_TO_COMMAND[] = {
        spasm_add, spasm_mul, spasm_sub, spasm_div,

        spasm_les, spasm_and, spasm_equ, spasm_not,

        spasm_la, spasm_lc, spasm_lv, spasm_str,

        spasm_pri, spasm_rea,

        spasm_jmp, spasm_jin, spasm_nop, spasm_stp,

        0, 0 };


/**
 * @brief CommandType to implementation size mapper
 */
const size_t SPASM_COMMANDTYPE_TO_COMMAND_SIZE[] = {
        sizeof(spasm_add), sizeof(spasm_mul), sizeof(spasm_sub),
        sizeof(spasm_div),

        sizeof(spasm_les), sizeof(spasm_and), sizeof(spasm_equ),
        sizeof(spasm_not),

        sizeof(spasm_la), sizeof(spasm_lc), sizeof(spasm_lv),
        sizeof(spasm_str),

        sizeof(spasm_pri), sizeof(spasm_rea),

        sizeof(spasm_jmp), sizeof(spasm_jin), sizeof(spasm_nop),
        sizeof(spasm_stp),

        0, 0 };


/**
 * @brief Write a command with a single uint32_t replacement.
 * @param command Command implementation to write.
 * @param command_size command size
 * @param command_data_offset Offset in command at which to perform replacement.
 * @param data Data to replace with.
 * @param buffer Buffer to write to. Will be advanced by size of command.
 * @return ERR_SUCCESS on success.
 */
Errc write_with_single_replacement(
        const unsigned char command[],
        const size_t command_size, const size_t command_data_offset,
        const uint32_t data, unsigned char **buffer) {

    const size_t behind_data_offset = command_data_offset + sizeof(data);

    assert(command_size >= command_data_offset + sizeof(data));

    memcpy(*buffer, command, command_data_offset);
    *buffer += command_data_offset;

    memcpy(*buffer, &data, sizeof(data));
    *buffer += sizeof(data);

    memcpy(*buffer, command + behind_data_offset, command_size
            - behind_data_offset);
    *buffer += command_size - behind_data_offset;

    return ERR_SUCCESS;
}


/**
 * @brief Write a command wit multiple uint32_t replacements.
 * @param command Command implementation to write.
 * @param command_size command size
 * @param offsets Array of offsets in command at which to perform replacements in ascending order.
 * @param replacements Replacement values for offsets indexes.
 * @param replacement_count Number of offsets/replacements.
 * @param buffer Buffer to write to. Will be advanced by size of command.
 * @return ERR_SUCCESS on success.
 */
Errc write_with_replacements(
        const unsigned char command[], size_t command_size,
        const uint32_t offsets[], const uint32_t replacements[], const size_t replacement_count,
        unsigned char **buffer)
{
    size_t i;
    uint32_t prevoff = 0;
    uint32_t curoff = 0;

    for (i = 0; i < replacement_count; ++i)
    {
        curoff = offsets[i] - prevoff;

        memcpy(*buffer, command, curoff);
        *buffer += curoff;
        command += curoff;

        memcpy(*buffer, &(replacements[i]), sizeof(int32_t));
        *buffer += sizeof(int32_t);
        command += sizeof(int32_t);

        command_size -= curoff + sizeof(int32_t);
        prevoff = offsets[i] + sizeof(int32_t);
    }

    memcpy(*buffer, command, command_size);
    *buffer += command_size;

    return ERR_SUCCESS;
}


/**
 * @brief Write the builtin spasm_readint32 command to the given buffer.
 * @param rodata_vaddr_base Base of rodata segment.
 * @param data_vaddr_base Base of data segment.
 * @param buffer Buffer to write to. Will be advanced by size of command.
 */
Errc write_spasm_readint32(const uint32_t rodata_vaddr_base, const uint32_t data_vaddr_base, unsigned char **buffer)
{
    const uint32_t prompt_rodata_vaddr = rodata_vaddr_base + 0;
    const uint32_t ofm_rodata_vaddr = rodata_vaddr_base + 42;
    const uint32_t nanm_rodata_vaddr = rodata_vaddr_base + 2;
    const uint32_t strbuf_data_vaddr = data_vaddr_base + 0;

    const uint32_t offsets[5] = {
            11,
            33,
            50,
            113,
            137
    };

    uint32_t replacements[5];
    replacements[0] = prompt_rodata_vaddr;
    replacements[1] = strbuf_data_vaddr;
    replacements[2] = strbuf_data_vaddr;
    replacements[3] = ofm_rodata_vaddr;
    replacements[4] = nanm_rodata_vaddr;

    return write_with_replacements(spasm_readint32, sizeof(spasm_readint32), offsets, replacements, 5, buffer);
}


/**
 * @brief Write the builtin spasm_writeint32 command to the given buffer.
 * @param data_vaddr_base Base of data segment.
 * @param buffer Buffer to write to. Will be advanced by size of command.
 */
Errc write_spasm_writeint32(const uint32_t data_vaddr_base, unsigned char **buffer)
{
    const uint32_t strbuf_data_vaddr = data_vaddr_base + 0;

    const uint32_t offsets[2] = {
            1,
            62
    };

    uint32_t replacements[2];
    replacements[0] = strbuf_data_vaddr + 255 - 1;
    replacements[1] = strbuf_data_vaddr + 255;

    return write_with_replacements(spasm_writeint32, sizeof(spasm_writeint32), offsets, replacements, 2, buffer);
}


/**
 * @brief Writes the implementation of a given command to the given buffer.
 * @param command Command to write
 * @param buffer Buffer to write to. Will be advanced by command implementation size.
 * @param builtins Builtin function addresses.
 * @param return ERR_SUCCESS on success.
 */
Errc write_command(const Command *command, unsigned char **buffer, const SpasmBuiltins *builtins) {
    switch (command->type) {
    case SPASM_REA:
        return write_with_single_replacement(spasm_rea, sizeof(spasm_rea),
                1, (uint32_t)((int64_t) builtins->readint32_vaddr - (int64_t) (command->vaddr + 1 + 4)), buffer);
    case SPASM_PRI:
        return write_with_single_replacement(spasm_pri, sizeof(spasm_pri),
                2, (uint32_t)((int64_t) builtins->printint32_vaddr - (int64_t) (command->vaddr + 2 + 4)), buffer);
    case SPASM_JMP:
        return write_with_single_replacement(spasm_jmp, sizeof(spasm_jmp),
                1, (uint32_t)(
                        (int64_t) command->argument.label_arg->command->vaddr
                                - (int64_t) (command->vaddr + 1 + 4)), buffer);
    case SPASM_JIN:
        return write_with_single_replacement(spasm_jin, sizeof(spasm_jin),
                5, (uint32_t)(
                        (int64_t) command->argument.label_arg->command->vaddr
                                - (int64_t) (command->vaddr + 5 + 4)), buffer);
    case SPASM_LC:
        return write_with_single_replacement(spasm_lc, sizeof(spasm_lc),
                1, command->argument.constant_arg, buffer);
    case SPASM_LA:
        assert(command->argument.memory_arg->vaddr % 4 == 0);
        return write_with_single_replacement(spasm_la, sizeof(spasm_la),
                1, command->argument.memory_arg->vaddr / 4, buffer);
    default:
        memcpy(*buffer, SPASM_COMMANDTYPE_TO_COMMAND[command->type],
                SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type]);

        *buffer += SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type];

        return ERR_SUCCESS;
    }
}


/**
 * @brief Writes the text segment part of the ParserState into the given buffer.
 * @param parser Parser to write data from.
 * @param buffer Buffer to write implementation to.
 * @param builtins Builtin function addresses.
 * @return ERR_SUCCESS on success.
 */
Errc write_text(const ParserState *parser, unsigned char *buffer, const SpasmBuiltins *builtins) {
    unsigned char *current = buffer;
    const Command *command = parser->command_first;
    while (command != 0) {
        Errc error = write_command(command, &current, builtins);
        if (error != ERR_SUCCESS)
            return error;

        command = command->next;
    }

    return ERR_SUCCESS;
}


/**
 * @brief Write data/rodata part of ParserState into given buffers.
 * @param parser Parser to write data from.
 * @param data_buffer Buffer to write data memory location initial content to.
 * @param rodata_buffer Buffer to write rodata memory location content to.
 * @return ERR_SUCCESS on success.
 */
Errc write_xdata(const ParserState *parser, unsigned char *data_buffer,
        unsigned char *rodata_buffer) {

    MemoryLocation *location = parser->memory_location_first;

    while (location) {
        switch (location->type) {
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


/**
 * @brief Walk given ParserState and set actual virtual addresses of memory and commands.
 * @param parser State
 * @param text_vaddr Base virtual address for text segment.
 * @param bss_vaddr Base virtual address for bss segment.
 * @param rodata_vaddr Base virtual address for rodata segment.
 * @param data_vaddr Base virtual address for data segment.
 * @return Required text segment size.
 */
size_t update_parser_state_vaddr_info(ParserState *parser, uint32_t text_vaddr,
        uint32_t bss_vaddr, uint32_t rodata_vaddr, uint32_t data_vaddr) {

    MemoryLocation *location = parser->memory_location_first;
    Command *command = parser->command_first;
    const uint32_t text_vaddr_first = text_vaddr;

    while (location) {
        switch (location->type) {
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

    while (command) {
        command->vaddr = text_vaddr;
        text_vaddr += SPASM_COMMANDTYPE_TO_COMMAND_SIZE[command->type];

        command = command->next;
    }

    return text_vaddr - text_vaddr_first;
}




Errc write_program(ParserState *parser, FILE *file) {
    uint32_t text_vaddr_base;
    uint32_t rodata_vaddr_base;
    uint32_t data_vaddr_base;
    uint32_t bss_vaddr_base;

    uint32_t entry_vaddr;

    SpasmBuiltins builtins;

    /* Do a dry run to get text_size */
    const size_t text_size = update_parser_state_vaddr_info(parser, 0, 0, 0, 0)
            + sizeof(spasm_readint32) + sizeof(spasm_writeint32);

    const size_t rodata_size = parser->rodata_used + sizeof(spasm_rodata);
    const size_t data_size = parser->data_used;
    const size_t bss_size = parser->bss_used + spasm_bss_usage;

    unsigned char *text_buffer = malloc(text_size);
    unsigned char *text_buffer_tmp = text_buffer;
    unsigned char *rodata_buffer = malloc(rodata_size);
    unsigned char *data_buffer = malloc(bss_size);

    Errc result = ERR_SUCCESS;

    if (!text_buffer || !rodata_buffer || (!data_buffer && !parser->data_used))
        return ERR_ALLOC;

    elf_optimize_alignment(0x08048000, text_size, rodata_size, data_size,
            &text_vaddr_base, &rodata_vaddr_base, &data_vaddr_base,
            &bss_vaddr_base);

    builtins.readint32_vaddr = text_vaddr_base;
    builtins.printint32_vaddr = text_vaddr_base + sizeof(spasm_readint32);

    entry_vaddr = text_vaddr_base + sizeof(spasm_readint32)
            + sizeof(spasm_writeint32);
    /* Do actual update run with optimized address values */
    update_parser_state_vaddr_info(parser, entry_vaddr, bss_vaddr_base
            + spasm_bss_usage, rodata_vaddr_base + sizeof(spasm_rodata),
            data_vaddr_base);


    write_spasm_readint32(rodata_vaddr_base, data_vaddr_base, &text_buffer_tmp);
    assert(text_buffer_tmp == text_buffer + sizeof(spasm_readint32));
    write_spasm_writeint32(data_vaddr_base, &text_buffer_tmp);
    assert(text_buffer_tmp == text_buffer + sizeof(spasm_readint32) + sizeof(spasm_writeint32));

    result = write_text(parser, text_buffer_tmp, &builtins);
    if (result != ERR_SUCCESS)
        goto cleanup;

    memcpy(rodata_buffer, spasm_rodata, sizeof(spasm_rodata));

    result = write_xdata(parser, data_buffer, rodata_buffer + sizeof(spasm_rodata));
    if (result != ERR_SUCCESS)
        goto cleanup;

    elf_write(file, entry_vaddr, text_vaddr_base, text_buffer, text_size,
            rodata_vaddr_base, rodata_buffer, rodata_size, data_vaddr_base,
            data_buffer, data_size, bss_vaddr_base, bss_size);

    cleanup: free(data_buffer);
    free(rodata_buffer);
    free(text_buffer);

    return result;
}

