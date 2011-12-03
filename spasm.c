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
#include "helpers/elfwrite.h"

int main(void) {
    const unsigned char rodata[] = "Hello World.\n";

    const unsigned char text[] = {  0xba, 0x0e, 0x00, 0x00, 0x00,  /* mov    edx,0xc           */
                                    0xb9, 0xD6, 0x00, 0x09, 0x10,  /* mov    ecx,0x100900D6    */
                                    0xbb, 0x01, 0x00, 0x00, 0x00,  /* mov    ebx,0x1           */
                                    0xb8, 0x04, 0x00, 0x00, 0x00,  /* mov    eax,0x4           */
                                    0xcd, 0x80,                    /* int    0x80              */
                                    0xbb, 0x00, 0x00, 0x00, 0x00,  /* mov    ebx,0x0           */
                                    0xb8, 0x01, 0x00, 0x00, 0x00,  /* mov    eax,0x1           */
                                    0xcd, 0x80 };                  /* int    0x80              */


    const unsigned char data[] = "jajajaj";

    FILE *file = fopen("test.bin", "wb");

    elf_write(file,
            0x080480B4,
            0x080480B4,
            text, sizeof(text),
            0x100900D6,
            rodata, sizeof(rodata),
            0x180D80E4,
            data, sizeof(data),
            0x20120000,
            1);

    fclose(file);

    return EXIT_SUCCESS;
}
