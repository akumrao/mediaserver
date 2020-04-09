#include <stdio.h>
#include "lib1.h"

void lib1_function(void) {
  fprintf(stdout, "Hello from lib1.c\n");
  fflush(stdout);
}
