#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

void *xalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(-1);
    }
    return p;
}

typedef struct state {
    mpz_t data;
    struct state *previous;
} state_t;

void state_init(state_t *s) {
    mpz_init(s->data);
    s->previous = NULL;
}

void state_clone(state_t *s, const state_t *from) {
    mpz_init_set(s->data, from->data);
    s->previous = from->previous;
}

void state_clear(state_t *s) {
    mpz_clear(s->data);
}

state_t *state_alloc(void) {
    state_t *s = xalloc(sizeof(*s));
    state_init(s);
    return s;
}

void state_free(state_t *s) {
    state_clear(s);
    free(s);
}

typedef struct {
    mpz_t offset;
    mpz_t size;
} field_t;

void state_read(mpz_t *result, state_t *s, const field_t f) {
    /* quotient = offset == 0 ? data : data / offset; */
    mpz_t quotient;
    if (mpz_cmp_ui(f.offset, 0) == 0) {
        mpz_init_set(quotient, s->data);
    } else {
        mpz_init(quotient);
        mpz_cdiv_q(quotient, s->data, f.offset);
    }

    /* remainder = quotient % size; */
    mpz_mod(*result, quotient, f.size);

    mpz_clear(quotient);
}

void state_write(state_t *s, const field_t f, const mpz_t value) {
    /* upper = offset == 0 ? data : data / offset; */
    mpz_t upper;
    if (mpz_cmp_ui(f.offset, 0) == 0) {
        mpz_init_set(upper, s->data);
    } else {
        mpz_init(upper);
        mpz_cdiv_q(upper, s->data, f.offset);
    }

    /* upper = upper / size; */
    mpz_cdiv_q(upper, upper, f.size);

    /* lower = offset == 0 ? 0 : data % offset; */
    mpz_t lower;
    mpz_init(lower);
    if (mpz_cmp_ui(f.offset, 0) == 0) {
        /* lower is already 0. */
    } else {
        mpz_mod(lower, s->data, f.offset);
    }

    /* updated = upper; */
    mpz_t updated;
    mpz_init_set(updated, upper);

    /* updated = updated * size + value; */
    mpz_mul(updated, updated, f.size);
    mpz_add(updated, updated, value);

    /* updated = offset == 0 ? updated : updated * offset + lower; */
    if (mpz_cmp_ui(f.offset, 0) == 0) {
        /* nothing required */
    } else {
        mpz_mul(updated, updated, f.offset);
        mpz_add(updated, updated, lower);
    }

    /* Update state */
    mpz_set(s->data, updated);

    /* clean up */
    mpz_clear(upper);
    mpz_clear(lower);
    mpz_clear(updated);
}

typedef struct {
    state_t *final;
    char *message;
} violation_t;

typedef void (*handler_t)(violation_t *v);

typedef struct {
    bool (*guard)(state_t *s);
    state_t *(*apply)(state_t *s, handler_t cb);
} rule_t;

rule_t rules[] = {
};

void dfs(state_t *s, unsigned int depth, handler_t cb) {
    if (depth == 0)
        return;

    for (unsigned int i = 0; i < sizeof(rules) / sizeof(rule_t); i++) {
        rule_t *r = &rules[i];
        if (r->guard(s)) {
            state_t *s1 = r->apply(s, cb);
            dfs(s1, depth - 1, cb);
            if (s1 != s)
                free(s1);
        }
    }
}

void check(unsigned int depth, handler_t cb) {
    state_t *s = xalloc(sizeof(*s));
    state_init(s);
    dfs(s, depth, cb);
}

int main(int argc, char **argv) {
    void handle(violation_t *v) {
        fprintf(stderr, "violation: %s", v->message);
        exit(1);
    }
    check(10, handle);
    return 0;
}
