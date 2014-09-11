#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

/* Some mpz functions that GMP does not seem to provide. */
void mpz_lor(mpz_t dest, const mpz_t x, const mpz_t y) {
    if (mpz_cmp_ui(x, 0) == 0) {
        mpz_set(dest, y);
    } else {
        mpz_set(dest, x);
    }
}

void mpz_land(mpz_t dest, const mpz_t x, const mpz_t y) {
    if (mpz_cmp_ui(x, 0) == 0) {
        mpz_set_ui(dest, 0);
    } else {
        mpz_set(dest, y);
    }
}

void mpz_implies(mpz_t dest, const mpz_t x, const mpz_t y) {
    if (mpz_cmp_ui(x, 0) != 0) {
        mpz_set_ui(dest, 1);
    } else {
        mpz_set(dest, y);
    }
}

void mpz_lt(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) < 0);
}

void mpz_lte(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) <= 0);
}

void mpz_gt(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) > 0);
}

void mpz_gte(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) >= 0);
}

void mpz_eq(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) == 0);
}

void mpz_neq(mpz_t dest, const mpz_t x, const mpz_t y) {
    mpz_set_si(dest, mpz_cmp(x, y) != 0);
}

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

/* This needs to be a macro so we can return an mpz_t. */
#define r(root, offset, cardinality) \
    ({ \
        mpz_t x; \
        mpz_init(x); \
        mpz_div_ui(x, root, offset); \
        mpz_mod_ui(x, x, cardinality); \
        x; \
    })

void w(mpz_t root, unsigned int offset, unsigned int cardinality,
        mpz_t src) {
    mpz_mul_ui(src, src, offset);

    /* lower = root % offset */
    mpz_t lower;
    mpz_init(lower);
    mpz_mod_ui(lower, root, offset);

    /* upper = root / (offset * cardinality) * (offset * cardinality) */
    mpz_t upper;
    mpz_init(upper);
    mpz_div_ui(upper, root, offset * cardinality);
    mpz_mul_ui(upper, root, offset * cardinality);

    /* root = upper + src + lower */
    mpz_add(upper, upper, lower);
    mpz_add(upper, upper, src);
    mpz_set(root, upper);

    mpz_clear(upper);
    mpz_clear(lower);
    mpz_clear(src);
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
