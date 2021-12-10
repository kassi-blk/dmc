#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <ncurses.h>
#include "utils.h"
#include "struct_mem.h"
#include "log.h"
#include "const.h"
#include "cache_data.h"
#include "types.h"

static unsigned cur;

struct mem *
mem_alloc (void)
{
    struct mem * mem;
    int i;

    mem = calloc(MEM_SIZE, sizeof(struct mem));
    for (i = 0; i < MEM_SIZE; i++)
        mem[i].color = MEM_COLOR_NONE;

    return mem;
}

int
mem_push (struct mem * mem, unsigned * data)
{
    if (cur == MEM_SIZE)
        return -1;

    mem[cur].addr = (unsigned) data;
    cur++;

    return 0;
}

void
mem_print (struct mem * mem)
{
    int i, j;

    move(CACHE_SIZE + 3, 0);
    printw("Memory");
    for (i = 0; i < MEM_SIZE; i++) {
        move(CACHE_SIZE + 3 + i, 0);
        attron(COLOR_PAIR(mem[i].color));
        printw("0x%*x ", ADDR_SIZE / CHAR_BIT * 2, mem[i].addr);
        mem[i].color = MEM_COLOR_NONE;
        attron(COLOR_PAIR(mem[i].color));
        for (j = 0; j < BYTE_Q; j++)
            printw("%03u ", *(byte *) (mem[i].addr + j));
    }
}

int
mem_cmp (struct mem * mem, const struct cache_data * cd)
{
    int i;

    for (i = 0; i < MEM_SIZE; i++)
        if (cd->addr - cd->offset == mem[i].addr) {
            mem[i].color = MEM_COLOR_SEL;
            return 0;
        }

    return -1;
}

void
mem_dump (const struct mem * mem)
{
    int i, j;

    fprintf(file_log, "Memory\n\n");
    for (i = 0; i < MEM_SIZE; i++) {
        fprintf(file_log, "0x%*x ", ADDR_SIZE / CHAR_BIT * 2, mem[i].addr);
        for (j = 0; j < BYTE_Q; j++)
            fprintf(file_log, "%03u ", *(byte *) (mem[i].addr + j));
        fprintf(file_log, "\n");
    }
}

void
mem_free (struct mem * mem)
{
    free(mem);
}
