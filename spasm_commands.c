#include "spasm_commands.h"

const unsigned char spasm_nop[] = {
                                        /* spasm_nop: */
    0x90,                               /* nop */
};

const unsigned char spasm_jin[] = {
                                        /* spasm_jin: */
    0x58,                               /* pop    eax */
    0x21, 0xc0,                         /* and    eax,eax */
    0xf, 0x84, 0xf6, 0x3d, 0xa9, 0xd6,  /* je     deadbeaf <_end+0xd6a92deb> */
};

const unsigned char spasm_add[] = {
                                        /* spasm_add: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x1, 0xd8,                          /* add    eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_and[] = {
                                        /* spasm_and: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x21, 0xd8,                         /* and    eax,ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_str[] = {
                                        /* spasm_str: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x89, 0x18,                         /* mov    DWORD PTR [eax],ebx */
};

const unsigned char spasm_mul[] = {
                                        /* spasm_mul: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0xf7, 0xe3,                         /* mul    ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_jmp[] = {
                                        /* spasm_jmp: */
    0xe9, 0xff, 0x3d, 0xa9, 0xd6,       /* jmp    deadbeaf <_end+0xd6a92deb> */
};

const unsigned char spasm_stp[] = {
                                        /* spasm_stop: */
    0xb8, 0x1, 0x0, 0x0, 0x0,           /* mov    eax,0x1 */
    0x31, 0xdb,                         /* xor    ebx,ebx */
    0xcd, 0x80,                         /* int    0x80 */
};

const unsigned char spasm_pri[] = {
                                        /* spasm_pri: */
    0x58,                               /* pop    eax */
    0xe8, 0xa, 0x3e, 0xa9, 0xd6,        /* call   deadbeaf <_end+0xd6a92deb> */
};

const unsigned char spasm_les[] = {
                                        /* spasm_les: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x39, 0xd8,                         /* cmp    eax,ebx */
    0x7c, 0x4,                          /* jl     8048076 <spasm_les.less> */
    0x6a, 0x0,                          /* push   0x0 */
    0xeb, 0x2,                          /* jmp    8048078 <spasm_les.done> */
                                        /* spasm_les.less: */
    0x6a, 0x1,                          /* push   0x1 */
};

const unsigned char spasm_lv[] = {
                                        /* spasm_lv: */
    0x58,                               /* pop    eax */
    0xff, 0x30,                         /* push   DWORD PTR [eax] */
};

const unsigned char spasm_sub[] = {
                                        /* spasm_sub: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x29, 0xd8,                         /* sub    eax,ebx */
    0x7d, 0x2,                          /* jge    8048066 <spasm_sub.done> */
    0x31, 0xc0,                         /* xor    eax,eax */
                                        /* spasm_sub.done: */
    0x50,                               /* push   eax */
};

const unsigned char spasm_not[] = {
                                        /* spasm_not: */
    0x58,                               /* pop    eax */
    0x83, 0xf0, 0x1,                    /* xor    eax,0x1 */
    0x50,                               /* push   eax */
};

const unsigned char spasm_lc[] = {
                                        /* spasm_lc: */
    0x68, 0xaf, 0xbe, 0xad, 0xde,       /* push   0xdeadbeaf */
};

const unsigned char spasm_la[] = {
                                        /* spasm_la: */
    0x68, 0xaf, 0xbe, 0xad, 0xde,       /* push   0xdeadbeaf */
};

const unsigned char spasm_rea[] = {
                                        /* spasm_rea: */
    0xe8, 0x5, 0x3e, 0xa9, 0xd6,        /* call   deadbeaf <_end+0xd6a92deb> */
    0x50,                               /* push   eax */
};

const unsigned char spasm_div[] = {
                                        /* spasm_div: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0xf7, 0xf3,                         /* div    ebx */
    0x50,                               /* push   eax */
};

const unsigned char spasm_equ[] = {
                                        /* spasm_equ: */
    0x58,                               /* pop    eax */
    0x5b,                               /* pop    ebx */
    0x39, 0xd8,                         /* cmp    eax,ebx */
    0x74, 0x4,                          /* je     8048087 <spasm_equ.equal> */
    0x6a, 0x0,                          /* push   0x0 */
    0xeb, 0x2,                          /* jmp    8048089 <spasm_equ.done> */
                                        /* spasm_equ.equal: */
    0x6a, 0x1,                          /* push   0x1 */
};


