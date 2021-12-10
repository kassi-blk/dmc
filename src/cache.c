#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>
#include "utils.h"
#include "struct_cache.h"
#include "cache_data.h"
#include "log.h"
#include "binbuf.h"

struct cache *
cache_alloc (void)
{
    struct cache * cache;
    int i;

    cache = calloc(CACHE_SIZE, sizeof(struct cache));
    for (i = 0; i < CACHE_SIZE; i++) {
        cache[i].num = i;
        cache[i].color = CACHE_COLOR_NONE;
    }

    return cache;
}

void
cache_print (struct cache * cache)
{
    unsigned i, j;

    move(0, 0);
    printw("Cache");
    for (i = 0; i < CACHE_SIZE; i++) {
        move(i + 2, 0);
        attron(COLOR_PAIR(cache[i].color));
        printw("%s ", bin(cache[i].num, CACHE_NUM_SIZE, 0));
        cache[i].color = CACHE_COLOR_NONE;
        attron(COLOR_PAIR(cache[i].color));
        printw("%d ", cache[i].valid);
        printw("%s ", bin(cache[i].tag, CACHE_TAG_SIZE, 0));
        printw("%s ", bin(cache[i].offset, CACHE_OFFSET_SIZE, 0));
        for (j = 0; j < BYTE_Q; j++)
            if (j == cache[i].offset && cache[i].valid) {
                cache[i].color = CACHE_COLOR_OFFSET;
                attron(COLOR_PAIR(cache[i].color));
                printw("%03u ", cache[i].data[j]);
                cache[i].color = CACHE_COLOR_NONE;
                attron(COLOR_PAIR(cache[i].color));
            }
            else
                printw("%03u ", cache[i].data[j]);
    }
}

int
cache_test (struct cache * cache, const struct cache_data * cd,
            unsigned * tacts)
{
    struct cache * row;
    int i;
    bool is_found = false;

    // Search cache block by index
    for (i = 0; i < CACHE_SIZE; i++)
        if (cache[i].num == cd->idx) {
            row = &cache[i];
            is_found = true;
            break;
        }
    if (!is_found)
        return -1;

    if (row->tag == cd->tag)
        // Hit
        row->color = CACHE_COLOR_HIT;
    else {
        // Miss
        row->color = CACHE_COLOR_MISS;
        row->valid = true;
        row->tag = cd->tag;

        for (i = 0; i < BYTE_Q; i++)
            row->data[ENDIAN == ENDIAN_LITTLE ? BYTE_Q - 1 - i : i]
                = *(byte *) (cd->addr + i);

        *tacts += TACT_Q_MISS;
    }

    row->offset = cd->offset;

    fprintf(file_data, "0x%*x %u %03d\n", ADDR_SIZE / CHAR_BIT * 2,
        cd->addr - cd->offset, cd->offset, row->data[cd->offset]);

    *tacts += TACT_Q_READ;

    return 0;
}

void
cache_dump (const struct cache * cache)
{
    unsigned i, j;

    fprintf(file_log, "Cache\n\n");
    for (i = 0; i < CACHE_SIZE; i++) {
        fprintf(file_log, "%s ", bin(cache[i].num, CACHE_NUM_SIZE, 0));
        fprintf(file_log, "%d ", cache[i].valid);
        fprintf(file_log, "%s ", bin(cache[i].tag, CACHE_TAG_SIZE, 0));
        fprintf(file_log, "%s ", bin(cache[i].offset, CACHE_OFFSET_SIZE, 0));
        for (j = 0; j < BYTE_Q; j++)
            fprintf(file_log, "%03u ", cache[i].data[j]);
        fprintf(file_log, "\n");
    }
}

void
cache_free (struct cache * cache)
{
    free(cache);
}
