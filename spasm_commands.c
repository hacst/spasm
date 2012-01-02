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

#include "spasm_commands.h"

const unsigned char spasm_nop[1] = {
                                        /* spasm_nop: */
    0x90,                               /* nop */
};

const unsigned char spasm_jin[9] = {
                                        /* spasm_jin: */
    0x58,                               /* pop    eax */
    0x21, 0xc0,                         /* and    eax,eax */
    0xf, 0x84, 0xc2, 0x3d, 0xa9, 0xd6,  /* je     deadbeaf <_end+0xd6a92db7> */
};

const unsigned char spasm_add[5] = {
                                        /* spasm_add: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x1, 0xd8,                          /* add    eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_and[5] = {
                                        /* spasm_and: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x21, 0xd8,                         /* and    eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_str[7] = {
                                        /* spasm_str: */
    0x58,                               /* pop    eax */
    0xc1, 0xe0, 0x2,                    /* shl    eax,0x2 */
    0x5b,                               /* pop    ebx */
    0x89, 0x18,                         /* mov    DWORD PTR [eax],ebx */
};


const unsigned char spasm_mul[6] = {
                                        /* spasm_mul: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0xf, 0xaf, 0xc3,                    /* imul   eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_jmp[5] = {
                                        /* spasm_jmp: */
    0xe9, 0xcb, 0x3d, 0xa9, 0xd6,       /* jmp    deadbeaf <_end+0xd6a92db7> */
};

const unsigned char spasm_stp[9] = {
                                        /* spasm_stp: */
    0xb8, 0x1, 0x0, 0x0, 0x0,           /* mov    eax,0x1 */
    0x31, 0xdb,                         /* xor    ebx,ebx */
    0xcd, 0x80,                         /* int    0x80 */
};

const unsigned char spasm_pri[6] = {
                                        /* spasm_pri: */
    0x58,                               /* pop    eax */
    0xe8, 0xd6, 0x3d, 0xa9, 0xd6,       /* call   deadbeaf <_end+0xd6a92db7> */
};

const unsigned char spasm_les[12] = {
                                        /* spasm_les: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x39, 0xd8,                         /* cmp    eax,ebx */
    0x7c, 0x4,                          /* jl     80480a1 <spasm_les.less> */
    0x6a, 0x0,                          /* push   0x0 */
    0xeb, 0x2,                          /* jmp    80480a3 <spasm_les.done> */
                                        /* spasm_les.less: */
    0x6a, 0x1,                          /* push   0x1 */
};

const unsigned char spasm_lv[6] = {
                                        /* spasm_lv: */
    0x58,                               /* pop    eax */
    0xc1, 0xe0, 0x2,                    /* shl    eax,0x2 */
    0xff, 0x30,                         /* push   DWORD PTR [eax] */
};


const unsigned char spasm_sub[5] = {
                                        /* spasm_sub: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x29, 0xd8,                         /* sub    eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_not[5] = {
                                        /* spasm_not: */
    0x58,                               /* pop    eax */
    0x83, 0xf0, 0x1,                    /* xor    eax,0x1 */
    0x50,                               /* push   eax */
};

const unsigned char spasm_lc[5] = {
                                        /* spasm_lc: */
    0x68, 0xaf, 0xbe, 0xad, 0xde,       /* push   0xdeadbeaf */
};

const unsigned char spasm_la[5] = {
                                        /* spasm_la: */
    0x68, 0xaf, 0xbe, 0xad, 0xde,       /* push   0xdeadbeaf */
};

const unsigned char spasm_rea[6] = {
                                        /* spasm_rea: */
    0xe8, 0xd1, 0x3d, 0xa9, 0xd6,       /* call   deadbeaf <_end+0xd6a92db7> */
    0x50,                               /* push   eax */
};

const unsigned char spasm_div[7] = {
                                        /* spasm_div: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x31, 0xd2,                         /* xor    edx,edx */
    0xf7, 0xfb,                         /* idiv   ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_equ[12] = {
                                        /* spasm_equ: */
    0x5b,                               /* pop    ebx */
    0x58,                               /* pop    eax */
    0x39, 0xd8,                         /* cmp    eax,ebx */
    0x74, 0x4,                          /* je     80480b2 <spasm_equ.equal> */
    0x6a, 0x0,                          /* push   0x0 */
    0xeb, 0x2,                          /* jmp    80480b4 <spasm_equ.done> */
                                        /* spasm_equ.equal: */
    0x6a, 0x1,                          /* push   0x1 */
};

const unsigned char spasm_readint32[160] = {
                                        /* readint32: */
    0xb8, 0x4, 0x0, 0x0, 0x0,           /* mov    eax,0x4 */
    0xbb, 0x1, 0x0, 0x0, 0x0,           /* mov    ebx,0x1 */
    0xb9, 0x0, 0x0, 0x0, 0x15,          /* mov    ecx,0x15000000 */
    0xba, 0x2, 0x0, 0x0, 0x0,           /* mov    edx,0x2 */
    0xcd, 0x80,                         /* int    0x80 */
    0xb8, 0x3, 0x0, 0x0, 0x0,           /* mov    eax,0x3 */
    0xbb, 0x0, 0x0, 0x0, 0x0,           /* mov    ebx,0x0 */
    0xb9, 0x0, 0x0, 0x0, 0x25,          /* mov    ecx,0x25000000 */
    0xba, 0xff, 0x0, 0x0, 0x0,          /* mov    edx,0xff */
    0xcd, 0x80,                         /* int    0x80 */
    0x83, 0xf8, 0x1,                    /* cmp    eax,0x1 */
    0x74, 0xcf,                         /* je     1500005e <readint32> */
    0xbe, 0x0, 0x0, 0x0, 0x25,          /* mov    esi,0x25000000 */
    0x1, 0xf0,                          /* add    eax,esi */
    0x31, 0xff,                         /* xor    edi,edi */
    0x80, 0x3e, 0x2d,                   /* cmp    BYTE PTR [esi],0x2d */
    0x75, 0x3,                          /* jne    150000a0 <readint32.positive> */
    0x46,                               /* inc    esi */
    0xf7, 0xd7,                         /* not    edi */
                                        /* readint32.positive: */
    0x31, 0xc0,                         /* xor    eax,eax */
    0x31, 0xdb,                         /* xor    ebx,ebx */
                                        /* readint32.parse: */
    0x8a, 0x1e,                         /* mov    bl,BYTE PTR [esi] */
    0x80, 0xfb, 0xa,                    /* cmp    bl,0xa */
    0x74, 0x4c,                         /* je     150000f7 <readint32.done> */
    0x80, 0xfb, 0x30,                   /* cmp    bl,0x30 */
    0x7c, 0x2c,                         /* jl     150000dc <readint32.errnan> */
    0x80, 0xfb, 0x39,                   /* cmp    bl,0x39 */
    0x7f, 0x27,                         /* jg     150000dc <readint32.errnan> */
    0x80, 0xeb, 0x30,                   /* sub    bl,0x30 */
    0x6b, 0xc0, 0xa,                    /* imul   eax,eax,0xa */
    0x70, 0x7,                          /* jo     150000c4 <readint32.overflow> */
    0x1, 0xd8,                          /* add    eax,ebx */
    0x70, 0x3,                          /* jo     150000c4 <readint32.overflow> */
    0x46,                               /* inc    esi */
    0xeb, 0xe0,                         /* jmp    150000a4 <readint32.parse> */
                                        /* readint32.overflow: */
    0xb8, 0x4, 0x0, 0x0, 0x0,           /* mov    eax,0x4 */
    0xbb, 0x1, 0x0, 0x0, 0x0,           /* mov    ebx,0x1 */
    0xb9, 0x2a, 0x0, 0x0, 0x15,         /* mov    ecx,0x1500002a */
    0xba, 0x34, 0x0, 0x0, 0x0,          /* mov    edx,0x34 */
    0xcd, 0x80,                         /* int    0x80 */
    0xeb, 0x82,                         /* jmp    1500005e <readint32> */
                                        /* readint32.errnan: */
    0xb8, 0x4, 0x0, 0x0, 0x0,           /* mov    eax,0x4 */
    0xbb, 0x1, 0x0, 0x0, 0x0,           /* mov    ebx,0x1 */
    0xb9, 0x2, 0x0, 0x0, 0x15,          /* mov    ecx,0x15000002 */
    0xba, 0x28, 0x0, 0x0, 0x0,          /* mov    edx,0x28 */
    0xcd, 0x80,                         /* int    0x80 */
    0xe9, 0x67, 0xff, 0xff, 0xff,       /* jmp    1500005e <readint32> */
                                        /* readint32.done: */
    0x21, 0xff,                         /* and    edi,edi */
    0x74, 0x2,                          /* je     150000fd <readint32.notnegative> */
    0xf7, 0xd8,                         /* neg    eax */
                                        /* readint32.notnegative: */
    0xc3,                               /* ret     */
};

const unsigned char spasm_writeint32[71] = {
                                        /* writeunsigned: */
    0xbe, 0xfe, 0x0, 0x0, 0x25,         /* mov    esi,0x250000fe */
    0xbb, 0xa, 0x0, 0x0, 0x0,           /* mov    ebx,0xa */
    0xc6, 0x6, 0xa,                     /* mov    BYTE PTR [esi],0xa */
    0x4e,                               /* dec    esi */
    0x31, 0xff,                         /* xor    edi,edi */
    0x83, 0xf8, 0x0,                    /* cmp    eax,0x0 */
    0x7d, 0x4,                          /* jge    1500011c <writeunsigned.generate> */
    0xf7, 0xd7,                         /* not    edi */
    0xf7, 0xd8,                         /* neg    eax */
                                        /* writeunsigned.generate: */
    0x31, 0xd2,                         /* xor    edx,edx */
    0xf7, 0xf3,                         /* div    ebx */
    0x83, 0xc2, 0x30,                   /* add    edx,0x30 */
    0x88, 0x16,                         /* mov    BYTE PTR [esi],dl */
    0x4e,                               /* dec    esi */
    0x83, 0xf8, 0x0,                    /* cmp    eax,0x0 */
    0x75, 0xf1,                         /* jne    1500011c <writeunsigned.generate> */
    0x21, 0xff,                         /* and    edi,edi */
    0x74, 0x4,                          /* je     15000133 <writeunsigned.nominusadd> */
    0xc6, 0x6, 0x2d,                    /* mov    BYTE PTR [esi],0x2d */
    0x4e,                               /* dec    esi */
                                        /* writeunsigned.nominusadd: */
    0x46,                               /* inc    esi */
    0xb8, 0x4, 0x0, 0x0, 0x0,           /* mov    eax,0x4 */
    0xbb, 0x1, 0x0, 0x0, 0x0,           /* mov    ebx,0x1 */
    0x89, 0xf1,                         /* mov    ecx,esi */
    0xba, 0xff, 0x0, 0x0, 0x25,         /* mov    edx,0x250000ff */
    0x29, 0xf2,                         /* sub    edx,esi */
    0xcd, 0x80,                         /* int    0x80 */
    0xc3,                               /* ret     */
};

const unsigned char spasm_rodata[94] =
        "> "
        "Invalid input. Please enter an integer.\n"
        "Number too large. Must be between -/+ (2 ^ 31 - 1).\n";

const uint32_t spasm_bss_usage = 256;

