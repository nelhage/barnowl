#include "owl.h"
#include <stdlib.h>
#include <string.h>

int owl_messagelist_create(owl_messagelist *ml) {
  owl_list_create(&(ml->list));
  return(0);
}

int owl_messagelist_get_size(owl_messagelist *ml) {
  return(owl_list_get_size(&(ml->list)));
}

void *owl_messagelist_get_element(owl_messagelist *ml, int n) {
  return(owl_list_get_element(&(ml->list), n));
}

owl_message *owl_messagelist_get_by_id(owl_messagelist *ml, int id) {
  /* return the message with id == 'id'.  If it doesn't exist return NULL. */
  /* we could make this much more efficient at some point */
  int i, j;
  owl_message *m;

  j=owl_list_get_size(&(ml->list));
  for (i=0; i<j; i++) {
    m=owl_list_get_element(&(ml->list), i);
    if (owl_message_get_id(m)==id) return(m);

    /* the message id's have to be sequential.  If we've passed the
       one we're looking for just bail */
    if (owl_message_get_id(m) > id) return(NULL);
  }
  return(NULL);
}

int owl_messagelist_append_element(owl_messagelist *ml, void *element) {
  return(owl_list_append_element(&(ml->list), element));
}

/* do we really still want this? */
int owl_messagelist_delete_element(owl_messagelist *ml, int n) {
  /* mark a message as deleted */
  owl_message_mark_delete(owl_list_get_element(&(ml->list), n));
  return(0);
}

int owl_messagelist_undelete_element(owl_messagelist *ml, int n) {
  /* mark a message as deleted */
  owl_message_unmark_delete(owl_list_get_element(&(ml->list), n));
  return(0);
}

int owl_messagelist_expunge(owl_messagelist *ml) {
  /* expunge deleted messages */
  int i, j;
  owl_list newlist;
  owl_message *m;

  owl_list_create(&newlist);
  /*create a new list without messages marked as deleted */
  j=owl_list_get_size(&(ml->list));
  for (i=0; i<j; i++) {
    m=owl_list_get_element(&(ml->list), i);
    if (owl_message_is_delete(m)) {
      owl_message_free(m);
    } else {
      owl_list_append_element(&newlist, m);
    }
  }

  /* free the old list */
  owl_list_free_simple(&(ml->list));

  /* copy the new list to the old list */
  memcpy(&(ml->list), &newlist, sizeof(owl_list));

  return(0);
}
