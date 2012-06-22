#ifndef _READER_H__
#define _READER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "eval.h"

object *read(FILE *in);

char is_delimiter(int c);
/*Looks at the next byte in file, without advancing pointer*/
int peek(FILE *in);
/*Eats up to a non-whitespace, passes through comments*/
void eat_whitespace(FILE *in);
/*Eats only the expected string, exits if that string isn't there*/
void eat_expected_string(FILE *in, char *str);
/*Makes sure a delimiter is next in the file, exits if not*/
void peek_expected_delimiter(FILE *in);

#endif
