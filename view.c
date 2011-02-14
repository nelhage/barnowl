#include <stdlib.h>
#define OWL_PERL
#include "owl.h"

#define BITS_PER_LONG (8 * sizeof(unsigned long))

typedef struct _ov_range  { /* noproto */
  long next_bk;
  long next_fwd;
  struct _ov_range *next;
  struct _ov_range *prev;
} ov_range;

struct _owl_view { /* noproto */
  unsigned long *messages;
  unsigned long next_cached_id;
  ov_range *ranges;
  char     *filter_name;
  long      saved_id;
};

struct _owl_view_iterator { /* noproto */
  owl_view *view;
  long      index;
  struct _owl_view_iterator *delete_next;
};

static owl_view_iterator *ovi_delete_later;

/* Internal prototypes */
/* ov_range */
static ov_range *ov_range_new(int bk, int fwd, ov_range *prev, ov_range *next);
static int ov_range_expand_fwd(ov_range *v, int id);
static int ov_range_expand_bk (ov_range *v, int id);
static ov_range *ov_range_find_or_insert(ov_range *v, int id);
/* owl_view */
static void ov_mark_message(owl_view *v, int id, bool mark);
static int  ov_marked(owl_view *v, int id);
static void ov_fill_back(owl_view *v, ov_range *r);
static void ov_fill_forward(owl_view *v, ov_range *r);
static ov_range *ov_range_at(owl_view *v, int id);
/* owl_view_iterator */
static ov_range *ovi_range(owl_view_iterator *it);
static int ovi_next(owl_view_iterator *it);
static int ovi_prev(owl_view_iterator *it);
static int ovi_fixup(owl_view_iterator *it);
static int ovi_fill_forward(owl_view_iterator *it);

static GList *all_views = NULL;

owl_view* owl_view_new(const char *filtname)
{
  owl_view *v = g_new(owl_view, 1);
  ov_range *init = ov_range_new(-1, -1, NULL, NULL);
  v->ranges = init;
  v->messages = g_malloc(4096);
  memset(v->messages, 0, 4096);
  v->next_cached_id  = 4096 * 8;
  v->saved_id = 0;
  v->filter_name = g_strdup(filtname);
  all_views = g_list_prepend(all_views, v);
  return v;
}

owl_filter * owl_view_get_filter(owl_view *v)
{
  return owl_global_get_filter(&g, owl_view_get_filtname(v));
}

/* if the message matches the filter then add to view */
void owl_view_consider_message(owl_view *v, const owl_message *m)
{
  const owl_filter *f = owl_global_get_filter(&g, v->filter_name);
  int id = owl_message_get_id(m);
  ov_range *r;
  if (!f)
    return;

  r = ov_range_at(v, id);
  ov_range_expand_fwd(r, id + 1);
  ov_mark_message(v, id, owl_filter_message_match(f, m));
}

void owl_view_handle_deletion(int id)
{
  GList *l = all_views;
  while(l) {
    ov_mark_message(l->data, id, false);
    l = g_list_next(l);
  }
}

int owl_view_is_empty(const owl_view *v)
{
  owl_view_iterator it;
  owl_view_iterator_init_start(&it, v);
  return owl_view_iterator_is_at_end(&it);
}

/* saves the current message position in the filter so it can 
 * be restored later if we switch back to this filter. */
void owl_view_save_curmsgid(owl_view *v, int curid)
{
  v->saved_id = curid;
}

int owl_view_get_saved_msgid(const owl_view *v)
{
  return v->saved_id;
}

/* fmtext should already be initialized */
void owl_view_to_fmtext(const owl_view *v, owl_fmtext *fm)
{
  owl_fmtext_append_normal(fm, "Filter: ");
  owl_fmtext_append_normal(fm, owl_view_get_filtname(v));
  owl_fmtext_append_normal(fm, "\n");
}

const char *owl_view_get_filtname(const owl_view *v)
{
  return v->filter_name;
}

void owl_view_delete(owl_view *v)
{
  all_views = g_list_remove(all_views, v);
  ov_range *r = v->ranges;
  g_free(v->messages);
  g_free(v->filter_name);
  while(r) {
    ov_range *next = r->next;
    g_free(r);
    r = next;
  }
  g_free(v);
}

/********************************************************************
 * View iterators
 *
 * The only supported way to access the elements of a view are through
 * the view iterator API presented here.
 *
 * There are three ways to initialize an iterator:
 * * Before the first of the view's messages
 * * Given an ID, before the message in the view closest to that ID
 * * After the last of the view's messages
 *
 * Iterators strictly point between two messages, not at a message. A
 * method is provided to return the message after an iterator; It
 * returns NULL if the iterator points beyond the last message in the
 * list.
 *
 * The predicates is_at_start and is_at_end test for the iterators at
 * either end of the list.
 *
 * _prev and _next, if applied to iterators at the start or end,
 * respectively, are no-ops.
 */

owl_view_iterator * owl_view_iterator_new(void)
{
  owl_view_iterator *it = g_new(owl_view_iterator, 1);
  it->delete_next = NULL;
  owl_view_iterator_invalidate(it);
  return it;
}

void owl_view_iterator_invalidate(owl_view_iterator *it)
{
  it->view = NULL;
}

int owl_view_iterator_is_valid(owl_view_iterator *it)
{
  return it->view != NULL;
}

void owl_view_iterator_init_id(owl_view_iterator *it, const owl_view *v, int message_id)
{
  it->view  = (owl_view *)v;
  it->index = message_id;
}

/* Initialized iterator to point at the first message */
void owl_view_iterator_init_start(owl_view_iterator *it, const owl_view *v)
{
  it->view = (owl_view *)v;
  it->index = 0;
}

/* Initialized iterator to point after the last message */
void owl_view_iterator_init_end(owl_view_iterator *it, const owl_view *v)
{
  ov_range *r;
  it->view = (owl_view *)v;
  r = ov_range_at(it->view, -1);
  ov_fill_back(it->view, r);
  it->index = r->next_fwd;
}

void owl_view_iterator_clone(owl_view_iterator *dst, owl_view_iterator *src)
{
  dst->view = src->view;
  dst->index = src->index;
}

int owl_view_iterator_is_at_end(owl_view_iterator *it)
{
  ovi_fixup(it);
  return !ov_marked(it->view, it->index);
}

int owl_view_iterator_is_at_start(owl_view_iterator *it)
{
  int save_index = it->index;
  int rv = ovi_prev(it);
  it->index = save_index;
  return rv;
}

void owl_view_iterator_prev(owl_view_iterator *it)
{
  ovi_prev(it);
}

void owl_view_iterator_next(owl_view_iterator *it)
{
  ovi_next(it);
}

owl_message* owl_view_iterator_get_message(owl_view_iterator *it)
{
  ovi_fixup(it);
  return owl_messagelist_get_by_id(owl_global_get_msglist(&g),
                                   it->index);
}

int owl_view_iterator_cmp(owl_view_iterator *it1, owl_view_iterator *it2)
{
  ovi_fixup(it1);
  ovi_fixup(it2);
  return it1->index - it2->index;
}

void owl_view_iterator_delete(owl_view_iterator *it)
{
  g_free(it);
}

owl_view_iterator* owl_view_iterator_delete_later(owl_view_iterator *it)
{
  it->delete_next = ovi_delete_later;
  ovi_delete_later = it;
  return it;
}

int owl_view_iterator_delayed_delete(owl_ps_action *d, void *p) {
  owl_view_iterator *it;
  while (ovi_delete_later) {
    it = ovi_delete_later;
    ovi_delete_later = it->delete_next;
    owl_view_iterator_delete(it);
  }
  return 0;
}

/* Internal methods to the view and iterator implementation */
/* ov_range */
static ov_range *ov_range_new(int bk, int fwd, ov_range *prev, ov_range *next)
{
  ov_range *r = g_new(ov_range, 1);
  r->next_bk  = bk;
  r->next_fwd = fwd;
  r->prev     = prev;
  r->next     = next;
  if (r->prev)
    r->prev->next = r;
  if (r->next)
    r->next->prev = r;
  return r;
}

static int ov_range_expand_fwd(ov_range *v, int id)
{
  int merged = 0;

  if (id < v->next_fwd)
    return 0;
  v->next_fwd = id;

  while (v->next &&
         v->next_fwd >= v->next->next_bk &&
         v->next->next_bk >= 0) {
    ov_range *tmp = v->next;
    merged = 1;
    v->next_fwd = MAX(v->next_fwd, tmp->next_fwd);
    v->next     = tmp->next;
    if (v->next) {
      v->next->prev = v;
    }
    g_free(tmp);
  }
  return merged;
}

static int ov_range_expand_bk(ov_range *v, int id)
{
  int merged = 0;
  if (v->next_bk >= 0 && id > v->next_bk)
    return 0;
  v->next_bk = id;

  while (v->prev &&
         v->next_bk <= v->prev->next_bk) {
    ov_range *tmp = v->prev;
    merged = 1;
    v->next_bk = MIN(v->next_bk, tmp->next_bk);
    v->prev    = tmp->prev;
    if (v->prev) {
      v->prev->next = v;
    }
    g_free(tmp);
  }
  return merged;
}

static ov_range *ov_range_find_or_insert(ov_range *v, int id)
{
  ov_range *last = NULL;

  if (id < 0) {
    while (v->next) v = v->next;
    if (v->next_bk < 0) return v;
    return ov_range_new(-1, -1, v, NULL);
  }

  while (v) {
    if (id < v->next_bk) {
      /*
       * We've passed the id without finding the range, insert a
       * zero-length range here.
       */
      return ov_range_new(id, id, v->prev, v);
    } else if (id >= v->next_bk && id <= v->next_fwd) {
      return v;
    }
    last = v;
    v = v->next;
  }

  return ov_range_new(id, id, last, NULL);
}

/* owl_view */
static void ov_mark_message(owl_view *v, int id, bool mark)
{
  if (id >= v->next_cached_id) {
    unsigned long old_bytes = v->next_cached_id / 8;
    unsigned long new_bytes = old_bytes;
    while (id >= new_bytes * 8)
      new_bytes *= 2;
    new_bytes = ROUNDUP(new_bytes, sizeof (unsigned long));

    v->messages = g_realloc(v->messages, new_bytes);
    v->next_cached_id = new_bytes * 8;
    memset((unsigned char*)v->messages + old_bytes, 0, new_bytes - old_bytes);
  }

  if (mark)
    v->messages[id / BITS_PER_LONG] |= 1UL << (id % BITS_PER_LONG);
  else
    v->messages[id / BITS_PER_LONG] &= ~(1UL << (id % BITS_PER_LONG));
}

static int  ov_marked(owl_view *v, int id)
{
  if (id >= v->next_cached_id)
    return 0;
  return !!(v->messages[id / BITS_PER_LONG] & (1UL << (id % BITS_PER_LONG)));
}

#define FILL_STEP 100

static void ov_fill_back(owl_view *v, ov_range *r)
{
  int pos, mid;
  owl_messagelist *ml;
  owl_message *m;
  owl_filter *f;

  pos = r->next_bk;
  ml  = owl_global_get_msglist(&g);
  f   = owl_view_get_filter(v);

  owl_messagelist_iterate_begin(ml, pos, true);
  do {
    m = owl_messagelist_iterate_next(ml);
    if (!m)
      break;

    mid = owl_message_get_id(m);

    if (r->next_fwd < 0)
      ov_range_expand_fwd(r, mid + 1);

    if (pos < 0)
      pos = mid;

    if (owl_filter_message_match(f, m))
      ov_mark_message(v, mid, true);

    if (ov_range_expand_bk(r, mid)) {
      owl_messagelist_iterate_done(ml);
      owl_messagelist_iterate_begin(ml, r->next_bk, true);
    }

  } while ((pos - mid) < FILL_STEP);

  owl_messagelist_iterate_done(ml);
  return;
}

static void ov_fill_forward(owl_view *v, ov_range *r)
{
  int pos, mid;
  owl_messagelist *ml;
  owl_message *m;
  owl_filter *f;

  pos = r->next_fwd;
  ml  = owl_global_get_msglist(&g);
  f   = owl_view_get_filter(v);

  owl_messagelist_iterate_begin(ml, pos, false);
  do {
    m = owl_messagelist_iterate_next(ml);
    if (!m)
      break;

    mid = owl_message_get_id(m);

    if (pos < 0)
      pos = mid;

    if (owl_filter_message_match(f, m))
      ov_mark_message(v, mid, true);

    if (ov_range_expand_fwd(r, mid + 1)) {
      owl_messagelist_iterate_done(ml);
      owl_messagelist_iterate_begin(ml, r->next_fwd, false);
    }

  } while((mid - pos) < FILL_STEP);

  owl_messagelist_iterate_done(ml);
  return;
}

static ov_range *ov_range_at(owl_view *v, int id)
{
  return ov_range_find_or_insert(v->ranges, id);
}
/* owl_view_iterator */
static ov_range *ovi_range(owl_view_iterator *it)
{
  return ov_range_at(it->view, it->index);
}

static int ovi_next(owl_view_iterator *it)
{
  if (ovi_fixup(it))
    return 1;
  it->index++;
  return ovi_fixup(it);
}

static int ovi_prev(owl_view_iterator *it)
{
  int old_index = it->index;
  ov_range *r = ovi_range(it);

  do {
    if (it->index == r->next_bk) {
      ov_fill_back(it->view, r);
      if (it->index == r->next_bk) {
        /* Hit the start */
        it->index = old_index;
        return 1;
      }
    }
    it->index--;
  } while (!ov_marked(it->view, it->index));

  return 0;
}

static int ovi_fixup(owl_view_iterator *it)
{
  do {
    while (it->index < ovi_range(it)->next_fwd) {
      if (ov_marked(it->view, it->index))
        return 0;
      it->index++;
    }
    if (ovi_fill_forward(it))
      return 1;
  } while (!ov_marked(it->view, it->index));
  return 0;
}

static int ovi_fill_forward(owl_view_iterator *it)
{
  ov_range *r = ovi_range(it);

  if (it->index >= r->next_fwd) {
    ov_fill_forward(it->view, r);
    if (it->index >= r->next_fwd)
      return 1;
  }
  return 0;
}
