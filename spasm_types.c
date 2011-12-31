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

#include "spasm_types.h"

const char SPASM_MNEMONICS[][MAX_MNEMONIC_LENGTH + 1] = {
        "ADD",
        "MUL",
        "SUB",
        "DIV",

        "LES",
        "AND",
        "EQU",
        "NOT",

        "LA",
        "LC",
        "LV",
        "STR",

        "PRI",
        "REA",

        "JMP",
        "JIN",
        "NOP",
        "STP",

        "",
        "DS"
};

const char SPASM_ERR_STR[][128] = {
        "ERR_SUCCESS",
        "ERR_IO",
        "ERR_ALLOC",
        "ERR_LINE_TO_LONG",
        "ERR_SYNTAX",
        "ERR_UNDEFINED_VARIABLE",
        "ERR_UNDEFINED_LABEL",
        "ERR_VARIABLE_REDEFINITION",
        "ERR_LABEL_REDEFINITION",
        "ERR_CONSTANT_RANGE",
        "ERR_INVALID_MNEMONIC",
        "ERR_NO_COMMANDS",
        "ERR_INTERNAL",
};
