#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "owl.h"

static const char fileIdent[] = "$Id$";

static int in_regtest = 0;

#define OWLVAR_BOOL(name,default,summary,description) \
        { name, OWL_VARIABLE_BOOL, NULL, default, "on,off", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_BOOL_FULL(name,default,summary,description,validate,set,get) \
        { name, OWL_VARIABLE_BOOL, NULL, default, "on,off", summary,description, NULL, \
        validate, set, NULL, get, NULL }

#define OWLVAR_INT(name,default,summary,description) \
        { name, OWL_VARIABLE_INT, NULL, default, "<int>", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_INT_FULL(name,default,summary,description,validset,validate,set,get) \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        validate, set, NULL, get, NULL, NULL }

#define OWLVAR_PATH(name,default,summary,description) \
        { name, OWL_VARIABLE_STRING, default, 0, "<path>", summary,description,  NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_STRING(name,default,summary,description) \
        { name, OWL_VARIABLE_STRING, default, 0, "<string>", summary,description, NULL, \
        NULL, NULL, NULL, NULL, NULL, NULL }

#define OWLVAR_STRING_FULL(name,default,summary,description,validate,set,get) \
        { name, OWL_VARIABLE_STRING, default, 0, "<string>", summary,description, NULL, \
        validate, set, NULL, get, NULL, NULL }

/* enums are really integers, but where validset is a comma-separated
 * list of strings which can be specified.  The tokens, starting at 0,
 * correspond to the values that may be specified. */
#define OWLVAR_ENUM(name,default,summary,description,validset) \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        owl_variable_enum_validate, \
        NULL, owl_variable_enum_set_fromstring, \
        NULL, owl_variable_enum_get_tostring, \
        NULL }

#define OWLVAR_ENUM_FULL(name,default,summary,description,validset,validate, set, get) \
        { name, OWL_VARIABLE_INT, NULL, default, validset, summary,description, NULL, \
        validate, \
        set, owl_variable_enum_set_fromstring, \
        get, owl_variable_enum_get_tostring, \
        NULL }

static owl_variable variables_to_init[] = {

  OWLVAR_BOOL( "personalbell" /* %OwlVarStub */, 0,
	       "ring the terminal bell when personal messages are received",
	       "" ),

  OWLVAR_BOOL( "bell" /* %OwlVarStub */, 1,
	       "enable / disable the terminal bell", "" ),

  OWLVAR_BOOL_FULL( "debug" /* %OwlVarStub */, OWL_DEBUG,
		    "whether debugging is enabled",
		    "If set to 'on', debugging messages are logged to the\n"
		    "file specified by the debugfile variable.\n",
		    NULL, owl_variable_debug_set, NULL),

  OWLVAR_BOOL( "startuplogin" /* %OwlVarStub */, 1,
	       "send a login message when owl starts", "" ),

  OWLVAR_BOOL( "shutdownlogout" /* %OwlVarStub */, 1,
	       "send a logout message when owl exits", "" ),

  OWLVAR_BOOL( "rxping" /* %OwlVarStub */, 0,
	       "display received pings", "" ),

  OWLVAR_BOOL( "txping" /* %OwlVarStub */, 1,
	       "send pings", "" ),

  OWLVAR_BOOL( "smartstrip" /* %OwlVarStub */, 1,
	       "strip kerberos instance for reply", ""),

  OWLVAR_BOOL( "displayoutgoing" /* %OwlVarStub */, 1,
	       "display outgoing messages", "" ),

  OWLVAR_BOOL( "loginsubs" /* %OwlVarStub */, 1,
	       "load logins from .anyone on startup", "" ),

  OWLVAR_BOOL( "logging" /* %OwlVarStub */, 0,
	       "turn personal logging on or off", 
	       "If this is set to on, personal messages are\n"
	       "logged in the directory specified\n"
	       "by the 'logpath' variable.  The filename in that\n"
	       "directory is derived from the sender of the message.\n" ),

  OWLVAR_BOOL( "classlogging" /* %OwlVarStub */, 0,
	       "turn class logging on or off",
	       "If this is set to on, class messages are\n"
	       "logged in the directory specified\n"
	       "by the 'classlogpath' variable.\n" 
	       "The filename in that directory is derived from\n"
	       "the name of the class to which the message was sent.\n" ),

  OWLVAR_BOOL( "colorztext" /* %OwlVarStub */, 1,
	       "allow @color() in zephyrs to change color",
	       "Note that only messages received after this variable\n"
	       "is set will be affected." ),

  OWLVAR_ENUM_FULL( "disable-ctrl-d" /* %OwlVarStub:lockout_ctrld */, 1,
		    "don't send zephyrs on C-d",
		    "If set to 'off', C-d won't send a zephyr from the edit\n"
		    "window.  If set to 'on', C-d will always send a zephyr\n"
		    "being composed in the edit window.  If set to 'middle',\n"
		    "C-d will only ever send a zephyr if the cursor is at\n"
		    "the end of the message being composed.\n\n"
		    "Note that this works by changing the C-d keybinding\n"
		    "in the editmulti keymap.\n",
		    "off,middle,on",
		    NULL, owl_variable_disable_ctrl_d_set, NULL),

  OWLVAR_BOOL( "_burningears" /* %OwlVarStub:burningears */, 0,
	       "[NOT YET IMPLEMENTED] beep on messages matching patterns", "" ),

  OWLVAR_BOOL( "_summarymode" /* %OwlVarStub:summarymode */, 0,
	       "[NOT YET IMPLEMENTED]", "" ),

  OWLVAR_PATH( "logpath" /* %OwlVarStub */, "~/zlog/people",
	       "path for logging personal zephyrs", 
	       "Specifies a directory which must exist.\n"
	       "Files will be created in the directory for each sender.\n"),

  OWLVAR_PATH( "classlogpath" /* %OwlVarStub:classlogpath */, "~/zlog/class",
	       "path for logging class zephyrs",
	       "Specifies a directory which must exist.\n"
	       "Files will be created in the directory for each class.\n"),

  OWLVAR_PATH( "debug_file" /* %OwlVarStub */, OWL_DEBUG_FILE,
	       "path for logging debug messages when debugging is enabled",
	       "This file will be logged to if 'debug' is set to 'on'.\n"),
  
  OWLVAR_PATH( "zsigproc" /* %OwlVarStub:zsig_exec */, NULL,
	       "name of a program to run that will generate zsigs",
	       "This program should produce a zsig on stdout when run.\n"
	       "Note that it is important that this program not block.\n" ),

  OWLVAR_PATH( "newmsgproc" /* %OwlVarStub:newmsgproc */, NULL,
	       "name of a program to run when new messages are present",
	       "The named program will be run when owl recevies new.\n"
	       "messages.  It will not be run again until the first\n"
	       "instance exits"),

  OWLVAR_STRING( "zsig" /* %OwlVarStub */, "",
	         "zephyr signature", 
		 "If 'zsigproc' is not set, this string will be used\n"
		 "as a zsig.  If this is also unset, the 'zwrite-signature'\n"
		 "zephyr variable will be used instead.\n"),

  OWLVAR_STRING( "appendtosepbar" /* %OwlVarStub */, "",
	         "string to append to the end of the sepbar",
		 "The sepbar is the bar separating the top and bottom\n"
		 "of the owl screen.  Any string specified here will\n"
		 "be displayed on the right of the sepbar\n"),

  OWLVAR_BOOL( "zaway" /* %OwlVarStub */, 0,
	       "turn zaway on or off", "" ),

  OWLVAR_STRING( "zaway_msg" /* %OwlVarStub */, 
		 OWL_DEFAULT_ZAWAYMSG,
	         "zaway msg for responding to zephyrs when away", "" ),

  OWLVAR_STRING( "zaway_msg_default" /* %OwlVarStub */, 
		 OWL_DEFAULT_ZAWAYMSG,
	         "default zaway message", "" ),

  OWLVAR_STRING( "view_home" /* %OwlVarStub */, "all",
	         "home view to switch to after 'X' and 'V'", 
		 "SEE ALSO: view, filter\n" ),

  OWLVAR_STRING_FULL( "tty" /* %OwlVarStub */, "", "tty name for zephyr location", "",
		      NULL, owl_variable_tty_set, NULL),
  
  OWLVAR_INT(    "edit:maxfillcols" /* %OwlVarStub:edit_maxfillcols */, 70,
	         "maximum number of columns for M-q to fill text to",
		 "This specifies the maximum number of columns for M-q\n"
		 "to fill text to.  If set to 0, ther will be no maximum\n"
		 "limit.  In all cases, the current width of the screen\n"
		 "will also be taken into account.  It will be used instead\n"
		 "if it is narrower than the maximum, or if this\n"
		 "is set to 0.\n" ),

  OWLVAR_INT(    "edit:maxwrapcols" /* %OwlVarStub:edit_maxwrapcols */, 0,
	         "maximum number of columns for line-wrapping",
		 "This specifies the maximum number of columns for\n"
		 "auto-line-wrapping.  If set to 0, ther will be no maximum\n"
		 "limit.  In all cases, the current width of the screen\n"
		 "will also be taken into account.  It will be used instead\n"
		 "if it is narrower than the maximum, or if this\n"
		 "is set to 0.\n\n"
		 "It is recommended that outgoing messages be no wider\n"
		 "than 60 columns, as a courtesy to recipients.\n"),

  OWLVAR_INT_FULL( "typewinsize" /* %OwlVarStub:typwin_lines */, 
		   OWL_TYPWIN_SIZE,
		  "number of lines in the typing window", 
		   "This specifies the height of the window at the\n"
		   "bottom of the screen where commands are entered\n"
		   "and where messages are composed.\n",
		   "int > 0",
		   owl_variable_int_validate_gt0,
		   owl_variable_typewinsize_set,
		   NULL /* use default for get */
		   ),

  OWLVAR_ENUM( "scrollmode" /* %OwlVarStub */, OWL_SCROLLMODE_NORMAL,
	       "how to scroll up and down",
	       "This controls how the screen is scrolled as the\n"
	       "cursor moves between messages being displayed.\n"
	       "The following modes are supported:\n\n"
	       "   normal      - This is the owl default.  Scrolling happens\n"
	       "                 when it needs to, and an attempt is made to\n"
	       "                 keep the current message roughly near\n"
	       "                 the middle of the screen.\n"
	       "   top         - The current message will always be the\n"
	       "                 the top message displayed.\n"
	       "   neartop     - The current message will be one down\n"
	       "                 from the top message displayed,\n"
	       "                 where possible.\n"
	       "   center      - An attempt is made to keep the current\n"
	       "                 message near the center of the screen.\n"
	       "   paged       - The top message displayed only changes\n"
	       "                 when user moves the cursor to the top\n"
	       "                 or bottom of the screen.  When it moves,\n"
	       "                 the screen will be paged up or down and\n"
	       "                 the cursor will be near the top or\n"
	       "                 the bottom.\n"
	       "   pagedcenter - The top message displayed only changes\n"
	       "                 when user moves the cursor to the top\n"
	       "                 or bottom of the screen.  When it moves,\n"
	       "                 the screen will be paged up or down and\n"
	       "                 the cursor will be near the center.\n",
	       "normal,top,neartop,center,paged,pagedcenter" ),

  OWLVAR_ENUM( "webbrowser" /* %OwlVarStub */, OWL_WEBBROWSER_NETSCAPE,
	       "web browser to use to launch URLs",
	       "When the 'w' key is pressed, this browser is used\n"
	       "to display the requested URL.\n",
	       "none,netscape,galeon,opera" ),

  OWLVAR_BOOL( "_followlast" /* %OwlVarStub */, 0,
	       "enable automatic following of the last zephyr",
	       "If the cursor is at the last message, it will\n"
	       "continue to follow the last message if this is set.\n"
	       "Note that this is currently risky as you might accidentally\n"
	       "delete a message right as it came in.\n" ),

  /* This MUST be last... */
  { NULL, 0, NULL, 0, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL }

};

/**************************************************************************/
/*********************** SPECIFIC TO VARIABLES ****************************/
/**************************************************************************/


/* commonly useful */

int owl_variable_int_validate_gt0(owl_variable *v, void *newval) {
  if (newval == NULL) return(0);
  else if (*(int*)newval < 1) return(0);
  else return (1);
}

int owl_variable_int_validate_positive(owl_variable *v, void *newval) {
  if (newval == NULL) return(0);
  else if (*(int*)newval < 0) return(0);
  else return (1);
}

/* typewinsize */

int owl_variable_typewinsize_set(owl_variable *v, void *newval) {
  int rv;
  rv = owl_variable_int_set_default(v, newval);
  if (0 == rv) owl_function_resize();
  return(rv);
}

/* debug (cache value in g->debug) */

int owl_variable_debug_set(owl_variable *v, void *newval) {
  if (newval && (*(int*)newval == 1 || *(int*)newval == 0)) {
    g.debug = *(int*)newval;
  }
  return owl_variable_bool_set_default(v, newval);
}

/* note that changing the value of this will clobber 
 * any user setting of this */
int owl_variable_disable_ctrl_d_set(owl_variable *v, void *newval) {

  if (in_regtest) return owl_variable_int_set_default(v, newval);

  if (newval && !owl_context_is_startup(owl_global_get_context(&g))) {
    if (*(int*)newval == 2) {
      owl_function_command_norv("bindkey editmulti C-d command edit:delete-next-char");
    } else if (*(int*)newval == 1) {
      owl_function_command_norv("bindkey editmulti C-d command editmulti:done-or-delete");
    } else {
      owl_function_command_norv("bindkey editmulti C-d command editmulti:done");
    }
  }  
  return owl_variable_int_set_default(v, newval);  
}

int owl_variable_tty_set(owl_variable *v, void *newval) {
  ZInitLocationInfo(owl_global_get_hostname(&g), newval);
  return(owl_variable_string_set_default(v, newval));
}


/**************************************************************************/
/****************************** GENERAL ***********************************/
/**************************************************************************/

int owl_variable_dict_setup(owl_vardict *vd) {
  owl_variable *cur;
  if (owl_dict_create(vd)) return(-1);
  for (cur = variables_to_init; cur->name != NULL; cur++) {
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
    owl_dict_insert_element(vd, cur->name, (void*)cur, NULL);
  }
  return 0;
}

void owl_variable_dict_free(owl_vardict *d) {
  owl_dict_free_all(d, (void(*)(void*))owl_variable_free);
}

/* free the list with owl_variable_dict_namelist_free */
void owl_variable_dict_get_names(owl_vardict *d, owl_list *l) {
  owl_dict_get_keys(d, l);
}

void owl_variable_dict_namelist_free(owl_list *l) {
  owl_list_free_all(l, owl_free);
}

void owl_variable_free(owl_variable *v) {
  if (v->free_fn) v->free_fn(v);
}


char *owl_variable_get_description(owl_variable *v) {
  return v->description;
}

char *owl_variable_get_summary(owl_variable *v) {
  return v->summary;
}

char *owl_variable_get_validsettings(owl_variable *v) {
  if (v->validsettings) {
    return v->validsettings;
  } else {
    return "";
  }
}

/* functions for getting and setting variable values */

/* returns 0 on success, prints a status msg if msg is true */
int owl_variable_set_fromstring(owl_vardict *d, char *name, char *value, int msg, int requirebool) {
  owl_variable *v;
  char buff2[1024];
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL) {
    if (msg) owl_function_makemsg("Unknown variable %s", name);
    return -1;
  }
  if (!v->set_fromstring_fn) {
    if (msg) owl_function_makemsg("Variable %s is read-only", name);
    return -1;   
  }
  if (requirebool && v->type!=OWL_VARIABLE_BOOL) {
    if (msg) owl_function_makemsg("Variable %s is not a boolean", name);
    return -1;   
  }
  if (0 != v->set_fromstring_fn(v, value)) {
    if (msg) owl_function_makemsg("Unable to set %s (must be %s)", name, 
				  owl_variable_get_validsettings(v));
    return -1;
  }
  if (msg && v->get_tostring_fn) {
    v->get_tostring_fn(v, buff2, 1024, v->val);
    owl_function_makemsg("%s = '%s'", name, buff2);
  }    
  return 0;
}
 
int owl_variable_set_string(owl_vardict *d, char *name, char *newval) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->set_fn) return(-1);
  if (v->type!=OWL_VARIABLE_STRING) return(-1);
  return v->set_fn(v, (void*)newval);
}
 
int owl_variable_set_int(owl_vardict *d, char *name, int newval) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->set_fn) return(-1);
  if (v->type!=OWL_VARIABLE_INT && v->type!=OWL_VARIABLE_BOOL) return(-1);
  return v->set_fn(v, &newval);
}
 
int owl_variable_set_bool_on(owl_vardict *d, char *name) {
  return owl_variable_set_int(d,name,1);
}

int owl_variable_set_bool_off(owl_vardict *d, char *name) {
  return owl_variable_set_int(d,name,0);
}

int owl_variable_get_tostring(owl_vardict *d, char *name, char *buf, int bufsize) {
  owl_variable *v;
  if (!name) return(-1);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_tostring_fn) return(-1);
  return v->get_tostring_fn(v, buf, bufsize, v->val);
}

int owl_variable_get_default_tostring(owl_vardict *d, char *name, char *buf, int bufsize) {
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

/* returns a reference */
void *owl_variable_get(owl_vardict *d, char *name, int require_type) {
  owl_variable *v;
  if (!name) return(NULL);
  v = owl_dict_find_element(d, name);
  if (v == NULL || !v->get_fn || v->type != require_type) return(NULL);
  return v->get_fn(v);
}

/* returns a reference */
char *owl_variable_get_string(owl_vardict *d, char *name) {
  return (char*)owl_variable_get(d,name, OWL_VARIABLE_STRING);
}

/* returns a reference */
void *owl_variable_get_other(owl_vardict *d, char *name) {
  return (char*)owl_variable_get(d,name, OWL_VARIABLE_OTHER);
}

int owl_variable_get_int(owl_vardict *d, char *name) {
  int *pi;
  pi = (int*)owl_variable_get(d,name,OWL_VARIABLE_INT);
  if (!pi) return(-1);
  return(*pi);
}

int owl_variable_get_bool(owl_vardict *d, char *name) {
  int *pi;
  pi = (int*)owl_variable_get(d,name,OWL_VARIABLE_BOOL);
  if (!pi) return(-1);
  return(*pi);
}

void owl_variable_describe(owl_vardict *d, char *name, owl_fmtext *fm) {
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

void owl_variable_get_help(owl_vardict *d, char *name, owl_fmtext *fm) {
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

void *owl_variable_get_default(owl_variable *v) {
  return v->val;
}

void owl_variable_free_default(owl_variable *v) {
  if (v->val) owl_free(v->val);
}

/* default functions for booleans */

int owl_variable_bool_validate_default(owl_variable *v, void *newval) {
  if (newval == NULL) return(0);
  else if (*(int*)newval==1 || *(int*)newval==0) return(1);
  else return (0);
}

int owl_variable_bool_set_default(owl_variable *v, void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  *(int*)v->val = *(int*)newval;
  return(0);
}

int owl_variable_bool_set_fromstring_default(owl_variable *v, char *newval) {
  int i;
  if (!strcmp(newval, "on")) i=1;
  else if (!strcmp(newval, "off")) i=0;
  else return(-1);
  return (v->set_fn(v, &i));
}

int owl_variable_bool_get_tostring_default(owl_variable *v, char* buf, int bufsize, void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else if (*(int*)val == 0) {
    snprintf(buf, bufsize, "off");
    return 0;
  } else if (*(int*)val == 1) {
    snprintf(buf, bufsize, "on");
    return 0;
  } else {
    snprintf(buf, bufsize, "<invalid>");
    return -1;
  }
}

/* default functions for integers */

int owl_variable_int_validate_default(owl_variable *v, void *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_int_set_default(owl_variable *v, void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  *(int*)v->val = *(int*)newval;
  return(0);
}

int owl_variable_int_set_fromstring_default(owl_variable *v, char *newval) {
  int i;
  char *ep = "x";
  i = strtol(newval, &ep, 10);
  if (*ep || ep==newval) return(-1);
  return (v->set_fn(v, &i));
}

int owl_variable_int_get_tostring_default(owl_variable *v, char* buf, int bufsize, void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else {
    snprintf(buf, bufsize, "%d", *(int*)val);
    return 0;
  } 
}

/* default functions for enums (a variant of integers) */

int owl_variable_enum_validate(owl_variable *v, void *newval) {  
  char **enums;
  int nenums, val;
  if (newval == NULL) return(0);
  enums = atokenize(v->validsettings, ",", &nenums);
  if (enums == NULL) return(0);
  atokenize_free(enums, nenums);
  val = *(int*)newval;
  if (val < 0 || val >= nenums) {
    return(0);
  }
  return(1);
}

int owl_variable_enum_set_fromstring(owl_variable *v, char *newval) {
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

int owl_variable_enum_get_tostring(owl_variable *v, char* buf, int bufsize, void *val) {
  char **enums;
  int nenums, i;

  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  }
  enums = atokenize(v->validsettings, ",", &nenums);
  i = *(int*)val;
  if (i<0 || i>=nenums) {
    snprintf(buf, bufsize, "<invalid:%d>",i);
    atokenize_free(enums, nenums);
    return(-1);
  }
  snprintf(buf, bufsize, "%s", enums[i]);
  return 0;
}

/* default functions for stringeans */

int owl_variable_string_validate_default(struct _owl_variable *v, void *newval) {
  if (newval == NULL) return(0);
  else return (1);
}

int owl_variable_string_set_default(owl_variable *v, void *newval) {
  if (v->validate_fn) {
    if (!v->validate_fn(v, newval)) return(-1);
  }
  if (v->val) owl_free(v->val);
  v->val = owl_strdup(newval);
  return(0);
}

int owl_variable_string_set_fromstring_default(owl_variable *v, char *newval) {
  return (v->set_fn(v, newval));
}

int owl_variable_string_get_tostring_default(owl_variable *v, char* buf, int bufsize, void *val) {
  if (val == NULL) {
    snprintf(buf, bufsize, "<null>");
    return -1;
  } else {
    snprintf(buf, bufsize, "%s", (char*)val);
    return 0;
  }
}



/**************************************************************************/
/************************* REGRESSION TESTS *******************************/
/**************************************************************************/

#ifdef OWL_INCLUDE_REG_TESTS

#define FAIL_UNLESS(desc,pred) printf("\t%-4s: %s\n", (pred)?"ok":(numfailed++,"FAIL"), desc)

int owl_variable_regtest(void) {
  owl_vardict vd;
  int numfailed=0;
  char buf[1024];

  in_regtest = 1;

  printf("BEGIN testing owl_variable\n");
  FAIL_UNLESS("setup", 0==owl_variable_dict_setup(&vd));

  FAIL_UNLESS("get bool", 0==owl_variable_get_bool(&vd,"personalbell"));
  FAIL_UNLESS("get bool (no such)", -1==owl_variable_get_bool(&vd,"mumble"));
  FAIL_UNLESS("get bool as string 1", 0==owl_variable_get_tostring(&vd,"personalbell", buf, 1024));
  FAIL_UNLESS("get bool as string 2", 0==strcmp(buf,"off"));
  FAIL_UNLESS("set bool 1", 0==owl_variable_set_bool_on(&vd,"personalbell"));
  FAIL_UNLESS("get bool 2", 1==owl_variable_get_bool(&vd,"personalbell"));
  FAIL_UNLESS("set bool 3", 0==owl_variable_set_fromstring(&vd,"personalbell","off",0,0));
  FAIL_UNLESS("get bool 4", 0==owl_variable_get_bool(&vd,"personalbell"));
  FAIL_UNLESS("set bool 5", -1==owl_variable_set_fromstring(&vd,"personalbell","xxx",0,0));
  FAIL_UNLESS("get bool 6", 0==owl_variable_get_bool(&vd,"personalbell"));


  FAIL_UNLESS("get string", 0==strcmp("~/zlog/people", owl_variable_get_string(&vd,"logpath")));
  FAIL_UNLESS("set string 7", 0==owl_variable_set_string(&vd,"logpath","whee"));
  FAIL_UNLESS("get string", 0==strcmp("whee", owl_variable_get_string(&vd,"logpath")));

  FAIL_UNLESS("get int", 8==owl_variable_get_int(&vd,"typewinsize"));
  FAIL_UNLESS("get int (no such)", -1==owl_variable_get_int(&vd,"mmble"));
  FAIL_UNLESS("get int as string 1", 0==owl_variable_get_tostring(&vd,"typewinsize", buf, 1024));
  FAIL_UNLESS("get int as string 2", 0==strcmp(buf,"8"));
  FAIL_UNLESS("set int 1", 0==owl_variable_set_int(&vd,"typewinsize",12));
  FAIL_UNLESS("get int 2", 12==owl_variable_get_int(&vd,"typewinsize"));
  FAIL_UNLESS("set int 1b", -1==owl_variable_set_int(&vd,"typewinsize",-3));
  FAIL_UNLESS("get int 2b", 12==owl_variable_get_int(&vd,"typewinsize"));
  FAIL_UNLESS("set int 3", 0==owl_variable_set_fromstring(&vd,"typewinsize","9",0,0));
  FAIL_UNLESS("get int 4", 9==owl_variable_get_int(&vd,"typewinsize"));
  FAIL_UNLESS("set int 5", -1==owl_variable_set_fromstring(&vd,"typewinsize","xxx",0,0));
  FAIL_UNLESS("set int 6", -1==owl_variable_set_fromstring(&vd,"typewinsize","",0,0));
  FAIL_UNLESS("get int 7", 9==owl_variable_get_int(&vd,"typewinsize"));

  FAIL_UNLESS("get enum", OWL_WEBBROWSER_NETSCAPE==owl_variable_get_int(&vd,"webbrowser"));
  FAIL_UNLESS("get enum as string 1", 0==owl_variable_get_tostring(&vd,"webbrowser", buf, 1024));
  FAIL_UNLESS("get enum as string 2", 0==strcmp(buf,"netscape"));
  FAIL_UNLESS("set enum 1", 0==owl_variable_set_int(&vd,"webbrowser",OWL_WEBBROWSER_GALEON));
  FAIL_UNLESS("get enum 2", OWL_WEBBROWSER_GALEON==owl_variable_get_int(&vd,"webbrowser"));
  FAIL_UNLESS("set enum 1b", -1==owl_variable_set_int(&vd,"webbrowser",-3));
  FAIL_UNLESS("set enum 1b", -1==owl_variable_set_int(&vd,"webbrowser",209));
  FAIL_UNLESS("get enum 2b", OWL_WEBBROWSER_GALEON==owl_variable_get_int(&vd,"webbrowser"));
  FAIL_UNLESS("set enum 3", 0==owl_variable_set_fromstring(&vd,"webbrowser","none",0,0));
  FAIL_UNLESS("get enum 4", OWL_WEBBROWSER_NONE==owl_variable_get_int(&vd,"webbrowser"));
  FAIL_UNLESS("set enum 5", 0==owl_variable_set_fromstring(&vd,"webbrowser","netscape",0,0));
  FAIL_UNLESS("get enum 6", OWL_WEBBROWSER_NETSCAPE==owl_variable_get_int(&vd,"webbrowser"));
  FAIL_UNLESS("set enum 7", -1==owl_variable_set_fromstring(&vd,"webbrowser","xxx",0,0));
  FAIL_UNLESS("set enum 8", -1==owl_variable_set_fromstring(&vd,"webbrowser","",0,0));
  FAIL_UNLESS("set enum 9", -1==owl_variable_set_fromstring(&vd,"webbrowser","netscapey",0,0));
  FAIL_UNLESS("get enum 10", OWL_WEBBROWSER_NETSCAPE==owl_variable_get_int(&vd,"webbrowser"));



  owl_variable_dict_free(&vd);

  if (numfailed) printf("*** WARNING: failures encountered with owl_variable\n");
  printf("END testing owl_variable (%d failures)\n", numfailed);
  return(numfailed);
}


#endif /* OWL_INCLUDE_REG_TESTS */
