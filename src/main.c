#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <ncurses.h>
#include "utils.h"
#include "cache.h"
#include "mem.h"

#define CUR_H (CACHE_SIZE + MEM_SIZE + 4)

static void redraw (struct cache *, struct mem *, unsigned);
static int parse (char *, struct cache *, struct mem *, unsigned *);

FILE * file_log, * file_data;
char binbuf [BINBUF_SIZE];

static SCREEN * scr;

int
main (void)
{
    struct cache * cache;
    struct mem * mem;
    char * cmd = calloc(256, sizeof(char));
    unsigned tacts = 0, data [MEM_SIZE];
    int i;

    srand(time(NULL));

    cache = cache_alloc();
    mem = mem_alloc();
    scr = newterm(NULL, stdout, stdin);
    file_log = fopen("log", "w");
    file_data = fopen("data", "w");

    for (i = 0; i < MEM_SIZE; i++) {
        data[i] = RAND(0, UINT_MAX - 1);
        mem_push(mem, &data[i]);
    }

    if (has_colors()) {
        start_color();
        init_pair(CACHE_COLOR_NONE, COLOR_WHITE, COLOR_BLACK);
        init_pair(CACHE_COLOR_MISS, COLOR_RED, COLOR_BLACK);
        init_pair(CACHE_COLOR_HIT, COLOR_GREEN, COLOR_BLACK);
        init_pair(MEM_COLOR_SEL, COLOR_YELLOW, COLOR_BLACK);
    }

    cache_print(cache);
    mem_print(mem);
    move(0, getmaxx(stdscr) - 11);
    printw("%10u", tacts);

    mvaddch(CUR_H, 0, '>');
    while (1) {
        mvscanw(CUR_H, 2, "%[^n]s", cmd);
        fprintf(file_log, "%s\n", cmd);
        if (parse(cmd, cache, mem, &tacts) == -1)
            break;
        redraw(cache, mem, tacts);
    }

    endwin();
    delscreen(scr);

    fprintf(file_log, "\n");
    cache_dump(cache);
    fprintf(file_log, "\n");
    mem_dump(mem);
    // TODO  Dump commands here

    fclose(file_log);
    fclose(file_data);
    cache_free(cache);
    mem_free(mem);
    free(cmd);

    return 0;
}

static void
redraw (struct cache * cache, struct mem * mem, unsigned tacts)
{
    erase();
    cache_print(cache);
    mem_print(mem);
    move(0, getmaxx(stdscr) - 11);
    printw("%10u", tacts);
    move(CUR_H, 0);
    printw("> ");
}

static int
parse (char * cmd, struct cache * cache, struct mem * mem, unsigned * tacts)
{
    const char * arg, * delim = " ", * cmd_begin = cmd;
    struct cache_data cd;
    char * endptr;

    arg = strsep(&cmd, delim);
    if (strcmp(arg, "get") == 0) {
        // Parse memory address
        arg = strsep(&cmd, delim);
        if (arg == NULL)
            return 0;

        cd.addr = strtoul(arg, &endptr, 16);
        if (endptr == arg)
            return 0;

        cd.tag = dec(bin(cd.addr, 0, ADDR_SIZE - CACHE_TAG_SIZE));
        cd.idx = dec(bin(cd.addr, ADDR_SIZE - CACHE_TAG_SIZE, CACHE_OFFSET_SIZE));
        cd.offset = dec(bin(cd.addr, CACHE_OFFSET_SIZE, 0));

        // Compare given address with memory block address
        if (mem_cmp(mem, &cd) == -1) {
            printw("Couldn't find block of memory by 0x%x address.\n",
                cd.addr);
            getch();
            return 0;
        }

        if (cache_test(cache, &cd, tacts) == -1) {
            printw("Couldn't find block of cache by %d index.\n", cd.idx);
            getch();
            return -1;
        }
    }
    else if (strcmp(arg, "quit") == 0)
        return -1;
    else {
        printw("get [0x]<addr> "
            "(can add offset (0 <= offset <= sizeof(int)))\n");
        printw("quit\n");
        getch();
    }

    cmd = (char *) cmd_begin;

    return 0;
}
