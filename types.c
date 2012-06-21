#include "types.h"
#include "reader.h"

object *alloc_object(void){
  object *obj;

  obj = malloc(sizeof(object));
  if(obj == NULL){
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  return obj;
}

/*Used to determine what type is coming next in the stream
 *Inputs:
 *  in - the stream to read from
 *  c - the first non-whitespace character to look at*/
object_type next_type(FILE *in, char c){
  int next_char;
  if(c == EOF){
    printf("Got EOF, exiting\n");
    exit(0);
  }
  if(c == '#'){ /*a boolean or character*/
    next_char = peek(in);
    switch(next_char){
      case 't':
      case 'f':
        return BOOLEAN;
      case '\\':
        return CHARACTER;
      default:
        fprintf(stderr,
            "Unknown boolean or character literal\n");
        exit(1);
    }
  }else if (isdigit(c) || (c == '-' && isdigit(peek(in)))){
    ungetc(c, in);
    return FIXNUM;
  }else if(c == '('){
    if(peek(in) == ')'){
      return EMPTY_LIST;
    }else{
      return PAIR;
    }
  }else if(c == '"'){
    return STRING;
  }else{
    fprintf(stderr, "Bad input. Unexpected '%c'\n", c);
    exit(1);
  }
  fprintf(stderr, "No object type, no error... wtf?\n");
  exit(1);
}

char is_false(object *obj){
  return obj == false;
}

char is_true(object *obj){
  return !is_false(obj);
}

//Boolean
//no make_boolean because true and false are constants to be reused
char is_boolean(object *obj){
  return obj->type == BOOLEAN;
}

void write_boolean(object *obj){
  printf("#%c", is_false(obj) ? 'f' : 't');
}

object *read_boolean(FILE *in){
  int c = getc(in);
  if(c == 't'){
    return true;
  }else if(c == 'f'){
    return false;
  }else{
    fprintf(stderr, "Invalid boolean, expected [t|f], got %c\n", c);
    exit(1);
  }
}

//Fixnum
object *make_fixnum(long value){
  object *obj;

  obj = alloc_object();
  obj->type = FIXNUM;
  obj->data.fixnum.value = value;
  return obj;
}

char is_fixnum(object *obj){
  return obj->type == FIXNUM;
}

object *read_fixnum(FILE *in){
  int c = getc(in);
  long num = 0;
  short sign = 1;
  if(c == '-'){
    sign = -1;
  }else{
    ungetc(c, in);
  }
  while(isdigit(c = getc(in))){
    num = (num * 10) + (c - '0');
  }
  num *= sign;
  if(is_delimiter(c)){
    ungetc(c, in);
    return make_fixnum(num);
  }else{
    fprintf(stderr, "number not followed by a delimiter");
    exit(1);
  }
}

void write_fixnum(object *obj){
  printf("%ld", obj->data.fixnum.value);
}

//Character
object *make_character(char value){
  object *obj;

  obj = alloc_object();
  obj->type = CHARACTER;
  obj->data.character.value = value;
  return obj;
}

char is_character(object *obj){
  return obj->type == CHARACTER;
}

object *read_character(FILE *in){
  int c;
  c = getc(in); /* / */
  if(c != '\\'){
    fprintf(stderr, "Invalid character, expected '\\', got %c\n", c);
  }
  c = getc(in); /* <character> */
  switch(c){
    case EOF:
      fprintf(stderr, "Incomplete character literal\n");
      exit(1);
    case 's':
      if(peek(in) == 'p'){
        eat_expected_string(in, "pace");
        peek_expected_delimiter(in);
        return make_character(' ');
      }
      break;
    case 'n':
      if(peek(in) == 'e'){
        eat_expected_string(in, "ewline");
        peek_expected_delimiter(in);
        return make_character(' ');
      }
      break;
  }
  /* make sure the character is followed by a delimiter */
  peek_expected_delimiter(in);
  return make_character(c);
}

void write_character(object *obj){
  //Storing this, may do special chars differently
  char c = obj->data.character.value;
  printf("#\\");
  switch(c){
    default:
      putchar(c);
  }
}

//Strings
object *make_string(char *value){
  object *obj;

  obj = alloc_object();
  obj->type = STRING;
  obj->data.string.value = malloc(strlen(value) + 1);
  if(obj->data.string.value == NULL){
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  strcpy(obj->data.string.value, value);
  return obj;
}

char is_string(object *obj){
  return obj->type == STRING;
}

object *read_string(FILE *in){
  const int STRING_BUFFER_LEN = 1024;
  char buffer[STRING_BUFFER_LEN];
  int i = 0;
  int c;
  while((c = getc(in)) != '"'){
    if(c == '\\'){
      c = getc(in);
      switch(c){
        case 'n':
          c = '\n';
          break;
        case '"':
          c = '"';
          break;
        case '\\':
          c = '\\';
          break;
        default:
          fprintf(stderr, "Unsupported escape, \%c", c);
          exit(1);
      }
    }
    if(i < STRING_BUFFER_LEN - 1){
      buffer[i] = c;
      i++;
    }else{
      fprintf(stderr, "String to long, max is %d", STRING_BUFFER_LEN);
      exit(1);
    }
  }
  buffer[i] = '\0';
  return make_string(buffer);
}

void write_string(object *obj){
  printf("\"%s\"", obj->data.string.value);
}

//Empty list
object *read_empty_list(FILE *in){
  int c = getc(in);
  if(c != ')'){
    fprintf(stderr, "Invalid empty list");
    exit(1);
  }
  return empty_list;
}
void write_empty_list(object *obj){
  printf("()");
}

//Pair/list
object *read_pair(FILE *in){
  int c;
  object *car_obj;
  object *cdr_obj;

  eat_whitespace(in);
  c = getc(in);
  if(c == ')'){
    return empty_list;
  }
  ungetc(c, in);

  car_obj = read(in);

  eat_whitespace(in);

  c = getc(in);
  if(c == '.'){
    c = peek(in);
    if(!is_delimiter(c)){
      fprintf(stderr, "dot not followed by delimiter");
      exit(1);
    }
    cdr_obj = read(in);
    eat_whitespace(in);
    c = getc(in);
    if(c != ')'){
      fprintf(stderr, "Where's the trailing paren?");
      exit(1);
    }
  }else{ /*read a list*/
    ungetc(c, in);
    cdr_obj = read_pair(in);
  }
  return cons(car_obj, cdr_obj);
}

void write_pair_recurse(object *obj, char open_paren);
void write_pair(object *obj){
  write_pair_recurse(obj, 1);
}

void write_pair_recurse(object *obj, char open_paren){
  object *car_obj;
  object *cdr_obj;

  if(open_paren){
    printf("(");
  }

  car_obj = car(obj);
  cdr_obj = cdr(obj);
  write(car_obj);
  if(cdr_obj->type == PAIR){
    printf(" ");
    write_pair_recurse(cdr_obj, 0);
  }else if(cdr_obj->type == EMPTY_LIST){
    printf(")");
  }else{
    printf(" . ");
    write(cdr_obj);
    printf(")");
  }
}

object *car(object *obj){
  if(obj->type != PAIR){
    return obj;
  }else{
    return obj->data.pair.car;
  }
}

object *cdr(object *obj){
  if(obj->type != PAIR){
    return empty_list;
  }else{
    return obj->data.pair.cdr;
  }
}

object *cons(object *car, object *cdr){
  object *obj = alloc_object();

  obj->type = PAIR;
  obj->data.pair.car = car;
  obj->data.pair.cdr = cdr;
  return obj;
}

char is_pair(object *obj){
  return obj->type == PAIR;
}

void set_car(object *obj, object *value){
  obj->data.pair.car = value;
}

void set_cdr(object *obj, object *value){
  obj->data.pair.cdr = value;
}

//Initialization
void init_types(void){
  printf("Initializing types...\n");
  false = alloc_object();
  false->type = BOOLEAN;
  false->data.boolean.value = 0;

  true = alloc_object();
  true->type = BOOLEAN;
  true->data.boolean.value = 1;

  empty_list = alloc_object();
  empty_list->type = EMPTY_LIST;

  //set up reader function pointers
  //set up writer function pointers
  read_funcs[FIXNUM] = read_fixnum;
  write_funcs[FIXNUM] = write_fixnum;

  read_funcs[CHARACTER] = read_character;
  write_funcs[CHARACTER] = write_character;

  read_funcs[STRING] = read_string;
  write_funcs[STRING] = write_string;

  read_funcs[BOOLEAN] = read_boolean;
  write_funcs[BOOLEAN] = write_boolean;

  read_funcs[EMPTY_LIST] = read_empty_list;
  write_funcs[EMPTY_LIST] = write_empty_list;

  read_funcs[PAIR] = read_pair;
  write_funcs[PAIR] = write_pair;
}
