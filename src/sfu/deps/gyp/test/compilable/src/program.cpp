#include <stdio.h>
#include "lib1.h"

int main(void) {
  fprintf(stdout, "Hello from program.c\n");
  fflush(stdout);
  lib1_function();
  return 0;
}
