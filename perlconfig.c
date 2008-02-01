#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#define OWL_PERL
#include "owl.h"

static const char fileIdent[] = "$Id$";

extern XS(boot_BarnOwl);
extern XS(boot_DynaLoader);
/* extern XS(boot_DBI); */

static void owl_perl_xs_init(pTHX)
{
  char *file = __FILE__;
  dXSUB_SYS;
  {
    newXS("BarnOwl::bootstrap", boot_BarnOwl, file);
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
  }
}

SV *owl_perlconfig_message2hashref(owl_message *m)
{
  return SvREFCNT_inc(m);
}

SV *owl_perlconfig_curmessage2hashref(void) /*noproto*/
{
  owl_message *m = owl_global_get_current_message(&g);
  if(m == NULL) {
    return &PL_sv_undef;
  }
  return owl_perlconfig_message2hashref(m);
}

owl_message * owl_perlconfig_hashref2message(SV *msg)
{
  return (owl_message*)SvREFCNT_inc(msg);
}

/* Calls in a scalar context, passing it a hash reference.
   If return value is non-null, caller must free. */
char *owl_perlconfig_call_with_message(char *subname, owl_message *m)
{
  dSP ;
  int count;
  SV *msgref, *srv;
  char *out;
  
  ENTER ;
  SAVETMPS;
  
  PUSHMARK(SP) ;
  msgref = owl_perlconfig_message2hashref(m);
  XPUSHs(sv_2mortal(msgref));
  PUTBACK ;
  
  count = call_pv(subname, G_SCALAR|G_EVAL);
  
  SPAGAIN ;

  if (SvTRUE(ERRSV)) {
    owl_function_error("Perl Error: '%s'", SvPV_nolen(ERRSV));
    /* and clear the error */
    sv_setsv (ERRSV, &PL_sv_undef);
  }

  if (count != 1) {
    fprintf(stderr, "bad perl!  no biscuit!  returned wrong count!\n");
    abort();
  }

  srv = POPs;

  if (srv) {
    out = owl_strdup(SvPV_nolen(srv));
  } else {
    out = NULL;
  }
  
  PUTBACK ;
  FREETMPS ;
  LEAVE ;

  return out;
}


/* Calls a method on a perl object representing a message.
   If the return value is non-null, the caller must free it.
 */
char * owl_perlconfig_message_call_method(owl_message *m, char *method, int argc, char ** argv)
{
  dSP;
  unsigned int count, i;
  SV *msgref, *srv;
  char *out;

  msgref = owl_perlconfig_message2hashref(m);

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  XPUSHs(sv_2mortal(msgref));
  for(i=0;i<argc;i++) {
    XPUSHs(sv_2mortal(newSVpv(argv[i], 0)));
  }
  PUTBACK;

  count = call_method(method, G_SCALAR|G_EVAL);

  SPAGAIN;

  if(count != 1) {
    fprintf(stderr, "perl returned wrong count %d\n", count);
    abort();
  }

  if (SvTRUE(ERRSV)) {
    owl_function_error("Error: '%s'", SvPV_nolen(ERRSV));
    /* and clear the error */
    sv_setsv (ERRSV, &PL_sv_undef);
  }

  srv = POPs;

  if (srv) {
    out = owl_strdup(SvPV_nolen(srv));
  } else {
    out = NULL;
  }

  PUTBACK;
  FREETMPS;
  LEAVE;

  return out;
}


char *owl_perlconfig_initperl(char * file, int *Pargc, char ***Pargv, char *** Penv)
{
  int ret;
  PerlInterpreter *p;
  char *err;
  char *args[] = {"", "-e", "0;", NULL};
  AV *inc;
  char *path;

  /* create and initialize interpreter */
  PERL_SYS_INIT3(Pargc, Pargv, Penv);
  p=perl_alloc();
  owl_global_set_perlinterp(&g, (void*)p);
  perl_construct(p);

  owl_global_set_no_have_config(&g);

  ret=perl_parse(p, owl_perl_xs_init, sizeof(args)/sizeof(*args) - 1, args, NULL);
  if (ret || SvTRUE(ERRSV)) {
    err=owl_strdup(SvPV_nolen(ERRSV));
    sv_setsv(ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  ret=perl_run(p);
  if (ret || SvTRUE(ERRSV)) {
    err=owl_strdup(SvPV_nolen(ERRSV));
    sv_setsv(ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  owl_global_set_have_config(&g);

  /* create legacy variables */
  get_sv("BarnOwl::id", TRUE);
  get_sv("BarnOwl::class", TRUE);
  get_sv("BarnOwl::instance", TRUE);
  get_sv("BarnOwl::recipient", TRUE);
  get_sv("BarnOwl::sender", TRUE);
  get_sv("BarnOwl::realm", TRUE);
  get_sv("BarnOwl::opcode", TRUE);
  get_sv("BarnOwl::zsig", TRUE);
  get_sv("BarnOwl::msg", TRUE);
  get_sv("BarnOwl::time", TRUE);
  get_sv("BarnOwl::host", TRUE);
  get_av("BarnOwl::fields", TRUE);

  if(file) {
    SV * cfg = get_sv("BarnOwl::configfile", TRUE);
    sv_setpv(cfg, file);
  }

  /* Add the system lib path to @INC */
  inc = get_av("INC", 0);
  path = owl_sprintf("%s/lib", owl_get_datadir());
  av_unshift(inc, 1);
  av_store(inc, 0, newSVpv(path, 0));
  owl_free(path);

  eval_pv("use BarnOwl;", FALSE);

  if (SvTRUE(ERRSV)) {
    err=owl_strdup(SvPV_nolen(ERRSV));
    sv_setsv (ERRSV, &PL_sv_undef);     /* and clear the error */
    return(err);
  }

  /* check if we have the formatting function */
  if (owl_perlconfig_is_function("BarnOwl::format_msg")) {
    owl_global_set_config_format(&g, 1);
  }

  return(NULL);
}

void owl_perlconfig_shutdown() {
  PerlInterpreter *p;
  p = owl_global_get_perlinterp(&g);
  perl_destruct(p);
  perl_free(p);
}

/* returns whether or not a function exists */
int owl_perlconfig_is_function(char *fn) {
  if (get_cv(fn, FALSE)) return(1);
  else return(0);
}

/* caller is responsible for freeing returned string */
char *owl_perlconfig_execute(char *line)
{
  STRLEN len;
  SV *response;
  char *out, *preout;

  if (!owl_global_have_config(&g)) return NULL;

  ENTER;
  SAVETMPS;
  /* execute the subroutine */
  response = eval_pv(line, FALSE);

  if (SvTRUE(ERRSV)) {
    owl_function_error("Perl Error: '%s'", SvPV_nolen(ERRSV));
    sv_setsv (ERRSV, &PL_sv_undef);     /* and clear the error */
  }

  preout=SvPV(response, len);
  if (len == 0 || preout[len - 1] != '\n')
    out = owl_sprintf("%s\n", preout);
  else
    out = owl_strdup(preout);
  FREETMPS;
  LEAVE;

  return(out);
}

void owl_perlconfig_getmsg(owl_message *m, char *subname)
{
  char *ptr = NULL;
  if (owl_perlconfig_is_function("BarnOwl::Hooks::_receive_msg")) {
    ptr = owl_perlconfig_call_with_message(subname?subname
                                           :"BarnOwl::_receive_msg_legacy_wrap", m);
  }
  if (ptr) owl_free(ptr);
}

/* Called on all new messages; receivemsg is only called on incoming ones */
void owl_perlconfig_newmsg(owl_message *m, char *subname)
{
  char *ptr = NULL;
  if (owl_perlconfig_is_function("BarnOwl::Hooks::_new_msg")) {
    ptr = owl_perlconfig_call_with_message(subname?subname
                                           :"BarnOwl::Hooks::_new_msg", m);
  }
  if (ptr) owl_free(ptr);
}

char *owl_perlconfig_perlcmd(owl_cmd *cmd, int argc, char **argv)
{
  int i, count;
  char * ret = NULL;
  SV *rv;
  dSP;

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  for(i=0;i<argc;i++) {
    SV *tmp = newSVpv(argv[i], 0);
    SvUTF8_on(tmp);
    XPUSHs(sv_2mortal(tmp));
  }
  PUTBACK;

  count = call_sv(cmd->cmd_perl, G_SCALAR|G_EVAL);

  SPAGAIN;

  if(SvTRUE(ERRSV)) {
    owl_function_error("%s", SvPV_nolen(ERRSV));
    (void)POPs;
  } else {
    if(count != 1)
      croak("Perl command %s returned more than one value!", cmd->name);
    rv = POPs;
    if(SvTRUE(rv)) {
      ret = owl_strdup(SvPV_nolen(rv));
    }
  }

  FREETMPS;
  LEAVE;

  return ret;
}

void owl_perlconfig_cmd_free(owl_cmd *cmd)
{
  SvREFCNT_dec(cmd->cmd_perl);
}

void owl_perlconfig_dispatch_free(owl_dispatch *d)
{
  SvREFCNT_dec((SV*)d->data);
  owl_free(d);
}

void owl_perlconfig_edit_callback(owl_editwin *e)
{
  SV *cb = (SV*)(e->cbdata);
  SV *text;
  dSP;

  if(cb == NULL) {
    owl_function_error("Perl callback is NULL!");
  }
  text = newSVpv(owl_editwin_get_text(e), 0);
  SvUTF8_on(text);

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  XPUSHs(sv_2mortal(text));
  PUTBACK;
  
  call_sv(cb, G_DISCARD|G_EVAL);

  if(SvTRUE(ERRSV)) {
    owl_function_error("%s", SvPV_nolen(ERRSV));
  }

  FREETMPS;
  LEAVE;

  SvREFCNT_dec(cb);
  e->cbdata = NULL;
}

void owl_perlconfig_mainloop()
{
  dSP;
  if (!owl_perlconfig_is_function("BarnOwl::Hooks::_mainloop_hook"))
    return;
  PUSHMARK(SP) ;
  call_pv("BarnOwl::Hooks::_mainloop_hook", G_DISCARD|G_EVAL);
  if(SvTRUE(ERRSV)) {
    owl_function_error("%s", SvPV_nolen(ERRSV));
  }
  return;
}

void owl_perlconfig_dispatch(owl_dispatch *d)
{
  SV *cb = d->data;
  dSP;
  if(cb == NULL) {
    owl_function_error("Perl callback is NULL!");
    return;
  }

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  PUTBACK;
  
  call_sv(cb, G_DISCARD|G_KEEPERR|G_EVAL);

  if(SvTRUE(ERRSV)) {
    owl_function_error("%s", SvPV_nolen(ERRSV));
  }

  FREETMPS;
  LEAVE;
}

void owl_perlconfig_perl_timer(owl_timer *t, void *data)
{
  SV *obj = data;

  if(!SvROK(obj)) {
    return;
  }

  dSP;
  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  XPUSHs(obj);
  PUTBACK;

  call_method("do_callback", G_DISCARD|G_KEEPERR|G_EVAL);

  SPAGAIN;

  if (SvTRUE(ERRSV)) {
    owl_function_error("Error in calback: '%s'", SvPV_nolen(ERRSV));
    sv_setsv (ERRSV, &PL_sv_undef);
  }

  PUTBACK;
  FREETMPS;
  LEAVE;
}

void owl_perlconfig_perl_timer_destroy(owl_timer *t)
{
  if(SvOK((SV*)t->data)) {
    SvREFCNT_dec((SV*)t->data);
  }
}

SV * owl_perl_new(char *class)
{
  return owl_perl_new_argv(class, NULL, 0);
}

SV * owl_perl_new_argv(char *class, char **argv, int argc)
{
  SV *obj;
  int i;
  OWL_PERL_CALL_METHOD(sv_2mortal(newSVpv(class, 0)), "new",
                       for(i=0;i<argc;i++) {
                         XPUSHs(sv_2mortal(newSVpv(argv[i], 0)));
                       }
                       ,
                       "Error in perl: %s\n",
                       1,
                       obj = POPs;
                       SvREFCNT_inc(obj);
                       );
  return obj;
}

void owl_perl_savetmps() {
  ENTER;
  SAVETMPS;
}

void owl_perl_freetmps() {
  FREETMPS;
  LEAVE;
}

void owl_perlconfig_do_dispatch(owl_dispatch *d)
{
  SV *cb = (SV*)d->data;
  unsigned int n_a;
  dSP;
  if(cb == NULL) {
    owl_function_error("Perl callback is NULL!");
  }

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  PUTBACK;
  
  call_sv(cb, G_DISCARD|G_KEEPERR|G_EVAL);

  if(SvTRUE(ERRSV)) {
    owl_function_error("%s", SvPV(ERRSV, n_a));
  }

  FREETMPS;
  LEAVE;
}

void owl_perlconfig_invalidate_filter(owl_filter *f)
{
  dSP;
  ENTER;
  SAVETMPS;

  PUSHMARK(SP);
  XPUSHs(sv_2mortal(newSVpv(owl_filter_get_name(f), 0)));
  PUTBACK;

  call_pv("BarnOwl::Hooks::_invalidate_filter", G_DISCARD|G_KEEPERR|G_EVAL);

  FREETMPS;
  LEAVE;
}
