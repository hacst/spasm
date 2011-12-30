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

#ifndef ELFWRITE_H_
#define ELFWRITE_H_

#include <stdio.h>
#include <stdint.h>

/**
 *  @brief Writes an ELF executable with the given parameters to the given file.
 *  @param file File to write to
 *  @param entry_point Virtual address of entry point
 *  @param text_vaddr Address to load .text segment to
 *  @param text Code to write
 *  @param text_size Size of the given code
 *  @param rodata_vaddr Address to load .rodata segment to
 *  @param rodata Read only data to write
 *  @param rodata_size Size of the given read only data
 *  @param data_vaddr Address to load .data segment to
 *  @param data Writable data to write (== initialized variables)
 *  @param data_size Size of the given writable data
 *  @param bss_vaddr Address to put the zero initialized writable segment at (== 0 initialized variables)
 *  @param bss_size Size to reserve for the zero initialized data
 */
void elf_write(FILE *file,
        uint32_t entry_point,
        uint32_t text_vaddr,
        const unsigned char *text, size_t text_size,
        uint32_t rodata_vaddr,
        const unsigned char *rodata, size_t rodata_size,
        uint32_t data_vaddr,
        const unsigned char *data, size_t data_size,
        uint32_t bss_vaddr,
        size_t bss_size);

/**
 *  @brief Return a set of addresses for the required sizes that prevent
 *         padding from being required in files written with elf_write.
 *
 *  @param base_vaddr Lowest vaddr to consider.
 *  @param text_size Size of text segment
 *  @param rodata_size Size of rodata segment
 *  @param data_size Size of data segment
 *  @param text_vaddr Target variable for optimal text vaddr.
 *  @param rodata_vaddr Target variable for optimal rodata vaddr.
 *  @param data_vaddr Target variable for optimal data vaddr.
 *  @param bss_vaddr Target variable for optimal bss vaddr.
 */
void elf_optimize_alignment(
        uint32_t base_vaddr,
        uint32_t text_size,
        uint32_t rodata_size,
        uint32_t data_size,
        uint32_t *text_vaddr,
        uint32_t *rodata_vaddr,
        uint32_t *data_vaddr,
        uint32_t *bss_vaddr);

#endif /* ELFWRITE_H_ */
