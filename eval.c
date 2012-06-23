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

object *make_lambda(object *params, object *body){
  return cons(lambda_symbol, cons(params, body));
}

char is_lambda(object *exp){
  return is_tagged_list(exp, lambda_symbol);
}

object *lambda_params(object *exp){
  return cadr(exp);
}

object *lambda_body(object *exp){
  return cddr(exp);
}

/*Begin symbol, like progn*/
object *make_begin(object *exp){
  return cons(begin_symbol, exp);
}

char is_begin(object *exp){
  return is_tagged_list(exp, begin_symbol);
}

object *begin_actions(object *exp){
  return cdr(exp);
}


char is_last_exp(object *seq){
  return is_empty_list(cdr(seq));
}

object *first_exp(object *seq){
  return car(seq);
}

object *rest_exps(object *seq){
  return cdr(seq);
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
  if(is_symbol(cadr(exp))){
    return car(cdr(exp));
  }else{
    return caadr(exp);
  }
}
object *definition_value(object *exp){
  if(is_symbol(cadr(exp))){
    return car(cdr(cdr(exp)));
  }else{
    return make_lambda(cdr(cadr(exp)), cddr(exp));
  }
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
  }else if(is_lambda(exp)){
    return make_compound_proc(lambda_params(exp),
                              lambda_body(exp),
                              env);
  }else if(is_begin(exp)){
    exp = begin_actions(exp);
    while(!is_last_exp(exp)){
      eval(first_exp(exp), env);
      exp = rest_exps(exp);
    }
    exp = first_exp(exp);
    goto tailcall;
  }else if(is_if(exp)){
    exp = is_true(eval(if_predicate(exp), env)) ?
        if_then(exp) :
        if_else(exp);
    /*top of this function, with new exp to eval*/
    goto tailcall;
  }else if(is_proc_call(exp)){
    procedure = eval(proc_symbol(exp), env);
    args = list_of_values(proc_args(exp), env);
    if(is_primitive_proc(procedure)){
      return (procedure->data.primitive_proc.fn)(args);
    }else if(is_compound_proc(procedure)){
      env = extend_environment(
              procedure->data.compound_proc.params,
              args,
              procedure->data.compound_proc.env);
      exp = make_begin(procedure->data.compound_proc.body);
      goto tailcall;
    }else{
      fprintf(stderr, "Unknown procedure type\n");
      exit(1);
    }
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

object *less_than_proc(object *args){
  long val = (car(args))->data.fixnum.value;
  args = cdr(args);

  while(!is_empty_list(args)){
    if(val > (car(args))->data.fixnum.value){
      return false;
    }
    val = (car (args))->data.fixnum.value;
    args = cdr(args);
  }

  return true;
}

object *greater_than_proc(object *args){
  long val = (car(args))->data.fixnum.value;
  args = cdr(args);

  while(!is_empty_list(args)){
    if(val < (car(args))->data.fixnum.value){
      return false;
    }
    val = (car (args))->data.fixnum.value;
    args = cdr(args);
  }

  return true;
}

object *nullp_proc(object *args){
  return is_empty_list(car(args)) ? true : false;
}

object *booleanp_proc(object *args){
  return is_boolean(car(args)) ? true : false;
}
object *symbolp_proc(object *args){
  return is_symbol(car(args)) ? true : false;
}
object *integerp_proc(object *args){
  return is_fixnum(car(args)) ? true : false;
}
object *characterp_proc(object *args){
  return is_character(car(args)) ? true : false;
}
object *pairp_proc(object *args){
  return is_pair(car(args)) ? true : false;
}
object *procedurep_proc(object *args){
  return (is_primitive_proc(car(args)) ||
          is_compound_proc(car(args)))
          ? true : false;
}

object *cons_proc(object *args){
  return cons(car(args), cadr(args));
}

object *car_proc(object *args){
  return caar(args);
}
object *cdr_proc(object *args){
  return cdar(args);
}

object *list_proc(object *args){
  return cons(car(args), cdr(args));
}

object *set_car_proc(object *args){
  set_car(car(args), cadr(args));
  return car(args);
}

object *set_cdr_proc(object *args){
  set_cdr(car(args), cadr(args));
  return car(args);
}

object *char_to_int_proc(object *args){
  object *obj = make_fixnum((long)(car(args))->data.character.value);
}
object *int_to_char_proc(object *args){
  object *obj = make_character((char)(car(args))->data.fixnum.value);
}

object *int_to_string_proc(object *args){
  char buffer[64];
  sprintf(buffer, "%d", (car(args))->data.fixnum.value);
  object *obj = make_string(buffer);
}

object *string_to_int_proc(object *args){
  return make_fixnum(atol((car(args))->data.string.value));
}

object *string_to_symbol_proc(object *args){
  return make_symbol((car(args))->data.string.value);
}
object *symbol_to_string_proc(object *args){
  return make_string((car(args))->data.symbol.value);
}

object *global_env_proc(object *args){
  return global_environment;
}

/*TODO: add:
  Equality testing (polymorphic)
    eq?
*/

void init_environment(void){
  quote_symbol = make_symbol("quote");
  define_symbol = make_symbol("define");
  set_symbol = make_symbol("set!");
  if_symbol = make_symbol("if");
  lambda_symbol = make_symbol("lambda");
  begin_symbol = make_symbol("begin");

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
  define_variable(make_symbol("<"),
                  make_primitive_proc(less_than_proc),
                  global_environment);
  define_variable(make_symbol(">"),
                  make_primitive_proc(greater_than_proc),
                  global_environment);
  define_variable(make_symbol("null?"),
                  make_primitive_proc(nullp_proc),
                  global_environment);
  define_variable(make_symbol("boolean?"),
                  make_primitive_proc(booleanp_proc),
                  global_environment);
  define_variable(make_symbol("symbol?"),
                  make_primitive_proc(symbolp_proc),
                  global_environment);
  define_variable(make_symbol("integer?"),
                  make_primitive_proc(integerp_proc),
                  global_environment);
  define_variable(make_symbol("char?"),
                  make_primitive_proc(characterp_proc),
                  global_environment);
  define_variable(make_symbol("pair?"),
                  make_primitive_proc(pairp_proc),
                  global_environment);
  define_variable(make_symbol("procedure?"),
                  make_primitive_proc(procedurep_proc),
                  global_environment);
  define_variable(make_symbol("cons"),
                  make_primitive_proc(cons_proc),
                  global_environment);
  define_variable(make_symbol("car"),
                  make_primitive_proc(car_proc),
                  global_environment);
  define_variable(make_symbol("cdr"),
                  make_primitive_proc(cdr_proc),
                  global_environment);
  define_variable(make_symbol("list"),
                  make_primitive_proc(list_proc),
                  global_environment);
  define_variable(make_symbol("set-car!"),
                  make_primitive_proc(set_car_proc),
                  global_environment);
  define_variable(make_symbol("set-cdr!"),
                  make_primitive_proc(set_cdr_proc),
                  global_environment);
  define_variable(make_symbol("char->int"),
                  make_primitive_proc(char_to_int_proc),
                  global_environment);
  define_variable(make_symbol("int->char"),
                  make_primitive_proc(int_to_char_proc),
                  global_environment);
  define_variable(make_symbol("int->string"),
                  make_primitive_proc(int_to_string_proc),
                  global_environment);
  define_variable(make_symbol("string->int"),
                  make_primitive_proc(string_to_int_proc),
                  global_environment);
  define_variable(make_symbol("string->symbol"),
                  make_primitive_proc(string_to_symbol_proc),
                  global_environment);
  define_variable(make_symbol("symbol->string"),
                  make_primitive_proc(symbol_to_string_proc),
                  global_environment);
  define_variable(make_symbol("global-env"), 
                  make_primitive_proc(global_env_proc),
                  global_environment);
}

