#include "owl.h"
#include <unistd.h>
#include <stdlib.h>

static const char fileIdent[] = "$Id$";

owl_global g;

#define OWL_DICT_NTESTS         19
#define OWL_UTIL_NTESTS         6
#define OWL_OBARRAY_NTESTS      6
#define OWL_VARIABLE_NTESTS     36
#define OWL_FILTER_NTESTS       24
#define OWL_LIST_NTESTS         84

int main(int argc, char **argv, char **env)
{
  owl_errqueue_init(owl_global_get_errqueue(&g));
  owl_obarray_init(&(g.obarray));
  owl_perlconfig_initperl(NULL, &argc, &argv, &env);
  /* Now that we have perl, we can initialize the msssage list*/
  g.msglist = owl_messagelist_new();

  int numfailures=0;
  printf("1..%d\n", OWL_UTIL_NTESTS+OWL_DICT_NTESTS+OWL_VARIABLE_NTESTS
         +OWL_FILTER_NTESTS+OWL_OBARRAY_NTESTS+OWL_LIST_NTESTS);
  numfailures += owl_util_regtest();
  numfailures += owl_dict_regtest();
  numfailures += owl_variable_regtest();
  numfailures += owl_filter_regtest();
  numfailures += owl_obarray_regtest();
  numfailures += owl_list_regtest();
  if (numfailures) {
      fprintf(stderr, "# *** WARNING: %d failures total\n", numfailures);
  }
  return(numfailures);
}
