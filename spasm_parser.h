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
#include <stdio.h>

#include "spasm_types.h"

#ifndef SPASM_PARSER_H_
#define SPASM_PARSER_H_

/*
 * @brief Prepares parser for parse_file call.
 * @param parser ParserState to initialize
 */
void init_parser(ParserState *parser);

/**
 * @brief Parses the given file updating the given ParserState.
 * @note Make sure to always release the memory allocated in the parser structure
 *       using cleanup_parser even if the parse_file call failed.
 *
 * @param parser ParserState to update
 * @param file File to parse
 * @return Error @see ErrcCode
 */
Errc parse_file(ParserState *parser, FILE *file);

/**
 * @brief Releases all memory held in the ParserState and resets it.
 * @parser ParserState to reset.
 */
void cleanup_parser(ParserState *parser);

#endif /* SPASM_PARSER_H_ */
