#ifndef _TYPES_H_
#define _TYPES_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {FIXNUM, CHARACTER, BOOLEAN, EMPTY_LIST, 
              STRING, SYMBOL, PAIR, PRIMITIVE_PROC,
              COMPOUND_PROC, NUM_TYPES} object_type;

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
    struct {
      struct object *(*fn)(struct object *args);
    }primitive_proc;
    struct{
      struct object *parameters;
      struct object *body;
      struct object*env;
    }compound_proc;
  } data;
} object;

object *true;
object *false;
object *empty_list;
object *symbol_table;

object *car(object *obj);
object *cdr(object *obj);
object *cons(object *car, object *cdr);

char is_false(object *obj);
char is_true(object *obj);

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

char is_quoted(object *expression);
char is_empty_list(object *obj);
object *quoted_contents(object *exp);
char is_self_evaluating(object *exp);

void set_cdr(object *obj, object *value);
void set_car(object *obj, object *value);

char is_boolean(object *);
char is_symbol(object *);
char is_fixnum(object *);
char is_character(object *);
char is_pair(object *);
char is_primitive_proc(object *);
char is_compound_proc(object *);

void init_types(void);

object *make_symbol(const char *);
object *make_primitive_proc(object *(*fn)(struct object *arguments));
object *make_fixnum(const long);
object *make_string(const char *);
object *make_character(const char);

#endif
