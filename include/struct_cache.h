#ifndef STRUCT_CACHE_H_
#include <stdbool.h>
#include "const.h"
#include "types.h"

struct cache {
    bool valid;
    unsigned tag : CACHE_TAG_SIZE;
    unsigned num : CACHE_NUM_SIZE;
    unsigned offset : CACHE_OFFSET_SIZE;
    byte data [BYTE_Q]; // Data of entire block of memory
    enum cache_color color;
};
#define STRUCT_CACHE_H_
#endif
