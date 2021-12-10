#ifndef MEM_H_
#include "struct_mem.h"
#include "cache_data.h"

extern struct mem * mem_alloc (void);
extern int mem_push (struct mem *, unsigned *);
extern void mem_print (struct mem *);
extern int mem_cmp (struct mem *, const struct cache_data *);
extern void mem_dump (const struct mem *);
extern void mem_free (struct mem *);
#define MEM_H_
#endif
