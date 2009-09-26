#include "owl.h"

#define OWLVAR_BOOL(name,cname,default,summary,description)             \
  void owl_global_set_ ## cname ## _on(owl_global *g) {                 \
    owl_variable_set_bool_on(&g->vars, name);                           \
  }                                                                     \
  void owl_global_set_ ## cname ## _off(owl_global *g) {                \
    owl_variable_set_bool_off(&g->vars, name);                          \
  }                                                                     \
  int owl_global_is_ ## cname (const owl_global *g) {                   \
    return owl_variable_get_bool(&g->vars, name);                       \
  }

#define OWLVAR_STRING(name,cname,default,summary,description)           \
  void owl_global_set_ ## cname (owl_global *g, const char *val) {      \
    owl_variable_set_string(&g->vars, name, val);                       \
  }                                                                     \
  const char *owl_global_get_ ## cname (owl_global *g) {                \
    return owl_variable_get_string(&g->vars, name);                     \
  }

#define OWLVAR_INT(name,cname,default,summary,description)              \
  void owl_global_set_ ## cname (owl_global *g, int n) {                \
    owl_variable_set_int(&g->vars, name, n);                            \
  }                                                                     \
  int owl_global_get_ ## cname (const owl_global *g) {                  \
    return owl_variable_get_int(&g->vars, name);                        \
  }

#define OWLVAR_ONLY_NAMES

#include "varlist.h"
