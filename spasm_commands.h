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

#ifndef SPASM_COMMANDS
#define SPASM_COMMANDS

#include <stdint.h>

extern const unsigned char spasm_nop[1];
extern const unsigned char spasm_jin[9];
extern const unsigned char spasm_add[5];
extern const unsigned char spasm_and[5];
extern const unsigned char spasm_str[7];
extern const unsigned char spasm_mul[6];
extern const unsigned char spasm_jmp[5];
extern const unsigned char spasm_stp[9];
extern const unsigned char spasm_pri[6];
extern const unsigned char spasm_les[12];
extern const unsigned char spasm_lv[6];
extern const unsigned char spasm_sub[5];
extern const unsigned char spasm_not[5];
extern const unsigned char spasm_lc[5];
extern const unsigned char spasm_la[5];
extern const unsigned char spasm_rea[6];
extern const unsigned char spasm_div[7];
extern const unsigned char spasm_equ[12];

extern const unsigned char spasm_readint32[160];
extern const unsigned char spasm_writeint32[71];
extern const unsigned char spasm_rodata[94];

extern const uint32_t spasm_bss_usage;

#endif

