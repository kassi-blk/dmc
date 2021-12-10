#ifndef CACHE_H_
#include "struct_cache.h"
#include "cache_data.h"

extern struct cache * cache_alloc (void);
extern void cache_print (struct cache *);
extern int cache_test (struct cache *, const struct cache_data *, unsigned *);
extern void cache_dump (const struct cache *);
extern void cache_free (struct cache *);
#define CACHE_H_
#endif
