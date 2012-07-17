#ifndef _EVAL_H__
#define _EVAL_H_

#include "types.h"

#define caar(obj) car(car(obj))
#define cadr(obj) car(cdr(obj))
#define cddr(obj) cdr(cdr(obj))
#define cdar(obj) cdr(car(obj))
#define caadr(obj) car(car(cdr(obj)))
#define caddr(obj) car(cdr(cdr(obj)))
#define cadddr(obj) car(cdr(cdr(cdr(obj))))

object *quote_symbol;
object *define_symbol;
object *set_symbol;
object *if_symbol;
object *lambda_symbol;
object *begin_symbol;
object *let_symbol;

object *empty_environment;
object *global_environment;

object *eval(object *exp, object *env);
void init_environment(void);

#endif
