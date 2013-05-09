#ifndef _EVAL_H_
#define _EVAL_H_

#include "types.h"

#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cddr(obj) cdr(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define caadr(obj) car(car(cdr(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))

extern object *quote_symbol;
extern object *define_symbol;
extern object *set_symbol;
extern object *if_symbol;
extern object *lambda_symbol;
extern object *begin_symbol;
extern object *let_symbol;
extern object *while_symbol;

extern object *empty_environment;
extern object *global_environment;

object *scheme_eval(object *exp, object *env);
void init_environment(void);

#endif
