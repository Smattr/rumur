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
