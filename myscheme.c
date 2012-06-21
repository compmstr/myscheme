#include<stdio.h>
#include "types.h"
#include "reader.h"
#include "writer.h"
#include "eval.h"

int main(int argc, char **argv){
  printf("Welcome to my scheme.\nUse ctrl-c to exit.\n");

  init_types();
  while(1){
    printf("> ");
    write(eval(read(stdin)));
    printf("\n");
  }

  return 0;
}
