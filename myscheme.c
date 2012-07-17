#include<stdio.h>
#include "types.h"
#include "reader.h"
#include "writer.h"
#include "eval.h"

int main(int argc, char **argv){
  printf("Welcome to my scheme.\nUse ctrl-c to exit.\n");

  init_types();
  init_environment();
  while(1){
    printf("> ");
    write(eval(scheme_read(stdin), global_environment));
    printf("\n");
  }

  return 0;
}
