/**
* @file Retrieves the next token from a string.
*/ 

#ifndef _NEXTTOKEN_H_
#define _NEXTTOKEN_H_

#include <sys/types.h>

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * @returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim);

#endif