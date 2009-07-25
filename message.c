#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define OWL_PERL
#include "owl.h"

static const char fileIdent[] = "$Id$";

static owl_fmtext_cache fmtext_cache[OWL_FMTEXT_CACHE_SIZE];
static owl_fmtext_cache * fmtext_cache_next = fmtext_cache;

owl_message *owl_message_new() {
  return (owl_message*)owl_perl_new("BarnOwl::Message");
}

void owl_message_lock(owl_message *m)
{
  char *rv = owl_perlconfig_message_call_method(m, "lock_message", 0, NULL);
  if(rv) owl_free(rv);
}

void owl_message_init(owl_message *m)
{
}

SV* owl_message_get_attribute_internal(owl_message *m, char *attrname)
{
  HV * hash = (HV*)SvRV((SV*)m);
  SV **attr = hv_fetch(hash, attrname, strlen(attrname), 0);
  if(attr == NULL)
    return NULL;
  return *attr;
}

/* add the named attribute to the message.  If an attribute with the
 * name already exists, replace the old value with the new value
 */
void owl_message_set_attribute(owl_message *m, char *attrname, char *attrvalue)
{
  char *argv[] = {attrname, attrvalue, NULL};
  char *rv = owl_perlconfig_message_call_method(m, "__set_attribute", 2, argv);
  if(rv) owl_free(rv);
}

/* return the value associated with the named attribute, or NULL if
 * the attribute does not exist
 */
char *owl_message_get_attribute_value(owl_message *m, char *attrname)
{
  /*
   * Horrible kludge for historical reasons.
   *
   * Stock owl stores a ``isprivate'' field in C code, and a
   * ``private'' field in perl, and converts between them when
   * marshalling. Since perl and C now read the same objects, we unify
   * on ``private'', but need to provide support for old filters which
   * use ``isprivate''.
   *
   * This kludge could arguably live in filter.c instead.
   */
  if(!strcmp(attrname, "isprivate")) attrname = "private";
  SV *attr = owl_message_get_attribute_internal(m, attrname);
  if(attr == NULL || !SvOK(attr)) return NULL;
  return SvPV_nolen(attr);
}

char *owl_message_get_meta(owl_message *m, char *attr)
{
  char *argv[] = {attr};
  char *perlrv;
  char *str;
  perlrv = owl_perlconfig_message_call_method(m, "get_meta", 1, argv);
  if(!perlrv) {
    return "";
  }
  str = SvPV_nolen(sv_2mortal(newSVpv(perlrv, 0)));
  owl_free(perlrv);
  return str;
}

char *owl_message_get_attribute_value_nonull(owl_message *m, char *attrname)
{
  char *att = owl_message_get_attribute_value(m, attrname);
  if(att == NULL) return "";
  return att;
}

int owl_message_get_attribute_int(owl_message *m, char *attrname)
{
  SV* attr = owl_message_get_attribute_internal(m, attrname);
  if(attr == NULL)
    return 0;
  return SvIV(attr);
}

int owl_message_get_id(owl_message *m) {
  return owl_message_get_attribute_int(m, "id");
}

void owl_message_set_class(owl_message *m, char *class)
{
  owl_message_set_attribute(m, "class", class);
}

char *owl_message_get_class(owl_message *m)
{
  char *class;

  class=owl_message_get_attribute_value(m, "class");
  if (!class) return("");
  return(class);
}

void owl_message_set_instance(owl_message *m, char *inst)
{
  owl_message_set_attribute(m, "instance", inst);
}

char *owl_message_get_instance(owl_message *m)
{
  char *instance;

  instance=owl_message_get_attribute_value(m, "instance");
  if (!instance) return("");
  return(instance);
}

void owl_message_set_sender(owl_message *m, char *sender)
{
  owl_message_set_attribute(m, "sender", sender);
}

char *owl_message_get_sender(owl_message *m)
{
  char *sender;

  sender=owl_message_get_attribute_value(m, "sender");
  if (!sender) return("");
  return(sender);
}

void owl_message_set_zsig(owl_message *m, char *zsig)
{
  owl_message_set_attribute(m, "zsig", zsig);
}

char *owl_message_get_zsig(owl_message *m)
{
  char *zsig;

  zsig=owl_message_get_attribute_value(m, "zsig");
  if (!zsig) return("");
  return(zsig);
}

void owl_message_set_recipient(owl_message *m, char *recip)
{
  owl_message_set_attribute(m, "recipient", recip);
}

char *owl_message_get_recipient(owl_message *m)
{
  /* this is stupid for outgoing messages, we need to fix it. */

  char *recip;

  recip=owl_message_get_attribute_value(m, "recipient");
  if (!recip) return("");
  return(recip);
}

void owl_message_set_realm(owl_message *m, char *realm)
{
  owl_message_set_attribute(m, "realm", realm);
}

char *owl_message_get_realm(owl_message *m)
{
  char *realm;
  
  realm=owl_message_get_attribute_value(m, "realm");
  if (!realm) return("");
  return(realm);
}

void owl_message_set_body(owl_message *m, char *body)
{
  owl_message_set_attribute(m, "body", body);
}

char *owl_message_get_body(owl_message *m)
{
  char *body;

  body=owl_message_get_attribute_value(m, "body");
  if (!body) return("");
  return(body);
}


void owl_message_set_opcode(owl_message *m, char *opcode)
{
  owl_message_set_attribute(m, "opcode", opcode);
}

char *owl_message_get_opcode(owl_message *m)
{
  char *opcode;

  opcode=owl_message_get_attribute_value(m, "opcode");
  if (!opcode) return("");
  return(opcode);
}


void owl_message_set_islogin(owl_message *m)
{
  owl_message_set_attribute(m, "login", "login");
}


void owl_message_set_islogout(owl_message *m)
{
  owl_message_set_attribute(m, "login", "logout");
}

int owl_message_is_loginout(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "login");
  if(!res) return(0);
  return strcmp(res, "none");
}

int owl_message_is_login(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "login");
  if (!res) return(0);
  if (!strcmp(res, "login")) return(1);
  return(0);
}


int owl_message_is_logout(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "login");
  if (!res) return(0);
  if (!strcmp(res, "logout")) return(1);
  return(0);
}

void owl_message_set_isprivate(owl_message *m)
{
  owl_message_set_attribute(m, "private", "true");
}

int owl_message_is_private(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "private");
  if (!res) return(0);
  return !strcmp(res, "true");
}

/*
  A note on times: For backwards compatibility with owl, we store the
  ``timestr'' (ctime) under the ``time'' key in the has, and the
  numeric time as ``unix_time''. Setting the numeric time
  automatically recomputes the timestr.
 */
void owl_message_set_time(owl_message *m, time_t tm)
{
  char * timestr;
  timestr = owl_sprintf("%d", tm);
  owl_message_set_attribute(m, "unix_time", timestr);
  owl_free(timestr);
  timestr = owl_strdup(ctime(&tm));
  /* Chop the newline */
  timestr[strlen(timestr)-1] = 0;
  owl_message_set_attribute(m, "time", timestr);
  owl_free(timestr);
}

time_t owl_message_get_time(owl_message *m)
{
  return (time_t)owl_message_get_attribute_int(m, "unix_time");
}

char *owl_message_get_timestr(owl_message *m)
{
  char *tm = owl_message_get_attribute_value(m, "time");
  if(tm == NULL) return "";
  return tm;
}

/* caller must free the return */
char *owl_message_get_shorttimestr(owl_message *m)
{
  struct tm *tmstruct;
  time_t time = owl_message_get_time(m);
  char *out;

  tmstruct=localtime(&time);
  out=owl_sprintf("%2.2i:%2.2i", tmstruct->tm_hour, tmstruct->tm_min);
  if (out) return(out);
  return("??:??");
}

void owl_message_set_type_admin(owl_message *m)
{
  owl_message_set_type(m, "admin");
}

void owl_message_set_type_loopback(owl_message *m)
{
  owl_message_set_type(m, "loopback");
}

void owl_message_set_type_zephyr(owl_message *m)
{
  owl_message_set_type(m, "zephyr");
}

void owl_message_set_type_aim(owl_message *m)
{
  owl_message_set_type(m, "AIM");
}

void owl_message_set_type(owl_message *m, char* type)
{
  char *blessas;
  HV *stash;
  owl_message_set_attribute(m, "type", type);
  type = owl_strdup(type);
  type[0] = toupper(type[0]);
  blessas = owl_sprintf("BarnOwl::Message::%s", type);
  stash = gv_stashpv(blessas, 0);
  if(!stash) {
    owl_function_error("No such class: %s for message type %s", blessas, owl_message_get_type(m));
    stash = gv_stashpv("BarnOwl::Message", 1);
  }
  sv_bless(m, stash);
  owl_free(type);
  owl_free(blessas);
}

int owl_message_is_type(owl_message *m, char *type) {
  char * t = owl_message_get_attribute_value(m, "type");
  if(!t) return 0;
  return !strcasecmp(t, type);
}
						
int owl_message_is_type_admin(owl_message *m)
{
  return owl_message_is_type(m, "admin");
}

int owl_message_is_type_zephyr(owl_message *m)
{
  return owl_message_is_type(m, "zephyr");
}

int owl_message_is_type_aim(owl_message *m)
{
  return owl_message_is_type(m, "aim");
}

/* XXX TODO: deprecate this */
int owl_message_is_type_jabber(owl_message *m)
{
  return owl_message_is_type(m, "jabber");
}

int owl_message_is_type_loopback(owl_message *m)
{
  return owl_message_is_type(m, "loopback");
}

int owl_message_is_pseudo(owl_message *m)
{
  if (owl_message_get_attribute_value(m, "pseudo")) return(1);
  return(0);
}

void owl_message_set_direction_in(owl_message *m)
{
  owl_message_set_direction(m, OWL_MESSAGE_DIRECTION_IN);
}

void owl_message_set_direction_out(owl_message *m)
{
  owl_message_set_direction(m, OWL_MESSAGE_DIRECTION_OUT);
}

void owl_message_set_direction_none(owl_message *m)
{
  owl_message_set_direction(m, OWL_MESSAGE_DIRECTION_NONE);
}

void owl_message_set_direction(owl_message *m, char *direction)
{
  owl_message_set_attribute(m, "direction", direction);
}

int owl_message_is_direction_in(owl_message *m)
{
  return !strcmp(owl_message_get_direction(m), OWL_MESSAGE_DIRECTION_IN);
}

int owl_message_is_direction_out(owl_message *m)
{
  return !strcmp(owl_message_get_direction(m), OWL_MESSAGE_DIRECTION_OUT);
}

int owl_message_is_direction_none(owl_message *m)
{
  return !strcmp(owl_message_get_direction(m), OWL_MESSAGE_DIRECTION_NONE);
}

void owl_message_mark_delete(owl_message *m)
{
  if (m == NULL) return;
  OWL_PERL_CALL_METHOD(m, "delete", /* no args */, "Error in delete: %s", 0, (void)POPs);
}

void owl_message_unmark_delete(owl_message *m)
{
  if (m == NULL) return;
  OWL_PERL_CALL_METHOD(m, "undelete", /* no args */, "Error in delete: %s", 0, (void)POPs);
}

char *owl_message_get_zwriteline(owl_message *m)
{
  return owl_message_get_attribute_value_nonull(m, "zwriteline");
}

void owl_message_set_zwriteline(owl_message *m, char *line)
{
  owl_message_set_attribute(m, "zwriteline", line);
}

int owl_message_is_delete(owl_message *m)
{
  if (m == NULL) return(0);
  char *str = owl_perlconfig_message_call_method(m, "is_deleted", 0, NULL);
  if(!str) return 0;
  int deleted = atoi(str);
  owl_free(str);
  return deleted;
}

void owl_message_set_hostname(owl_message *m, char *hostname)
{
  owl_message_set_attribute(m, "hostname", hostname);
}

char *owl_message_get_hostname(owl_message *m)
{
  return owl_message_get_attribute_value_nonull(m, "hostname");
}

int owl_message_is_personal(owl_message *m)
{
  owl_filter * f = owl_global_get_filter(&g, "personal");
  if(!f) {
      owl_function_error("personal filter is not defined");
      return (0);
  }
  return owl_filter_message_match(f, m);
}

int owl_message_is_question(owl_message *m)
{
  if(!owl_message_is_type_admin(m)) return 0;
  if(owl_message_get_attribute_value(m, "question") != NULL) return 1;
  return 0;
}

int owl_message_is_mail(owl_message *m)
{
  if (owl_message_is_type_zephyr(m)) {
    if (!strcasecmp(owl_message_get_class(m), "mail") && owl_message_is_private(m)) {
      return(1);
    } else {
      return(0);
    }
  }
  return(0);
}

/* caller must free return value. */
char *owl_message_get_cc(owl_message *m)
{
  char *cur, *out, *end;

  cur = owl_message_get_body(m);
  while (*cur && *cur==' ') cur++;
  if (strncasecmp(cur, "cc:", 3)) return(NULL);
  cur+=3;
  while (*cur && *cur==' ') cur++;
  out = owl_strdup(cur);
  end = strchr(out, '\n');
  if (end) end[0] = '\0';
  return(out);
}

/* caller must free return value */
char *owl_message_get_cc_without_recipient(owl_message *m)
{
  char *cc, *out, *end, *user, *shortuser, *recip;

  cc = owl_message_get_cc(m);
  if (cc == NULL)
    return NULL;

  recip = short_zuser(owl_message_get_recipient(m));
  out = owl_malloc(strlen(cc));
  end = out;

  user = strtok(cc, " ");
  while (user != NULL) {
    shortuser = short_zuser(user);
    if (strcasecmp(shortuser, recip) != 0) {
      strcpy(end, user);
      end[strlen(user)] = ' ';
      end += strlen(user) + 1;
    }
    free(shortuser);
    user = strtok(NULL, " ");
  }
  end[0] = '\0';

  owl_free(recip);
  owl_free(cc);

  if (strlen(out) == 0) {
    owl_free(out);
    out = NULL;
  }

  return(out);
}

char *owl_message_get_type(owl_message *m) {
  char * type = owl_message_get_attribute_value(m, "type");
  if(!type) {
    return "generic";
  }
  return type;
}

char *owl_message_get_direction(owl_message *m) {
  char *dir =  owl_message_get_attribute_value(m, "direction");
  if(dir == NULL) return "unknown";
  return dir;
}

char *owl_message_get_login(owl_message *m) {
  if (owl_message_is_login(m)) {
    return "login";
  } else if (owl_message_is_logout(m)) {
    return "logout";
  } else {
    return "none";
  }
}


char *owl_message_get_header(owl_message *m) {
  return owl_message_get_attribute_value(m, "adminheader");
}

/* return 1 if the message contains "string", 0 otherwise.  This is
 * case insensitive because the functions it uses are
 */
int owl_message_search(owl_message *m, char *string)
{

  return (owl_fmtext_search(owl_message_get_fmtext(m), string));
}


/* if loginout == -1 it's a logout message
 *                 0 it's not a login/logout message
 *                 1 it's a login message
 */
void owl_message_create_aim(owl_message *m, char *sender, char *recipient, char *text, char *direction, int loginout)
{
  owl_message_init(m);
  owl_message_set_body(m, text);
  owl_message_set_sender(m, sender);
  owl_message_set_recipient(m, recipient);
  owl_message_set_type_aim(m);

  owl_message_set_direction(m, direction);

  /* for now all messages that aren't loginout are private */
  if (!loginout) {
    owl_message_set_isprivate(m);
  }

  if (loginout==-1) {
    owl_message_set_islogout(m);
  } else if (loginout==1) {
    owl_message_set_islogin(m);
  }

  owl_message_lock(m);
}

void owl_message_create_admin(owl_message *m, char *header, char *text)
{
  owl_message_init(m);
  owl_message_set_type_admin(m);
  owl_message_set_body(m, text);
  owl_message_set_attribute(m, "adminheader", header); /* just a hack for now */

  owl_message_lock(m);
}

owl_message *owl_message_create_loopback(char *text, int outgoing)
{
  owl_message *m = owl_message_new();
  owl_message_init(m);
  owl_message_set_type_loopback(m);
  owl_message_set_body(m, text);
  owl_message_set_sender(m, "loopsender");
  owl_message_set_recipient(m, "looprecip");
  owl_message_set_isprivate(m);
  if (outgoing) {
    owl_message_set_direction_out(m);
  } else {
    owl_message_set_direction_in(m);
  }

  owl_message_lock(m);
  return m;
}

#ifdef HAVE_LIBZEPHYR
void owl_message_create_from_znotice(owl_message *m, ZNotice_t *n)
{
  struct hostent *hent;
  char *ptr, *tmp, *tmp2;
  int i,j,len;

  owl_message_init(m);
  
  owl_message_set_type_zephyr(m);
  owl_message_set_direction_in(m);

  owl_message_set_time(m, n->z_time.tv_sec);

  /* set other info */
  owl_message_set_sender(m, n->z_sender);
  owl_message_set_class(m, n->z_class);
  owl_message_set_instance(m, n->z_class_inst);
  owl_message_set_recipient(m, n->z_recipient);
  if (n->z_opcode) {
    owl_message_set_opcode(m, n->z_opcode);
  } else {
    owl_message_set_opcode(m, "");
  }
  owl_message_set_zsig(m, owl_zephyr_get_zsig(n, &len));

  if ((ptr=strchr(n->z_recipient, '@'))!=NULL) {
    owl_message_set_realm(m, ptr+1);
  } else {
    owl_message_set_realm(m, owl_zephyr_get_realm());
  }

  /* Set the "isloginout" attribute if it's a login message */
  if (!strcasecmp(n->z_class, "login") || !strcasecmp(n->z_class, OWL_WEBZEPHYR_CLASS)) {
    if (!strcasecmp(n->z_opcode, "user_login") || !strcasecmp(n->z_opcode, "user_logout")) {
      tmp=owl_zephyr_get_field(n, 1);
      owl_message_set_attribute(m, "loginhost", tmp);
      owl_free(tmp);

      tmp=owl_zephyr_get_field(n, 3);
      owl_message_set_attribute(m, "logintty", tmp);
      owl_free(tmp);
    }

    if (!strcasecmp(n->z_opcode, "user_login")) {
      owl_message_set_islogin(m);
    } else if (!strcasecmp(n->z_opcode, "user_logout")) {
      owl_message_set_islogout(m);
    }
  }

  
  /* set the "isprivate" attribute if it's a private zephyr.
   ``private'' means recipient is non-empty and doesn't start wit 
   `@' */
  if (*n->z_recipient && *n->z_recipient != '@') {
    owl_message_set_isprivate(m);
  }

  /* set the "isauto" attribute if it's an autoreply */
  if (!strcasecmp(n->z_message, "Automated reply:") ||
      !strcasecmp(n->z_opcode, "auto")) {
    owl_message_set_attribute(m, "isauto", "");
  }

  /* save the hostname */
  owl_function_debugmsg("About to do gethostbyaddr");
  hent=gethostbyaddr((char *) &(n->z_uid.zuid_addr), sizeof(n->z_uid.zuid_addr), AF_INET);
  if (hent && hent->h_name) {
    owl_message_set_hostname(m, hent->h_name);
  } else {
    owl_message_set_hostname(m, inet_ntoa(n->z_sender_addr));
  }

  /* set the body */
  tmp=owl_zephyr_get_message(n, m);
  if (owl_global_is_newlinestrip(&g)) {
    tmp2=owl_util_stripnewlines(tmp);
    owl_message_set_body(m, tmp2);
    owl_free(tmp2);
  } else {
    owl_message_set_body(m, tmp);
  }
  owl_free(tmp);

  /* Save the fields */
  AV *av_zfields;

  av_zfields = newAV();
  j=owl_zephyr_get_num_fields(n);
  for (i=0; i<j; i++) {
    ptr=owl_zephyr_get_field(n, i+1);
    av_push(av_zfields, newSVpvn(ptr, strlen(ptr)));
    owl_free(ptr);
  }
  (void)hv_store((HV*)SvRV(m), "fields", strlen("fields"), newRV_noinc((SV*)av_zfields), 0);

  /* Auth */
  owl_message_set_attribute(m, "auth", owl_zephyr_get_authstr(n));

#ifdef OWL_ENABLE_ZCRYPT
  /* if zcrypt is enabled try to decrypt the message */
  if (owl_global_is_zcrypt(&g) && !strcasecmp(n->z_opcode, "crypt")) {
    char *out;
    int ret;

    out=owl_malloc(strlen(owl_message_get_body(m))*16+20);
    ret=owl_zcrypt_decrypt(out, owl_message_get_body(m), owl_message_get_class(m), owl_message_get_instance(m));
    if (ret==0) {
      owl_message_set_body(m, out);
    } else {
      owl_free(out);
    }
  }
#endif

  owl_message_lock(m);
}

int owl_message_get_num_fields(owl_message *m)
{
  SV * ref_fields;
  AV * fields;
  ref_fields = owl_message_get_attribute_internal(m, "fields");
  if(!ref_fields || !SvROK(ref_fields)) return 0;
  fields = (AV*)(SvRV(ref_fields));
  return av_len(fields) + 1;
}

char *owl_message_get_field(owl_message *m, int n)
{
  n--;
  SV * ref_fields;
  AV * fields;
  SV ** f;
  ref_fields = owl_message_get_attribute_internal(m, "fields");
  if(!ref_fields || !SvROK(ref_fields)) return "";
  fields = (AV*)SvRV(ref_fields);
  f = av_fetch(fields, n, 0);
  if(!f) return "";
  return owl_strdup(SvPV_nolen(*f));
}
#else
void owl_message_create_from_znotice(owl_message *m, void *n)
{
}
#endif

/* If 'direction' is '0' it is a login message, '1' is a logout message. */
void owl_message_create_pseudo_zlogin(owl_message *m, int direction, char *user, char *host, char *time, char *tty)
{
  char *longuser, *ptr;

  longuser=long_zuser(user);
  
  owl_message_init(m);
  
  owl_message_set_type_zephyr(m);
  owl_message_set_direction_in(m);

  owl_message_set_attribute(m, "pseudo", "");
  owl_message_set_attribute(m, "loginhost", host ? host : "");
  owl_message_set_attribute(m, "logintty", tty ? tty : "");

  owl_message_set_sender(m, longuser);
  owl_message_set_class(m, "LOGIN");
  owl_message_set_instance(m, longuser);
  owl_message_set_recipient(m, "");
  if (direction==0) {
    owl_message_set_opcode(m, "USER_LOGIN");
    owl_message_set_islogin(m);
  } else if (direction==1) {
    owl_message_set_opcode(m, "USER_LOGOUT");
    owl_message_set_islogout(m);
  }

  if ((ptr=strchr(longuser, '@'))!=NULL) {
    owl_message_set_realm(m, ptr+1);
  } else {
    owl_message_set_realm(m, owl_zephyr_get_realm());
  }

  owl_message_set_body(m, "<uninitialized>");

  /* save the hostname */
  owl_function_debugmsg("create_pseudo_login: host is %s", host ? host : "");
  owl_message_set_hostname(m, host ? host : "");
  owl_free(longuser);

  owl_message_lock(m);
}

void owl_message_create_from_zwriteline(owl_message *m, char *line, char *body, char *zsig)
{
  owl_zwrite z;
  int ret;
  char hostbuff[5000];
  
  owl_message_init(m);

  /* create a zwrite for the purpose of filling in other message fields */
  owl_zwrite_create_from_line(&z, line);

  /* set things */
  owl_message_set_direction_out(m);
  owl_message_set_type_zephyr(m);
  owl_message_set_sender(m, owl_zephyr_get_sender());
  owl_message_set_class(m, owl_zwrite_get_class(&z));
  owl_message_set_instance(m, owl_zwrite_get_instance(&z));
  if (owl_zwrite_get_numrecips(&z)>0) {
    char *longzuser = long_zuser(owl_zwrite_get_recip_n(&z, 0));
    owl_message_set_recipient(m,
			      longzuser); /* only gets the first user, must fix */
    owl_free(longzuser);
  }
  owl_message_set_opcode(m, owl_zwrite_get_opcode(&z));
  owl_message_set_realm(m, owl_zwrite_get_realm(&z)); /* also a hack, but not here */
  owl_message_set_zwriteline(m, line);
  owl_message_set_body(m, body);
  owl_message_set_zsig(m, zsig);
  
  /* save the hostname */
  ret=gethostname(hostbuff, MAXHOSTNAMELEN);
  hostbuff[MAXHOSTNAMELEN]='\0';
  if (ret) {
    owl_message_set_hostname(m, "localhost");
  } else {
    owl_message_set_hostname(m, hostbuff);
  }

  /* set the "isprivate" attribute if it's a private zephyr. */
  if (owl_zwrite_is_personal(&z)) {
    owl_message_set_isprivate(m);
  }

  owl_zwrite_free(&z);
  owl_message_lock(m);
}

void owl_message_free(owl_message *m)
{
  owl_message_invalidate_format(m);
  SvREFCNT_dec((SV*)m);
}


/**********************************************************************
 * Functions related to message styling and formatting
 **********************************************************************/

#define NO_MESSAGE (-1);
void owl_message_init_fmtext_cache ()
{
    int i;
    for(i = 0; i < OWL_FMTEXT_CACHE_SIZE; i++) {
        owl_fmtext_init_null(&(fmtext_cache[i].fmtext));
        fmtext_cache[i].message_id = NO_MESSAGE;
    }
}

owl_fmtext_cache * owl_message_next_fmtext() /*noproto*/
{
    owl_fmtext_cache * f = fmtext_cache_next;
    fmtext_cache_next++;
    if(fmtext_cache_next - fmtext_cache == OWL_FMTEXT_CACHE_SIZE)
        fmtext_cache_next = fmtext_cache;
    owl_fmtext_clear(&(f->fmtext));
    f->message_id = NO_MESSAGE;
    f->seq = owl_global_get_fmtext_seq(&g);
    return f;
}

void owl_message_invalidate_format(owl_message *m)
{
  owl_fmtext_cache * fmtext = owl_message_get_fmtext_cache(m);
  if(fmtext) {
    fmtext->message_id = NO_MESSAGE;
    owl_fmtext_clear(&(fmtext->fmtext));
    owl_message_set_fmtext_cache(m, NULL);
  }
}

owl_fmtext_cache *owl_message_get_fmtext_cache(owl_message *m)
{
  SV *fmtext = owl_message_get_attribute_internal(m, "__fmtext");
  if(fmtext && SvROK(fmtext)) {
    int fm_which = SvIV(SvRV(fmtext));
    if(fm_which >= 0 && fm_which < OWL_FMTEXT_CACHE_SIZE) {
      return fmtext_cache + fm_which;
    }
  }
  return NULL;
}

void owl_message_set_fmtext_cache(owl_message *m, owl_fmtext_cache *fm)
{
  SV *fmtext = newSV(0);
  sv_setref_iv(fmtext, Nullch, (fm - fmtext_cache));
  HV *hash = (HV*)SvRV((SV*)m);
  (void)hv_store(hash, "__fmtext", strlen("__fmtext"), fmtext, 0);
}

owl_fmtext *owl_message_get_fmtext(owl_message *m)
{
  owl_message_format(m);
  return(&(owl_message_get_fmtext_cache(m)->fmtext));
}

void owl_message_format(owl_message *m)
{
  owl_style *s;
  owl_view *v;
  owl_fmtext_cache *fm;

  fm = owl_message_get_fmtext_cache(m);

  if (!fm
      || fm->message_id != owl_message_get_id(m)
      || fm->seq != owl_global_get_fmtext_seq(&g)) {
    fm = owl_message_next_fmtext();
    owl_message_set_fmtext_cache(m, fm);
    fm->message_id = owl_message_get_id(m);
    /* for now we assume there's just the one view and use that style */
    v=owl_global_get_current_view(&g);
    s=owl_global_get_current_style(&g);

    owl_style_get_formattext(s, &(fm->fmtext), m);
  }
}

char *owl_message_get_text(owl_message *m)
{
  return(owl_fmtext_get_text(owl_message_get_fmtext(m)));
}

int owl_message_get_numlines(owl_message *m)
{
  if (m == NULL) return(0);
  return(owl_fmtext_num_lines(owl_message_get_fmtext(m)));
}

void owl_message_curs_waddstr(owl_message *m, WINDOW *win, int aline, int bline, int acol, int bcol, int fgcolor, int bgcolor)
{
  owl_fmtext a, b;

  owl_fmtext *fm = owl_message_get_fmtext(m);

  owl_fmtext_init_null(&a);
  owl_fmtext_init_null(&b);
  
  owl_fmtext_truncate_lines(fm, aline, bline-aline, &a);
  owl_fmtext_truncate_cols(&a, acol, bcol, &b);
  owl_fmtext_colorize(&b, fgcolor);
  owl_fmtext_colorizebg(&b, bgcolor);

  owl_fmtext_curs_waddstr(&b, win);

  owl_fmtext_free(&a);
  owl_fmtext_free(&b);
}

void owl_message_attributes_tofmtext(owl_message *m, owl_fmtext *fm) {
  owl_fmtext_init_null(fm);

  char *text = owl_perlconfig_message_call_method(m, "__format_attributes", 0, NULL);

  owl_fmtext_append_normal(fm, text);

  owl_free(text);
}

