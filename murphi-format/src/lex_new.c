#include "lex.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>

int lex_new(lex_t *me, FILE *src) {
  assert(me != NULL);

  *me = (lex_t){0};
  int rc = 0;

  me->src = src;

  me->stage = open_memstream(&me->stage_base, &me->stage_size);
  if (me->stage == NULL) {
    rc = errno;
    goto done;
  }

done:
  return rc;
}
