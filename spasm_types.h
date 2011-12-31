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
#include <stdint.h>
#include <limits.h>

#ifndef SPASM_TYPES_H_
#define SPASM_TYPES_H_

#define MAX_LINE_LENGTH 4096
#define MAX_SYMBOL_NAME_LENGTH 1024
#define MAX_MNEMONIC_LENGTH 3
#define INVALID_VADDR 0
#define INVALID_LINE UINT_MAX

typedef struct MemoryLocation MemoryLocation;
typedef struct Label Label;
typedef struct Command Command;
typedef struct ParserState ParserState;


/**
 * @brief LL-entry for representing memory(/variable) storage in a SPASM application.
 */
struct MemoryLocation
{
    enum MemoryLocationType
    {
        SPASM_BSS,   /* zero initialized memory */
        SPASM_DATA,  /* pre-initialized read/write memory */
        SPASM_RODATA /* pre-initialized read-only memory */
    } type;

    char name[MAX_SYMBOL_NAME_LENGTH]; /* name for this memory location */

    uint32_t size; /* size of this memory location in bytes */
    uint32_t source_line; /* source code line this location was defined at */

    unsigned char *content; /* 0 for BSS */

    uint32_t vaddr; /* absolute location in virtual memory during execution */

    MemoryLocation *next;
};


/**
 * @brief LL-entry for holding a named label pointing to a SPASM Command.
 */
struct Label
{
    char name[MAX_SYMBOL_NAME_LENGTH]; /* name for this label */

    Command *command; /* command this label points to */

    Label *next;
};


/**
 * @brief Enumeration of all command types in the SPASM language.
 */
typedef enum CommandType
{

    /*
     * Unsigned integer operations
     */

    SPASM_ADD, /* push(pop() + pop()) */
    SPASM_MUL, /* push(pop() * pop()) */
    SPASM_SUB, /* a=pop(); b=pop(); push(a > b ? a - b : 0) */
    SPASM_DIV, /* push(pop() / pop()); */

    /*
     * Logic operations
     */

    SPASM_LES, /* push(pop() < pop()) */
    SPASM_AND, /* push(pop() && pop()) */
    SPASM_EQU, /* push(pop() == pop()) */
    SPASM_NOT, /* push(!pop()) */

    /*
     * Memory operations
     */

    SPASM_LA,  /* push(memory_arg->vaddr/4) */
    SPASM_LC,  /* push(constant) */
    SPASM_LV,  /* push(*pop()) */
    SPASM_STR, /* *(pop() * 4) = pop() */

    /*
     * IO operations
     */

    SPASM_PRI, /* print_int(pop()) */
    SPASM_REA, /* push(read_int()) */

    /*
     * Control flow operations
     */

    SPASM_JMP, /* jmp(label_arg->vaddr) */
    SPASM_JIN, /* a = pop(); if (a == 0) jmp(label_arg->vaddr) */
    SPASM_NOP, /* nop */
    SPASM_STP, /* exit() */

    SPASM_RUNTIME_COMMAND_COUNT,
    /* Note: Memory allocation (DS) is not a command that is executed during runtime */
    SPASM_DS
} CommandType;


/**
 * Index in this array equals CommandType
 */
extern const char SPASM_MNEMONICS[][MAX_MNEMONIC_LENGTH + 1];


/**
 * @brief LL-entry for holding a single SPASM application command (e.g. LC 1).
 */
struct Command
{
    CommandType type;

    union CommandArgument
    {
        Label *label_arg;           /* Label argument (e.g. JMP cmds) */
        MemoryLocation *memory_arg; /* Address argument (e.g. LA) */
        uint32_t constant_arg;      /* Constant argument (e.g. LC) */
    } argument; /* Argument of this command. Ignored for commands without argument. */

    Label *label; /* Label pointing to this command, 0 if none */
    uint32_t source_line; /* source code line this location was defined at */

    uint32_t vaddr; /* absolute location of command in virtual memory */

    Command *next;
};


/**
 * @brief Structure for holding and processing a SPASM application AST.
 */
struct ParserState
{
    uint32_t bss_used;
    uint32_t rodata_used;
    uint32_t data_used;

    MemoryLocation *memory_location_first; /* root of memory location list */
    MemoryLocation *memory_location_last;

    Label *label_first; /* root of label list */
    Label *label_last;

    Command *command_first; /* root of command list */
    Command *command_last;

    uint32_t last_line; /* Last source line processed by the parser */
};

typedef int Errc;


/**
 * @brief Enumeration of possible error codes.
 */
enum ErrcCode
{
    ERR_SUCCESS,

    ERR_IO, /* System I/O error */
    ERR_ALLOC, /* Allocation failure */
    ERR_LINE_TO_LONG,
    ERR_SYNTAX,
    ERR_UNDEFINED_VARIABLE,
    ERR_UNDEFINED_LABEL,
    ERR_VARIABLE_REDEFINITION,
    ERR_LABEL_REDEFINITION,
    ERR_CONSTANT_RANGE,
    ERR_INVALID_MNEMONIC,
    ERR_NO_COMMANDS,
    ERR_INTERNAL /* Internal spasm failure */
};


/*
 * @brief Mapping between ErrcCode and string representation
 */
extern const char SPASM_ERR_STR[][128];


#endif /* SPASM_TYPES_H_ */
