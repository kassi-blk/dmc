#ifndef CACHE_DATA_H_
struct cache_data {
    unsigned addr;
    unsigned tag;
    unsigned idx;
    unsigned offset;
};
#define CACHE_DATA_H_
#endif
