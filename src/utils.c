#include <string.h>
#include <math.h>
#include "binbuf.h"

static void
swap (char arr [], unsigned a, unsigned b)
{
    char buf;

    buf = arr[a], arr[a] = arr[b], arr[b] = buf;
}

const char *
bin (unsigned val, unsigned l, unsigned r)
{
    unsigned n = 0, i;

    memset(binbuf, '0', BINBUF_SIZE);
    do {
        binbuf[n] = val % 2 ? '1' : '0';
        if (++n == BINBUF_SIZE)
            break;
        val /= 2;
    }
    while (val > 0);

    // Turn over number
    for (i = 0; i < n / 2; i++)
        swap(binbuf, i, n - 1 - i);

    // Move number to end of buffer
    for (i = 1; i < n + 1; i++)
        swap(binbuf, n - i, BINBUF_SIZE - 1 - i);

    binbuf[BINBUF_SIZE - 1 - r] = '\0';
    return binbuf + BINBUF_SIZE - 1 - (l == 0 ? BINBUF_SIZE - 1 : l);
}

unsigned
dec (const char * arr)
{
    unsigned i, v = 0, len = strlen(arr);

    for (i = 0; i < len; i++)
        v += (arr[i] - '0') * pow(2, len - 1 - i);

    return v;
}
