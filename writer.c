#include "writer.h"

void write(object *obj){
  if(obj->type < NUM_TYPES){
    (*write_funcs[obj->type])(obj);
  }else{
    fprintf(stderr, "Invalid type passed to write\n");
    exit(1);
  }
}
