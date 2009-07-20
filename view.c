#include <stdlib.h>
#define OWL_PERL
#include "owl.h"

static const char fileIdent[] = "$Id$";

owl_view* owl_view_new(char *filtname)
{
  char *args[] = {filtname};
  return owl_perl_new_argv("BarnOwl::View", args, 1);
}

owl_filter * owl_view_get_filter(owl_view *v)
{
  return owl_global_get_filter(&g, owl_view_get_filtname(v));
}

/* if the message matches the filter then add to view */
void owl_view_consider_message(owl_view *v, owl_message *m)
{
  OWL_PERL_CALL_METHOD(v, "consider_message",
                       XPUSHs((SV*)m);,
                       "Error in consider_message: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

void owl_view_new_filter(owl_view *v, char *filtname)
{
  OWL_PERL_CALL_METHOD(v, "new_filter",
                       mXPUSHp(filtname, strlen(filtname));,
                       "Error in new_filter: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

int owl_view_is_empty(owl_view *v)
{
  int empty;
  OWL_PERL_CALL_METHOD(v, "is_empty",
                       /* no args */,
                       "Error: is_empty: %s",
                       /* fatal */ 1,
                       empty = POPi);
  return empty;
}

/* saves the current message position in the filter so it can 
 * be restored later if we switch back to this filter. */
void owl_view_save_curmsgid(owl_view *v, int curid)
{
  owl_filter_set_cachedmsgid(owl_view_get_filter(v), curid);
}

/* fmtext should already be initialized */
void owl_view_to_fmtext(owl_view *v, owl_fmtext *fm)
{
  owl_fmtext_append_normal(fm, "Filter: ");
  owl_fmtext_append_normal(fm, owl_view_get_filtname(v));
  owl_fmtext_append_normal(fm, "\n");
}

char *owl_view_get_filtname(owl_view *v)
{
  SV *name;
  OWL_PERL_CALL_METHOD(v, "get_filter",
                       /* no args */,
                       "Error: get_filter: %s",
                       1 /* fatal errors */,
                       name = SvREFCNT_inc(POPs));
  sv_2mortal(name);
  return SvPV_nolen(name);
}

void owl_view_free(owl_view *v)
{
  SvREFCNT_dec((SV*)v);
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

#define CALL_BOOL(it, method)                           \
  int _rv;                                              \
  OWL_PERL_CALL_METHOD(it, method,                      \
                       /* No args */,                   \
                       "Error: " #method ": %s",        \
                       /* fatal */ 1,                   \
                       _rv = POPi);                     \
  return _rv;

#define CALL_VOID(it, method)                           \
  OWL_PERL_CALL_METHOD(it, method,                      \
                       /* No args */,                   \
                       "Error: " #method ": %s",        \
                       /* fatal */ 1,                   \
                       OWL_PERL_VOID_CALL);             \

owl_view_iterator * owl_view_iterator_new()
{
  return owl_perl_new("BarnOwl::View::Iterator");
}

void owl_view_iterator_invalidate(owl_view_iterator *it)
{
  CALL_VOID(it, "invalidate");
}

int owl_view_iterator_is_valid(owl_view_iterator *it)
{
  CALL_BOOL(it, "valid");
}

void owl_view_iterator_init_id(owl_view_iterator *it, owl_view *v, int message_id)
{
  OWL_PERL_CALL_METHOD(it, "initialize_at_id",
                       /* args */
                       XPUSHs(v);
                       mXPUSHi(message_id);,
                       "Error: initialize at id: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

/* Initialized iterator to point at the first message */
void owl_view_iterator_init_start(owl_view_iterator *it, owl_view *v)
{
  OWL_PERL_CALL_METHOD(it, "initialize_at_start",
                       /* args */
                       XPUSHs(v);,
                       "Error: initialize at start: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

/* Initialized iterator to point after the last message */
void owl_view_iterator_init_end(owl_view_iterator *it, owl_view *v)
{
  OWL_PERL_CALL_METHOD(it, "initialize_at_end",
                       /* args */
                       XPUSHs(v);,
                       "Error: initialize at end: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

void owl_view_iterator_clone(owl_view_iterator *dst, owl_view_iterator *src)
{
  OWL_PERL_CALL_METHOD(dst, "clone",
                       XPUSHs(src);,
                       "Error: clone: %s",
                       /* fatal */ 1,
                       OWL_PERL_VOID_CALL);
}

int owl_view_iterator_is_at_end(owl_view_iterator *it)
{
  CALL_BOOL(it, "is_at_end");
}

int owl_view_iterator_is_at_start(owl_view_iterator *it)
{
  CALL_BOOL(it, "is_at_start");
}

void owl_view_iterator_prev(owl_view_iterator *it)
{
  CALL_VOID(it, "prev");
}

void owl_view_iterator_next(owl_view_iterator *it)
{
  CALL_VOID(it, "next");
}

owl_message* owl_view_iterator_get_message(owl_view_iterator *it)
{
  SV *msg;
  OWL_PERL_CALL_METHOD(it, "get_message",
                       /* no args */,
                       "Error: get_message: %s",
                       /* fatal errors */ 1,
                       msg = POPs;
                       if(SvROK(msg)) SvREFCNT_inc(msg);
                       );
  return SvROK(msg) ? sv_2mortal(msg) : NULL;
}

int owl_view_iterator_cmp(owl_view_iterator *it1, owl_view_iterator *it2)
{
  int cmp;
  OWL_PERL_CALL_METHOD(it1, "cmp",
                       XPUSHs(it2);,
                       "Error: cmp: %s",
                       /* fatal */ 1,
                       cmp = POPi);
  return cmp;
}

void owl_view_iterator_free(owl_view_iterator *it)
{
  SvREFCNT_dec(it);
}

owl_view_iterator* owl_view_iterator_free_later(owl_view_iterator *it)
{
  return sv_2mortal(it);
}

int owl_view_iterator_same_view(owl_view_iterator *it1, owl_view_iterator *it2)
{
  int same;
  OWL_PERL_CALL_METHOD(it1, "same_view",
                       XPUSHs(it2);,
                       "Error: same_view: %s",
                       /* fatal */ 1,
                       same = POPi);
  return same;
}
