#ifndef CACHE_CONST_H_
enum endian {
    ENDIAN_LITTLE,
    ENDIAN_BIG
};

enum cache_color {
    CACHE_COLOR_NONE = 1,
    CACHE_COLOR_MISS,
    CACHE_COLOR_HIT,
    CACHE_COLOR_OFFSET
};

enum mem_color {
    MEM_COLOR_NONE = 1,
    MEM_COLOR_SEL = 4
};

#define CACHE_SIZE 8
#define CACHE_TAG_SIZE 27
#define CACHE_NUM_SIZE 3
#define CACHE_OFFSET_SIZE 2
#define BYTE_Q 4
#define ADDR_SIZE 32
#define BINBUF_SIZE (ADDR_SIZE + 1)
#define TACT_Q_MISS 40
#define TACT_Q_READ 2
#define ENDIAN ENDIAN_BIG
#define MEM_SIZE 16

#if CACHE_TAG_SIZE + CACHE_NUM_SIZE + CACHE_OFFSET_SIZE != ADDR_SIZE
#error "Cache bit quantity is not equal to ADDR_SIZE"
#endif
#define CACHE_CONST_H_
#endif
