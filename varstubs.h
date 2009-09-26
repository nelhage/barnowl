#define OWLVAR_BOOL(name,cname,default,summary,description)             \
  extern void owl_global_set_ ## cname ## _on(owl_global *g);           \
  extern void owl_global_set_ ## cname ## _off(owl_global *g);          \
  extern int owl_global_is_ ## cname (const owl_global *g);

#define OWLVAR_STRING(name,cname,default,summary,description)            \
  extern void owl_global_set_ ## cname (owl_global *g, const char *val); \
  extern const char *owl_global_get_ ## cname (owl_global *g);

#define OWLVAR_INT(name,cname,default,summary,description)              \
  extern void owl_global_set_ ## cname (owl_global *g, int n);          \
  extern int owl_global_get_ ## cname (const owl_global *g);

#define OWLVAR_ONLY_NAMES

#include "varlist.h"

#undef OWLVAR_BOOL
#undef OWLVAR_BOOL_FULL
#undef OWLVAR_STRING
#undef OWLVAR_STRING_FULL
#undef OWLVAR_PATH
#undef OWLVAR_INT
#undef OWLVAR_INT_FULL
#undef OWLVAR_ENUM
#undef OWLVAR_ENUM_FULL
