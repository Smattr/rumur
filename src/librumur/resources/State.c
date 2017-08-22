#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct State_ {
    unsigned char data[STATE_SIZE_BITS / CHAR_BIT + (STATE_SIZE_BITS % CHAR_BIT == 0 ? 0 : 1)];
    const struct State_ *previous;
} State;

static State *state_copy(const State *s) {
    State *next = malloc(sizeof(*next));
    if (next == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(next->data, s->data, sizeof(next->data));
    next->previous = s;
    return next;
}

static bool state_cmp(const State *x, const State *y) {
    return memcmp(x->data, y->data, sizeof(x->data)) == 0;
}
