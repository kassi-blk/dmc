#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <limits.h>
#include <math.h>
#include <time.h>

enum endian {
    ENDIAN_LITTLE,
    ENDIAN_BIG
};

#define ADDR_SIZE 32
#define CACHE_SIZE 8
#define CACHE_TAG_SIZE 27
#define CACHE_NUM_SIZE 3
#define CACHE_OFFSET_SIZE 2
#define MEM_SIZE 16
#define MEM_COL_H 8
#define MEM_COL_X_OFFSET 23
#define BYTE_Q 4
#define CUR_H (CACHE_SIZE + MEM_COL_H + 6)
#define ENDIAN ENDIAN_LITTLE
#define TACT_Q_MISS 40
#define TACT_Q_READ 2

#if CACHE_TAG_SIZE + CACHE_NUM_SIZE + CACHE_OFFSET_SIZE != ADDR_SIZE
#error "A cache's bit quantity isn't equal to ADDR_SIZE"
#endif

#define RAND(min, max) ((min) + rand() % ((max) - (min) + 1))
#define GET_TAG(addr) (dec(bin((addr), ADDR_SIZE, ADDR_SIZE - CACHE_TAG_SIZE)))
#define GET_INDEX(addr) (dec(bin((addr), ADDR_SIZE - CACHE_TAG_SIZE, CACHE_OFFSET_SIZE)))
#define GET_OFFSET(addr) (dec(bin((addr), CACHE_OFFSET_SIZE, 0)))

enum test {
    TEST_NONE = 1,
    TEST_MISS,
    TEST_HIT
};

enum mem_color {
    MEM_COLOR_NONE = 1,
    MEM_COLOR_SEL = 4
};

typedef unsigned char byte;

struct cache {
    bool valid;
    unsigned tag : CACHE_TAG_SIZE;
    unsigned num : CACHE_NUM_SIZE;
    unsigned offset : CACHE_OFFSET_SIZE;
    byte data [BYTE_Q]; /* Data of an entire block of the memory. */
    enum test testres;
};

struct mem {
    unsigned addr;
    unsigned data;
    enum mem_color color;
};

static void cache_init (void);
static void cache_print (void);
static void cache_dump (void);
static void mem_init (void);
static void mem_print (void);
static void mem_dump (void);
static void redraw (void);
static void cmd_parse (void);
static void swap (char * arr, unsigned isrc, unsigned idst);
static const char * bin (unsigned val, unsigned len, unsigned shift);
static unsigned dec (const char * val);
static void assert (const char * str);

static char * cmd;
static char binbuf [ADDR_SIZE];
static SCREEN * scr;
static struct cache * cache;
static struct mem * mem;
static char errbuf [256];
static FILE * log_file;
static FILE * data_file;
static unsigned data [MEM_SIZE];
static unsigned tacts;

int
main (void)
{
    srand(time(NULL));

    cache_init();
    mem_init();
    scr = newterm(NULL, stdout, stdin);
    log_file = fopen("log", "w");
    data_file = fopen("data", "w");
    cmd = calloc(sizeof(char), 256);

    if (has_colors()) {
        start_color();

        init_pair(TEST_NONE, COLOR_WHITE, COLOR_BLACK);
        init_pair(TEST_MISS, COLOR_RED, COLOR_BLACK);
        init_pair(TEST_HIT, COLOR_GREEN, COLOR_BLACK);
        init_pair(MEM_COLOR_SEL, COLOR_YELLOW, COLOR_BLACK);
    }

    cache_print();
    mem_print();

    move(0, getmaxx(stdscr) - 11);
    printw("%10u", tacts);

    mvaddch(CUR_H, 0, '>');

    while (strcmp(cmd, "quit")) {
        (void) mvscanw(CUR_H, 2, "%[^n]s", cmd);

        fprintf(log_file, "%s\n", cmd);

        cmd_parse();
        redraw();
    }

    endwin();
    delscreen(scr);

    fprintf(log_file, "\n");
    cache_dump();
    fprintf(log_file, "\n");
    mem_dump();
    fprintf(log_file, "\n");
    fclose(log_file);

    fprintf(data_file, "\n");
    fclose(data_file);

    free(cache);
    free(mem);
    free(cmd);

    return 0;
}

static void
cache_init (void)
{
    int i;

    cache = calloc(sizeof(struct cache), CACHE_SIZE);
    for (i = 0; i < CACHE_SIZE; i++) {
        cache[i].num = i;
        cache[i].testres = TEST_NONE;
    }
}

static void
cache_print (void)
{
    unsigned i, j;

    move(0, 0);
    printw("CACHE");
    for (i = 0; i < CACHE_SIZE; i++) {
        move(i + 2, 0);

        attron(COLOR_PAIR(cache[i].testres));
        printw("%s ", bin(cache[i].num, CACHE_NUM_SIZE, 0));
        cache[i].testres = TEST_NONE;
        attron(COLOR_PAIR(cache[i].testres));

        printw("%d ", cache[i].valid);
        printw("%s ", bin(cache[i].tag, CACHE_TAG_SIZE, 0));
        printw("%s ", bin(cache[i].offset, CACHE_OFFSET_SIZE, 0));

        for (j = 0; j < BYTE_Q; j++)
            printw("%03u ", cache[i].data[j]);
    }
}

static void
mem_init (void)
{
    int i;

    mem = malloc(sizeof(struct mem) * MEM_SIZE);
    for (i = 0; i < MEM_SIZE; i++) {
        mem[i].addr = (unsigned) &data[i];
        mem[i].data = data[i] = RAND(0, UINT_MAX - 1);
        mem[i].color = MEM_COLOR_NONE;
    }
}

static void
mem_print (void)
{
    int i;

    move(CACHE_SIZE + 3, 0);
    printw("MEMORY");
    for (i = 0; i < MEM_SIZE; i++) {
        move(CACHE_SIZE + 3 + i - i / MEM_COL_H * MEM_COL_H + 2,
            i / MEM_COL_H * MEM_COL_X_OFFSET);

        attron(COLOR_PAIR(mem[i].color));
        printw("0x%08x ", mem[i].addr);
        mem[i].color = MEM_COLOR_NONE;
        attron(COLOR_PAIR(mem[i].color));

        printw("%010u", mem[i].data);
    }
}

static void
redraw (void)
{
    erase();

    cache_print();
    mem_print();

    move(0, getmaxx(stdscr) - 11);
    printw("%10u", tacts);

    move(CUR_H, 0);
    printw("> ");
}

static void
cmd_parse (void)
{
    const char * arg, * delim = " ", * cmd_begin = cmd;
    struct cache * cache_sel;
    bool mbf = false, cbf = false;
    unsigned i, addr, tag, idx, offset;

    arg = strsep(&cmd, delim);
    if (!strcmp(arg, "help")) {
        printw("get <addr>\n");
        getch();
    }
    if (!strcmp(arg, "get")) {
        /* Parse a memory's address. */
        arg = strsep(&cmd, delim);
        if (!arg)
            return;

        addr = strtol(arg, NULL, 16);

        tag = GET_TAG(addr);
        idx = GET_INDEX(addr);
        offset = GET_OFFSET(addr);

        /* Compare a given address with a memory block's address. */
        for (i = 0; i < MEM_SIZE; i++)
            if (addr - offset == mem[i].addr) {
                mem[i].color = MEM_COLOR_SEL;

                mbf = true;

                break;
            }

        if (!mbf) {
            sprintf(errbuf, "Couldn't find a block of the memory by the "
                "0x%08x address.", addr);

            assert(errbuf);
        }

        /* Search a cache's block by an index. */
        for (i = 0; i < CACHE_SIZE; i++)
            if (cache[i].num == idx) {
                cache_sel = &cache[i];

                cbf = true;

                break;
            }

        if (!cbf) {
            sprintf(errbuf, "Couldn't find a block of the cache by the 0x%x "
                "index.", idx);

            assert(errbuf);
        }

        /* A hit test. */
        if (cache_sel->tag == tag)
            /* There was a hit. */
            cache_sel->testres = TEST_HIT;
        else {
            /* There was a miss. */
            cache_sel->testres = TEST_MISS;

            cache_sel->valid = true;
            cache_sel->tag = tag;

            for (i = 0; i < BYTE_Q; i++)
                cache_sel->data[ENDIAN == ENDIAN_LITTLE ? BYTE_Q - 1 - i : i]
                    = *(byte *) (addr + i);

            tacts += TACT_Q_MISS;
        }

        cache_sel->offset = offset;

        fprintf(data_file, "0x%08x %u %03d\n", addr - offset, offset,
            cache_sel->data[offset]);

        tacts += TACT_Q_READ;
    }

    cmd = (char *) cmd_begin;
}

static void
cache_dump (void)
{
    unsigned i, j;

    fprintf(log_file, "CACHE\n\n");
    for (i = 0; i < CACHE_SIZE; i++) {
        fprintf(log_file, "%s ", bin(cache[i].num, CACHE_NUM_SIZE, 0));
        fprintf(log_file, "%d ", cache[i].valid);
        fprintf(log_file, "%s ", bin(cache[i].tag, CACHE_TAG_SIZE, 0));
        fprintf(log_file, "%s ", bin(cache[i].offset, CACHE_OFFSET_SIZE, 0));

        for (j = 0; j < BYTE_Q; j++)
            fprintf(log_file, "%03u ", cache[i].data[j]);

        fprintf(log_file, "\n");
    }
}

static void
mem_dump (void)
{
    int i;

    fprintf(log_file, "MEMORY\n\n");
    for (i = 0; i < MEM_SIZE; i++)
        fprintf(log_file, "0x%08x %010u\n", mem[i].addr, mem[i].data);
}

static void
assert (const char * str)
{
    endwin();
    delscreen(scr);

    printf("%s\n", str);

    fprintf(log_file, "\n");
    cache_dump();
    fprintf(log_file, "\n");
    mem_dump();
    fprintf(log_file, "\n");
    fclose(log_file);

    fprintf(data_file, "\n");
    fclose(data_file);

    free(cache);
    free(mem);
    free(cmd);

    exit(EXIT_FAILURE);
}

static void
swap (char * arr, unsigned isrc, unsigned idst)
{
    char buf = arr[isrc];

    binbuf[isrc] = binbuf[idst];
    binbuf[idst] = buf;
}

static const char *
bin (unsigned val, unsigned len, unsigned shift)
{
    unsigned offset = 0, i;

    memset(binbuf, '0', ADDR_SIZE);

    do {
        binbuf[offset] = val % 2 ? '1' : '0';

        offset++;
        if (offset == ADDR_SIZE)
            break;

        val /= 2;
    }
    while (val > 0);

    /* Turn over the number. */
    for (i = 0; i < offset / 2; i++)
        swap(binbuf, i, offset - 1 - i);

    /* Move the number to the end of the buffer. */
    for (i = 1; i < offset + 1; i++)
        swap(binbuf, offset - i, ADDR_SIZE - i);

    binbuf[ADDR_SIZE - shift] = 0;

    return binbuf + ADDR_SIZE - len;
}

static unsigned
dec (const char * val)
{
    unsigned i, res = 0, len = strlen(val);

    for (i = 0; i < len; i++)
        res += (val[i] - '0') * pow(2, len - 1 - i);

    return res;
}
