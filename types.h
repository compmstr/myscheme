#ifndef _TYPES_H_
#define _TYPES_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {FIXNUM, CHARACTER, BOOLEAN, EMPTY_LIST, 
              STRING, SYMBOL, PAIR, NUM_TYPES} object_type;

typedef struct object {
  object_type type;
  union{
    struct {
      char value;
    }boolean;
    struct {
      char value;
    }character;
    struct {
      char *value;
    }string;
    struct {
      long value;
    }fixnum;
    struct {
      struct object *car;
      struct object *cdr;
    }pair;
    struct {
      char *value;
    }symbol;
  } data;
} object;

object *true;
object *false;
object *empty_list;
object *symbol_table;

object *car(object *obj);
object *cdr(object *obj);
object *cons(object *car, object *cdr);

/**
 *List of read functions for each type
 * these are initialized in init_types
 */
object * (*read_funcs[NUM_TYPES])(FILE *in);
/**
 *List of write functions for each type
 * these are initialized in init_types
 */
void (*write_funcs[NUM_TYPES])(object *obj);

/*Used to determine what type is coming next in the stream
 *Inputs:
 *  in - the stream to read from
 *  c - the first non-whitespace character to look at*/
object_type next_type(FILE *in, char c);

void init_types(void);

#endif
