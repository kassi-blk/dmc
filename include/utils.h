#ifndef UTILS_H_
#include <stdlib.h>

extern const char * bin (unsigned, unsigned, unsigned);
extern unsigned dec (const char *);

#define RAND(min, max) ((min) + rand() % ((max) - (min) + 1))
#define UTILS_H_
#endif
