/* Wrappers around GLib to provide various container data structures.
 *
 * If you want to swap GLib out for another implementation that provides similar
 * data structures, this should be straightforward. Simply modify the following
 * functions, retaining the same API and remove the glib.h #include.
 */

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    GHashTable *ghashtable;
} set_t;

static void free_state(gpointer data) {
    State *s = (State*)data;
    free(s);
}

static int set_init(set_t *set) {
    assert(set != NULL);
    set->ghashtable = g_hash_table_new_full(/* TODO */ NULL, /* TODO */ NULL,
      free_state, NULL);
    return 0;
}

static bool set_insert(set_t *set, State *s) {
    assert(set != NULL);
    assert(set->ghashtable != NULL);
    return (bool)g_hash_table_add(set->ghashtable, (gpointer)s);
}

static void set_deinit(set_t *set) {
    assert(set != NULL);
    assert(set->ghashtable != NULL);
    g_hash_table_destroy(set->ghashtable);
}

typedef struct {
    GQueue *gqueue;
} queue_t;

static int queue_init(queue_t *queue) {
    assert(queue != NULL);
    queue->gqueue = g_queue_new();
    return 0;
}

static void queue_push(queue_t *queue, State *s) {
    assert(queue != NULL);
    assert(queue->gqueue != NULL);
    g_queue_push_tail(queue->gqueue, (gpointer)s);
}

static State *queue_pop(queue_t *queue) {
    assert(queue != NULL);
    assert(queue->gqueue != NULL);
    return (State*)g_queue_pop_head(queue->gqueue);
}
