#include "reader.h"

object *read(FILE *in){
  int c;
  object_type type;
  char quoted = 0;

  eat_whitespace(in);
  c = getc(in);
  if(c == '\''){
    quoted = 1;
    c = getc(in);
  }
  type = next_type(in, c);
  if(read_funcs[type] != 0){
    if(quoted == 1){
      return cons(quote_symbol, cons((*read_funcs[type])(in), empty_list));
    }else{
      return (*read_funcs[type])(in);
    }
  }else{
    fprintf(stderr, "Unsupported read type\n");
    exit(1);
  }
}

char is_delimiter(int c){
  return isspace(c) || c == EOF ||
         c == '('   || c == ')' ||
         c == '"'   || c == ';';
}
//Looks at the next byte in file, without advancing pointer
int peek(FILE *in){
  int c = getc(in);
  ungetc(c, in);
  return c;
}
//Eats up to a non-whitespace, passes through comments
void eat_whitespace(FILE *in){
  int c;
  while((c = getc(in)) != EOF){
    if(isspace(c)){
      continue;
    }else if(c == ';') {
      //ignore comments
      while(((c = getc(in)) != EOF) && (c != '\n'));
      continue;
    }
    ungetc(c, in);
    break;
  }
}
//Eats only the expected string, exits if that string isn't there
void eat_expected_string(FILE *in, char *str){
  int c;
  while(*str != '\0'){
    c = getc(in);
    if(c != *str){
      fprintf(stderr, "unexpected character: %c\n", c);
      exit(1);
    }
    str++;
  }
}
//Makes sure a delimiter is next in the file, exits if not
void peek_expected_delimiter(FILE *in){
  if(!is_delimiter(peek(in))){
    fprintf(stderr, "error: expected delimiter");
    exit(1);
  }
}
