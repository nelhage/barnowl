#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "owl.h"

#define OWLVAR_BOOL(name,cname,default,summary,description)             \
        { name, OWL_VARIABLE_BOOL, NULL, default, "on,off", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL },

#define OWLVAR_BOOL_FULL(name,cname,default,summary,description,validate,set,get) \
        { name, OWL_VARIABLE_BOOL, NULL, default, "on,off", summary,description, NULL, \
        validate, set, NULL, get, NULL, NULL },

#define OWLVAR_INT(name,cname,default,summary,description)              \
        { name, OWL_VARIABLE_INT, NULL, default, "<int>", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL },

#define OWLVAR_INT_FULL(name,cname,default,summary,description,validset,validate,set,get) \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        validate, set, NULL, get, NULL, NULL },

#define OWLVAR_PATH(name,cname,default,summary,description)             \
        { name, OWL_VARIABLE_STRING, default, 0, "<path>", summary,description,  NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL },

#define OWLVAR_STRING(name,cname,default,summary,description)           \
        { name, OWL_VARIABLE_STRING, default, 0, "<string>", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL },

#define OWLVAR_STRING_FULL(name,cname,default,summary,description,validate,set,get) \
        { name, OWL_VARIABLE_STRING, default, 0, "<string>", summary,description, NULL, \
        validate, set, NULL, get, NULL, NULL },

/* enums are really integers, but where validset is a comma-separated
 * list of strings which can be specified.  The tokens, starting at 0,
 * correspond to the values that may be specified. */
#define OWLVAR_ENUM(name,cname,default,summary,description,validset)    \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        owl_variable_enum_validate, \
        NULL, owl_variable_enum_set_fromstring, \
        NULL, owl_variable_enum_get_tostring, \
        NULL },

#define OWLVAR_ENUM_FULL(name,cname,default,summary,description,validset,validate, set, get) \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        validate, \
        set, owl_variable_enum_set_fromstring, \
        get, owl_variable_enum_get_tostring, \
        NULL },

static owl_variable variables_to_init[] = {

#include "varlist.h"

  { NULL, 0, NULL, 0, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL }
};

/**************************************************************************/
/*********************** SPECIFIC TO VARIABLES ****************************/
/**************************************************************************/


/* commonly useful */

int owl_variable_int_validate_gt0(const owl_variable *v, const void *newval)
{
  if (newval == NULL) return(0);
  else if (*(const int*)newval < 1) return(0);
  else return (1);
}

int owl_variable_int_validate_positive(const owl_variable *v, const void *newval)
{
  if (newval == NULL) return(0);
  else if (*(const int*)newval < 0) return(0);
  else return (1);
}

/* typewinsize */
int owl_variable_typewinsize_set(owl_variable *v, const void *newval)
{
  int rv;
  rv = owl_variable_int_set_default(v, newval);
  if (0 == rv) owl_function_resize();
  return(rv);
}

/* debug (cache value in g->debug) */
int owl_variable_debug_set(owl_variable *v, const void *newval)
{
  if (newval && (*(const int*)newval == 1 || *(const int*)newval == 0)) {
    g.debug = *(const int*)newval;
  }
  return owl_variable_bool_set_default(v, newval);
}

/* When 'aaway' is changed, need to notify the AIM server */
int owl_variable_aaway_set(owl_variable *v, const void *newval)
{
  if (newval) {
    if (*(const int*)newval == 1) {
      owl_aim_set_awaymsg(owl_global_get_aaway_msg(&g));
    } else if (*(const int*)newval == 0) {
      owl_aim_set_awaymsg("");
    }
  }
  return owl_variable_bool_set_default(v, newval);
}

int owl_variable_pseudologins_set(owl_variable *v, const void *newval)
{
  if (newval) {
    if (*(const int*)newval == 1) {
      owl_function_zephyr_buddy_check(0);
    }
  }
  return owl_variable_bool_set_default(v, newval);
}

/* note that changing the value of this will clobber 
 * any user setting of this */
int owl_variable_disable_ctrl_d_set(owl_variable *v, const void *newval)
{
  if (newval && !owl_context_is_startup(owl_global_get_context(&g))) {
    if (*(const int*)newval == 2) {
      owl_function_command_norv("bindkey editmulti C-d command edit:delete-next-char");
    } else if (*(const int*)newval == 1) {
      owl_function_command_norv("bindkey editmulti C-d command edit:done-or-delete");
    } else {
      owl_function_command_norv("bindkey editmulti C-d command edit:done");
    }
  }  
  return owl_variable_int_set_default(v, newval);  
}

int owl_variable_tty_set(owl_variable *v, const void *newval)
{
  owl_zephyr_set_locationinfo(owl_global_get_hostname(&g), newval);
  return(owl_variable_string_set_default(v, newval));
}


/**************************************************************************/
/****************************** GENERAL ***********************************/
/**************************************************************************/

int owl_variable_dict_setup(owl_vardict *vd) {
  owl_variable *var, *cur;
  if (owl_dict_create(vd)) return(-1);
  for (var = variables_to_init; var->name != NULL; var++) {
    cur = owl_malloc(sizeof(owl_variable));
    memcpy(cur, var, sizeof(owl_variable));
    switch (cur->type) {
    case OWL_VARIABLE_OTHER:
      cur->set_fn(cur, cur->pval_default);
      break;
    case OWL_VARIABLE_STRING:
      if (!cur->validate_fn) 
	cur->validate_fn = owl_variable_string_validate_default;
      if (!cur->set_fn) 
	cur->set_fn = owl_variable_string_set_default;
      if (!cur->set_fromstring_fn) 
	cur->set_fromstring_fn = owl_variable_string_set_fromstring_default;
      if (!cur->get_fn) 
	cur->get_fn = owl_variable_get_default;
      if (!cur->get_tostring_fn) 
	cur->get_tostring_fn = owl_variable_string_get_tostring_default;      
      if (!cur->free_fn) 
	cur->free_fn = owl_variable_free_default;
      cur->set_fn(cur, cur->pval_default);
      break;
    case OWL_VARIABLE_BOOL:
      if (!cur->validate_fn) 
	cur->validate_fn = owl_variable_bool_validate_default;
      if (!cur->set_fn) 
	cur->set_fn = owl_variable_bool_set_default;
      if (!cur->set_fromstring_fn) 
	cur->set_fromstring_fn = owl_variable_bool_set_fromstring_default;
      if (!cur->get_fn) 
	cur->get_fn = owl_variable_get_default;
      if (!cur->get_tostring_fn) 
	cur->get_tostring_fn = owl_variable_bool_get_tostring_default;      
      if (!cur->free_fn) 
	cur->free_fn = owl_variable_free_default;
      cur->val = owl_malloc(sizeof(int));
      cur->set_fn(cur, &cur->ival_default);
      break;
    case OWL_VARIABLE_INT:
      if (!cur->validate_fn) 
	cur->validate_fn = owl_variable_int_validate_default;
      if (!cur->set_fn) 
	cur->set_fn = owl_variable_int_set_default;
      if (!cur->set_fromstring_fn) 
	cur->set_fromstring_fn = owl_variable_int_set_fromstring_default;
      if (!cur->get_fn) 
	cur->get_fn = owl_variable_get_default;
      if (!cur->get_tostring_fn) 
	cur->get_tostring_fn = owl_variable_int_get_tostring_default;      
      if (!cur->free_fn) 
	cur->free_fn = owl_variable_free_default;
      cur->val = owl_malloc(sizeof(int));
      cur->set_fn(cur, &cur->ival_default);
      break;
    default:
      fprintf(stderr, "owl_variable_setup: invalid variable type\n");
      return(-2);
    }
    owl_dict_insert_element(vd, cur->name, cur, NULL);
  }
  return 0;
}

void owl_variable_dict_add_variable(owl_vardict * vardict,
                                    owl_variable * var) {
  owl_dict_insert_element(vardict, var->name, var, (void(*)(void*))owl_variable_free);
}

owl_variable * owl_variable_newvar(const char *name, const char *summary, const char * description) {
  owl_variable * var = owl_malloc(sizeof(owl_variable));
  memset(var, 0, sizeof(owl_variable));
  var->name = owl_strdup(name);
  var->summary = owl_strdup(summary);
  var->description = owl_strdup(description);
  return var;
}

void owl_variable_update(owl_variable *var, const char *summary, const char *desc) {
  if(var->summary) owl_free(var->summary);
  var->summary = owl_strdup(summary);
  if(var->description) owl_free(var->description);
  var->description = owl_strdup(desc);
}

void owl_variable_dict_newvar_string(owl_vardict * vd, const char *name, const char *summ, const char * desc, const char * initval) {
  owl_variable *old = owl_variable_get_var(vd, name, OWL_VARIABLE_STRING);
  if(old) {
    owl_variable_update(old, summ, desc);
    if(old->pval_default) owl_free(old->pval_default);
    old->pval_default = owl_strdup(initval);
  } else {
    owl_variable * var = owl_variable_newvar(name, summ, desc);
    var->type = OWL_VARIABLE_STRING;
    var->pval_default = owl_strdup(initval);
    var->set_fn = owl_variable_string_set_default;
    var->set_fromstring_fn = owl_variable_string_set_fromstring_default;
    var->get_fn = owl_variable_get_default;
    var->get_tostring_fn = owl_variable_string_get_tostring_default;
    var->free_fn = owl_variable_free_default;
    var->set_fn(var, initval);
    owl_variable_dict_add_variable(vd, var);
  }
}

void owl_variable_dict_newvar_int(owl_vardict * vd, const char *name, const char *summ, const char * desc, int initval) {
  owl_variable *old = owl_variable_get_var(vd, name, OWL_VARIABLE_INT);
  if(old) {
    owl_variable_update(old, summ, desc);
    old->ival_default = initval;
  } else {
    owl_variable * var = owl_variable_newvar(name, summ, desc);
    var->type = OWL_VARIABLE_INT;
    var->ival_default = initval;
    var->validate_fn = owl_variable_int_validate_default;
    var->set_fn = owl_variable_int_set_default;
    var->set_fromstring_fn = owl_variable_int_set_fromstring_default;
    var->get_fn = owl_variable_get_default;
    var->get_tostring_fn = owl_variable_int_get_tostring_default;
    var->free_fn = owl_variable_free_default;
    var->val = owl_malloc(sizeof(int));
    var->set_fn(var, &initval);
    owl_variable_dict_add_variable(vd, var);
  }
}

void owl_variable_dict_newvar_bool(owl_vardict * vd, const char *name, const char *summ, const char * desc, int initval) {
  owl_variable *old = owl_variable_get_var(vd, name, OWL_VARIABLE_BOOL);
  if(old) {
    owl_variable_update(old, summ, desc);
    old->ival_default = initval;
  } else {
    owl_variable * var = owl_variable_newvar(name, summ, desc);
    var->type = OWL_VARIABLE_BOOL;
    var->ival_default = initval;
    var->validate_fn = owl_variable_bool_validate_default;
    var->set_fn = owl_variable_bool_set_default;
    var->set_fromstring_fn = owl_variable_bool_set_fromstring_default;
    var->get_fn = owl_variable_get_default;
    var->get_tostring_fn = owl_variable_bool_get_tostring_default;
    var->free_fn = owl_variable_free_default;
    var->val = owl_malloc(sizeof(int));
    var->set_fn(var, &initval);
    owl_variable_dict_add_variable(vd, var);
  }
}

void owl_variable_dict_free(owl_vardict *d) {
  owl_dict_free_all(d, (void(*)(void*))owl_variable_free);
}

/* free the list with owl_variable_dict_namelist_free */
void owl_variable_dict_get_names(const owl_vardict *d, owl_list *l) {
  owl_dict_get_keys(d, l);
}

void owl_variable_dict_namelist_free(owl_list *l) {
  owl_list_free_all(l, owl_free);
}

void owl_variable_free(owl_variable *v) {
  if (v->free_fn) v->free_fn(v);
  owl_free(v);
}


const char *owl_variable_get_description(const owl_variable *v) {
  return v->description;
}

const char *owl_variable_get_summary(const owl_variable *v) {
  return v->summary;
}

const char *owl_variable_get_validsettings(const owl_variable *v) {
  if (v->validsettings) {
    return v->validsettings;
  } else {
    return "";
  }
}

/* functions for getting and setting variable values */

/* returns 0 on success, prints a status msg if msg is true */
int owl_variable_set_fromstring(owl_vardict *d, const char *name, const char *value, int msg, int requirebool) {
  owl_variable *v;
  char buff2[1024];
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL) {
    if (msg) owl_function_error("Unknown variable %s", name);
    return -1;
  }
  if (!v->set_fromstring_fn) {
    if (msg) owl_function_error("Variable %s is read-only", name);
    return -1;   
  }
  if (requirebool && v->type!=OWL_VARIABLE_BOOL) {
    if (msg) owl_function_error("Variable %s is not a boolean", name);
    return -1;   
  }
  if (0 != v->set_fromstring_fn(v, value)) {
    if (msg) owl_function_error("Unable to set %s (must be %s)", name, 
				  owl_variable_get_validsettings(v));
    return -1;
  }
  if (msg && v->get_tostring_fn) {
    v->get_tostring_fn(v, buff2, 1024, v->val);
    owl_function_makemsg("%s = '%s'", name, buff2);
  }    
  return 0;
}
 
int owl_variable_set_string(owl_vardict *d, const char *name, const char *newval) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->set_fn) return(-1);
  if (v->type!=OWL_VARIABLE_STRING) return(-1);
  return v->set_fn(v, newval);
}
 
int owl_variable_set_int(owl_vardict *d, const char *name, int newval) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->set_fn) return(-1);
  if (v->type!=OWL_VARIABLE_INT && v->type!=OWL_VARIABLE_BOOL) return(-1);
  return v->set_fn(v, &newval);
}
 
int owl_variable_set_bool_on(owl_vardict *d, const char *name) {
  return owl_variable_set_int(d,name,1);
}

int owl_variable_set_bool_off(owl_vardict *d, const char *name) {
  return owl_variable_set_int(d,name,0);
}

int owl_variable_get_tostring(const owl_vardict *d, const char *name, char *buf, int bufsize) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_tostring_fn) return(-1);
  return v->get_tostring_fn(v, buf, bufsize, v->val);
}

int owl_variable_get_default_tostring(const owl_vardict *d, const char *name, char *buf, int bufsize) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_tostring_fn) return(-1);
  if (v->type == OWL_VARIABLE_INT || v->type == OWL_VARIABLE_BOOL) {
    return v->get_tostring_fn(v, buf, bufsize, &(v->ival_default));
  } else {
    return v->get_tostring_fn(v, buf, bufsize, v->pval_default);
  }
}

owl_variable *owl_variable_get_var(const owl_vardict *d, const char *name, int require_type) {
  owl_variable *v;
  if (!name) return(NULL);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_fn || v->type != require_type) return(NULL);
  return v;
}

/* returns a reference */
const void *owl_variable_get(const owl_vardict *d, const char *name, int require_type) {
  owl_variable *v = owl_variable_get_var(d, name, require_type);
  if(v == NULL) return NULL;
  return v->get_fn(v);
}

/* returns a reference */
const char *owl_variable_get_string(const owl_vardict *d, const char *name) {
  return owl_variable_get(d,name, OWL_VARIABLE_STRING);
}

/* returns a reference */
const void *owl_variable_get_other(const owl_vardict *d, const char *name) {
  return owl_variable_get(d,name, OWL_VARIABLE_OTHER);
}

int owl_variable_get_int(const owl_vardict *d, const char *name) {
  const int *pi;
  pi = owl_variable_get(d,name,OWL_VARIABLE_INT);
  if (!pi) return(-1);
  return(*pi);
}

int owl_variable_get_bool(const owl_vardict *d, const char *name) {
  const int *pi;
  pi = owl_variable_get(d,name,OWL_VARIABLE_BOOL);
  if (!pi) return(-1);
  return(*pi);
}

void owl_variable_describe(const owl_vardict *d, const char *name, owl_fmtext *fm) {
  char defaultbuf[50];
  char buf[1024];
  int buflen = 1023;
  owl_variable *v;

  if (!name
      || (v = owl_dict_find_element(d, name)) == NULL 
      || !v->get_fn) {
    snprintf(buf, buflen, "     No such variable '%s'\n", name);     
    owl_fmtext_append_normal(fm, buf);
    return;
  }
  if (v->type == OWL_VARIABLE_INT || v->type == OWL_VARIABLE_BOOL) {
    v->get_tostring_fn(v, defaultbuf, 50, &(v->ival_default));
  } else {
    v->get_tostring_fn(v, defaultbuf, 50, v->pval_default);
  }
  snprintf(buf, buflen, OWL_TABSTR "%-20s - %s (default: '%s')\n", 
		  v->name, 
		  owl_variable_get_summary(v), defaultbuf);
  owl_fmtext_append_normal(fm, buf);
}

void owl_variable_get_help(const owl_vardict *d, const char *name, owl_fmtext *fm) {
  char buff[1024];
  int bufflen = 1023;
  owl_variable *v;

  if (!name
      || (v = owl_dict_find_element(d, name)) == NULL 
      || !v->get_fn) {
    owl_fmtext_append_normal(fm, "No such variable...\n");
    return;
  }

  owl_fmtext_append_bold(fm, "OWL VARIABLE\n\n");
  owl_fmtext_append_normal(fm, OWL_TABSTR);
  owl_fmtext_append_normal(fm, name);
  owl_fmtext_append_normal(fm, " - ");
  owl_fmtext_append_normal(fm, v->summary);
  owl_fmtext_append_normal(fm, "\n\n");

  owl_fmtext_append_normal(fm, "Current:        ");
  owl_variable_get_tostring(d, name, buff, bufflen);
  owl_fmtext_append_normal(fm, buff);
  owl_fmtext_append_normal(fm, "\n\n");


  if (v->type == OWL_VARIABLE_INT || v->type == OWL_VARIABLE_BOOL) {
    v->get_tostring_fn(v, buff, bufflen, &(v->ival_default));
  } else {
    v->get_tostring_fn(v, buff, bufflen, v->pval_default);
  }
  owl_fmtext_append_normal(fm, "Default:        ");
  owl_fmtext_append_normal(fm, buff);
  owl_fmtext_append_normal(fm, "\n\n");

  owl_fmtext_append_normal(fm, "Valid Settings: ");
  owl_fmtext_append_normal(fm, owl_variable_get_validsettings(v));
  owl_fmtext_append_normal(fm, "\n\n");

  if (v->description && *v->description) {
    owl_fmtext_append_normal(fm, "Description:\n");
    owl_fmtext_append_normal(fm, owl_variable_get_description(v));
    owl_fmtext_append_normal(fm, "\n\n");
  }
}




/**************************************************************************/
/*********************** GENERAL TYPE-SPECIFIC ****************************/
/**************************************************************************/

/* default common functions */

const void *owl_variable_get_default(const owl_variable *v) {
  return v->val;
}

void owl_variable_free_default(owl_variable *v) {
  if (v->val) owl_free(v->val);
}

/* default functions for booleans */

int owl_variable_bool_validate_default(const owl_variable *v, const void *newval) {
  if (newval == NULL) return(0);
  else if (*(const int*)newval==1 || *(const int*)newval==0) return(1);
  else return (0);
}

int owl_variable_bool_set_default(owl_variable *v, const void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  *(int*)v->val = *(const int*)newval;
  return(0);
}

int owl_variable_bool_set_fromstring_default(owl_variable *v, const char *newval) {
  int i;
  if (!strcmp(newval, "on")) i=1;
  else if (!strcmp(newval, "off")) i=0;
  else return(-1);
  return (v->set_fn(v, &i));
}

int owl_variable_bool_get_tostring_default(const owl_variable *v, char* buf, int bufsize, const void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else if (*(const int*)val == 0) {
    snprintf(buf, bufsize, "off");
    return 0;
  } else if (*(const int*)val == 1) {
    snprintf(buf, bufsize, "on");
    return 0;
  } else {
    snprintf(buf, bufsize, "<invalid>");
    return -1;
  }
}

/* default functions for integers */

int owl_variable_int_validate_default(const owl_variable *v, const void *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_int_set_default(owl_variable *v, const void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  *(int*)v->val = *(const int*)newval;
  return(0);
}

int owl_variable_int_set_fromstring_default(owl_variable *v, const char *newval) {
  int i;
  const char *ep = "x";
  i = strtol(newval, (char **)&ep, 10);
  if (*ep || ep==newval) return(-1);
  return (v->set_fn(v, &i));
}

int owl_variable_int_get_tostring_default(const owl_variable *v, char* buf, int bufsize, const void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else {
    snprintf(buf, bufsize, "%d", *(const int*)val);
    return 0;
  } 
}

/* default functions for enums (a variant of integers) */

int owl_variable_enum_validate(const owl_variable *v, const void *newval) {  
  char **enums;
  int nenums, val;
  if (newval == NULL) return(0);
  enums = atokenize(v->validsettings, ",", &nenums);
  if (enums == NULL) return(0);
  atokenize_free(enums, nenums);
  val = *(const int*)newval;
  if (val < 0 || val >= nenums) {
    return(0);
  }
  return(1);
}

int owl_variable_enum_set_fromstring(owl_variable *v, const char *newval) {
  char **enums;
  int nenums, i, val=-1;
  if (newval == NULL) return(-1);
  enums = atokenize(v->validsettings, ",", &nenums);
  if (enums == NULL) return(-1);
  for (i=0; i<nenums; i++) {
    if (0==strcmp(newval, enums[i])) {
      val = i;
    }
  }
  atokenize_free(enums, nenums);
  if (val == -1) return(-1);
  return (v->set_fn(v, &val));
}

int owl_variable_enum_get_tostring(const owl_variable *v, char* buf, int bufsize, const void *val) {
  char **enums;
  int nenums, i;

  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  }
  enums = atokenize(v->validsettings, ",", &nenums);
  i = *(const int*)val;
  if (i<0 || i>=nenums) {
    snprintf(buf, bufsize, "<invalid:%d>",i);
    atokenize_free(enums, nenums);
    return(-1);
  }
  snprintf(buf, bufsize, "%s", enums[i]);
  return 0;
}

/* default functions for stringeans */

int owl_variable_string_validate_default(const struct _owl_variable *v, const void *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_string_set_default(owl_variable *v, const void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  if (v->val) owl_free(v->val);
  v->val = owl_strdup(newval);
  return(0);
}

int owl_variable_string_set_fromstring_default(owl_variable *v, const char *newval) {
  return (v->set_fn(v, newval));
}

int owl_variable_string_get_tostring_default(const owl_variable *v, char* buf, int bufsize, const void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else {
    snprintf(buf, bufsize, "%s", (const char*)val);
    return 0;
  }
}

