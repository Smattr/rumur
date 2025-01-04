#include "lex.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void lex_free(lex_t *me) {
  assert(me != NULL);

  if (me->stage != NULL)
    (void)fclose(me->stage);
  free(me->stage_base);

  *me = (lex_t){0};
}
