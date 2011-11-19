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

/**
 *  @brief Writes an ELF executable with the given parameters to the given file.
 *  @param file File to write to
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
        unsigned int text_vaddr,
        const unsigned char *text, size_t text_size,
        unsigned int rodata_vaddr,
        const unsigned char *rodata, size_t rodata_size,
        unsigned int data_vaddr,
        const unsigned char *data, size_t data_size,
        unsigned int bss_vaddr,
        size_t bss_size);

#endif /* ELFWRITE_H_ */
