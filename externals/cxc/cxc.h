
#include <stdio.h>

#ifndef L
#define L fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
#endif
