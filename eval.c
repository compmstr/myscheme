#include "eval.h"

char is_self_evaluating(object *exp){
  return is_boolean(exp)   ||
         is_fixnum(exp)    ||
         is_character(exp) ||
         is_string(exp);
}

char is_tagged_list(object *expression, object *tag){
  object *the_car;
  if(is_pair(expression)){
    the_car = car(expression);
    return is_symbol(the_car) && (the_car == tag);
  }
  return 0;
}

char is_quoted(object *expression){
  return is_tagged_list(expression, quote_symbol);
}

object *quoted_contents(object *exp){
  return car(cdr(exp));
}

char is_variable(object *expression){
  return is_symbol(expression);
}

char is_if(object *expression){
  return is_tagged_list(expression, if_symbol);
}

object *if_predicate(object *exp){
  return cadr(exp);
}

object *if_then(object *exp){
  return caddr(exp);
}

object *if_else(object *exp){
  if(is_empty_list(cadddr(exp))){
    return false;
  }else{
    return cadddr(exp);
  }
}

/*Environment stuff */
object *enclosing_environment(object *env){
  return cdr(env);
}

object *first_frame(object *env){
  return car(env);
}

object *make_frame(object *vars, object *vals){
  return cons(vars, vals);
}

object *frame_vars(object *frame){
  return car(frame);
}

object *frame_vals(object *frame){
  return cdr(frame);
}

void add_binding_to_frame(object *var, object *val, object *frame){
  set_car(frame, cons(var, car(frame)));
  set_cdr(frame, cons(val, cdr(frame)));
}

object *extend_environment(object *vars, object *vals, object *base_env){
  return cons(make_frame(vars, vals), base_env);
}

object *lookup_variable_value(object *var, object *env){
  object *frame;
  object *vars;
  object *vals;
  while(!is_empty_list(env)){
    frame = first_frame(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);
    while(!is_empty_list(vars)){
      if(var == car(vars)){
        return car(vals);
      }
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = enclosing_environment(env);
  }
  fprintf(stderr, "Unbound variable: %s\n", var->data.symbol.value);
  exit(1);
}

void set_variable_value(object *var, object *val, object *env){
  object *frame;
  object *vars;
  object *vals;
  while(!is_empty_list(env)){
    frame = first_frame(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);
    while(!is_empty_list(vars)){
      if(var == car(vars)){
        set_car(vals, val);
        return;
      }
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = enclosing_environment(env);
  }
  fprintf(stderr, "Unbound variable: %s\n", var->data.symbol.value);
  exit(1);
}

void define_variable(object *var, object *val, object *env){
  object *frame;
  object *vars;
  object *vals;
  frame = first_frame(env);
  vars = frame_vars(frame);
  vals = frame_vals(frame);
  /*check if the variable exists, and if so, set it*/
  while(!is_empty_list(vars)){
    if(var == car(vars)){
      set_car(vals, val);
      return;
    }
    vars = cdr(vars);
    vals = cdr(vals);
  }
  /*otherwise add a new variable*/
  add_binding_to_frame(var, val, frame);
}

object *setup_environment(void){
  object *initial_env;
  initial_env = extend_environment(
                  empty_list, /*vars*/
                  empty_list, /*vals*/
                  empty_environment);
}

char is_assignment(object *exp){
  return is_tagged_list(exp, set_symbol);
}

object *assignment_variable(object *exp){
  return car(cdr(exp));
}
object *assignment_value(object *exp){
  return car(cdr(cdr(exp)));
}

char is_definition(object *exp){
  return is_tagged_list(exp, define_symbol);
}

object *definition_variable(object *exp){
  return car(cdr(exp));
}
object *definition_value(object *exp){
  return car(cdr(cdr(exp)));
}

object *eval_assignment(object *exp, object *env){
  set_variable_value(assignment_variable(exp),
                      eval(assignment_value(exp), env),
                      env);
  return true;
}

object *eval_definition(object *exp, object *env){
  define_variable(definition_variable(exp),
                      eval(definition_value(exp), env),
                      env);
  return true;
}

char is_proc_call(object *exp){
  return is_pair(exp);
}

object *proc_symbol(object *exp){
  return car(exp);
}

object *proc_args(object *exp){
  return cdr(exp);
}

char is_no_proc_args(object *args){
  return is_empty_list(args);
}

object *first_proc_arg(object *args){
  return car(args);
}
object *rest_proc_args(object *args){
  return cdr(args);
}

object *list_of_values(object *exps, object *env){
  if(is_no_proc_args(exps)){
    return empty_list;
  }else{
    return cons(eval(first_proc_arg(exps), env),
                list_of_values(rest_proc_args(exps), env));
  }
}

object *eval(object *exp, object *env){
  object *procedure;
  object *args;
/*goto target for tail calls*/
tailcall:
  if(is_self_evaluating(exp)){
    return exp;
  }else if(is_variable(exp)){
    return lookup_variable_value(exp, env);
  }else if(is_quoted(exp)){
    return quoted_contents(exp);
  }else if(is_assignment(exp)){
    return eval_assignment(exp, env);
  }else if(is_definition(exp)){
    return eval_definition(exp, env);
  }else if(is_if(exp)){
    exp = is_true(eval(if_predicate(exp), env)) ?
        if_then(exp) :
        if_else(exp);
    /*top of this function, with new exp to eval*/
    goto tailcall;
  }else if(is_proc_call(exp)){
    procedure = eval(proc_symbol(exp), env);
    args = list_of_values(proc_args(exp), env);
    return (procedure->data.primitive_proc.fn)(args);
  }else{
    fprintf(stderr, "Cannot eval unknown expression type\n");
    exit(1);
  }
  fprintf(stderr, "eval illegal state\n");
  exit(1);
}

/*Builin Procedures*/
/*builtin procedure for addition*/
object *plus_proc(object *arguments){
  long result = 0;

  while(!is_empty_list(arguments)){
    result += (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

/*builtin procedure for subtraction*/
object *minus_proc(object *arguments){
  long result = (car(arguments))->data.fixnum.value;
  arguments = cdr(arguments);

  while(!is_empty_list(arguments)){
    result -= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}
/*builtin procedure for multiplication*/
object *mult_proc(object *arguments){
  long result = 1;

  while(!is_empty_list(arguments)){
    result *= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}
/*builtin procedure for division*/
object *div_proc(object *arguments){
  long result = (car(arguments))->data.fixnum.value;
  arguments = cdr(arguments);

  while(!is_empty_list(arguments)){
    result /= (car(arguments))->data.fixnum.value;
    arguments = cdr(arguments);
  }
  return make_fixnum(result);
}

object *remainder_proc(object *args){
  long result = (car(args))->data.fixnum.value;
  long remainder = 0;
  args = cdr(args);

  while(!is_empty_list(args)){
    remainder = result % (car(args))->data.fixnum.value;
    result /= (car(args))->data.fixnum.value;
    args = cdr(args);
  }
  return make_fixnum(remainder);
}

object *num_equal_proc(object *args){
  long val = (car(args))->data.fixnum.value;
  args = cdr(args);

  while(!is_empty_list(args)){
    if(val != (car(args))->data.fixnum.value){
      return false;
    }
    args = cdr(args);
  }

  return true;
}

/*TODO: add:
  Type predicates:
    null?
    boolean?
    symbol?
    integer?
    char?
    pair?
    procedure?
  Type conversions:
    char->integer
    integer->char
    number->string
    string->number
    symbol->string
    string->symbol
  Math:
    >
    <
  List ops:
    cons
    car
    cdr
    set-car!
    set-cdr!
    list
  Equality testing (polymorphic)
    eq?
*/

void init_environment(void){
  quote_symbol = make_symbol("quote");
  define_symbol = make_symbol("define");
  set_symbol = make_symbol("set!");
  if_symbol = make_symbol("if");

  empty_environment = empty_list;

  global_environment = setup_environment();
  
  /*Set up builtin procedures*/
  define_variable(make_symbol("+"),
                  make_primitive_proc(plus_proc),
                  global_environment);
  define_variable(make_symbol("-"),
                  make_primitive_proc(minus_proc),
                  global_environment);
  define_variable(make_symbol("*"),
                  make_primitive_proc(mult_proc),
                  global_environment);
  define_variable(make_symbol("/"),
                  make_primitive_proc(div_proc),
                  global_environment);
  define_variable(make_symbol("mod"),
                  make_primitive_proc(remainder_proc),
                  global_environment);
  define_variable(make_symbol("="),
                  make_primitive_proc(num_equal_proc),
                  global_environment);
}

