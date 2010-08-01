#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "owl.h"
#include "filterproc.h"

char *owl_function_command(const char *cmdbuff)
{
  owl_function_debugmsg("executing command: %s", cmdbuff);
  return owl_cmddict_execute(owl_global_get_cmddict(&g), 
			     owl_global_get_context(&g), cmdbuff);
}

char *owl_function_command_argv(const char *const *argv, int argc)
{
  return owl_cmddict_execute_argv(owl_global_get_cmddict(&g),
                                  owl_global_get_context(&g),
                                  argv, argc);
}

void owl_function_command_norv(const char *cmdbuff)
{
  char *rv;
  rv=owl_function_command(cmdbuff);
  if (rv) owl_free(rv);
}

void owl_function_command_alias(const char *alias_from, const char *alias_to)
{
  owl_cmddict_add_alias(owl_global_get_cmddict(&g), alias_from, alias_to);
}

const owl_cmd *owl_function_get_cmd(const char *name)
{
  return owl_cmddict_find(owl_global_get_cmddict(&g), name);
}

void owl_function_show_commands(void)
{
  owl_list l;
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_bold(&fm, "Commands:  ");
  owl_fmtext_append_normal(&fm, "(use 'show command <name>' for details)\n");
  owl_cmddict_get_names(owl_global_get_cmddict(&g), &l);
  owl_fmtext_append_list(&fm, &l, "\n", owl_function_cmd_describe);
  owl_fmtext_append_normal(&fm, "\n");
  owl_function_popless_fmtext(&fm);
  owl_cmddict_namelist_cleanup(&l);
  owl_fmtext_cleanup(&fm);
}

void owl_function_show_view(const char *viewname)
{
  const owl_view *v;
  owl_fmtext fm;

  /* we only have the one view right now */
  v=owl_global_get_current_view(&g);
  if (viewname && strcmp(viewname, owl_view_get_name(v))) {
    owl_function_error("No view named '%s'", viewname);
    return;
  }

  owl_fmtext_init_null(&fm);
  owl_view_to_fmtext(v, &fm);
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_show_styles(void) {
  owl_list l;
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_bold(&fm, "Styles:\n");
  owl_global_get_style_names(&g, &l);
  owl_fmtext_append_list(&fm, &l, "\n", owl_function_style_describe);
  owl_fmtext_append_normal(&fm, "\n");
  owl_function_popless_fmtext(&fm);
  owl_list_cleanup(&l, owl_free);
  owl_fmtext_cleanup(&fm);
}

char *owl_function_style_describe(const char *name) {
  const char *desc;
  char *s;
  const owl_style *style;
  style = owl_global_get_style_by_name(&g, name);
  if (style) {
    desc = owl_style_get_description(style);
  } else {
    desc = "???";
  }
  s = owl_sprintf("%-20s - %s%s", name, 
		  0==owl_style_validate(style)?"":"[INVALID] ",
		  desc);
  return s;
}

char *owl_function_cmd_describe(const char *name)
{
  const owl_cmd *cmd = owl_cmddict_find(owl_global_get_cmddict(&g), name);
  if (cmd) return owl_cmd_describe(cmd);
  else return(NULL);
}

void owl_function_show_command(const char *name)
{
  owl_function_help_for_command(name);
}

void owl_function_show_license(void)
{
  const char *text;

  text=""
    "barnowl version " OWL_VERSION_STRING "\n"
    "Copyright (c) 2006-2010 The BarnOwl Developers. All rights reserved.\n"
    "Copyright (c) 2004 James Kretchmar. All rights reserved.\n"
    "\n"
    "Redistribution and use in source and binary forms, with or without\n"
    "modification, are permitted provided that the following conditions are\n"
    "met:\n"
    "\n"
    "   * Redistributions of source code must retain the above copyright\n"
    "     notice, this list of conditions and the following disclaimer.\n"
    "\n"
    "   * Redistributions in binary form must reproduce the above copyright\n"
    "     notice, this list of conditions and the following disclaimer in\n"
    "     the documentation and/or other materials provided with the\n"
    "     distribution.\n"
    "\n"
    "   * Redistributions in any form must be accompanied by information on\n"
    "     how to obtain complete source code for the Owl software and any\n"
    "     accompanying software that uses the Owl software. The source code\n"
    "     must either be included in the distribution or be available for no\n"
    "     more than the cost of distribution plus a nominal fee, and must be\n"
    "     freely redistributable under reasonable conditions. For an\n"
    "     executable file, complete source code means the source code for\n"
    "     all modules it contains. It does not include source code for\n"
    "     modules or files that typically accompany the major components of\n"
    "     the operating system on which the executable file runs.\n"
    "\n"
    "THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
    "IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
    "WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR\n"
    "NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE\n"
    "LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n"
    "CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF\n"
    "SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR\n"
    "BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,\n"
    "WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE\n"
    "OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN\n"
    "IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";
  owl_function_popless_text(text);
}

void owl_function_show_quickstart(void)
{
    const char *message =
    "Move between messages with the arrow keys, and press 'r' to reply.\n"
    "For more info, press 'h' or visit http://barnowl.mit.edu/\n\n"
#ifdef HAVE_LIBZEPHYR
    "@b(Zephyr:)\n"
    "To send a message to a user, type ':zwrite @b(username)'. You can also\n"
    "press 'z' and then type the username. To subscribe to a class, type\n"
    "':sub @b(class)', and then type ':zwrite -c @b(class)' to send.\n\n"
#endif
    "@b(AIM:)\n"
    "Log in to AIM with ':aimlogin @b(screenname)'. Use ':aimwrite @b(screenname)',\n"
    "or 'a' and then the screen name, to send someone a message.\n\n"
    ;

    if (owl_perlconfig_is_function("BarnOwl::Hooks::_get_quickstart")) {
        char *perlquickstart = owl_perlconfig_execute("BarnOwl::Hooks::_get_quickstart()");
        if (perlquickstart) {
            char *result = owl_sprintf("%s%s", message, perlquickstart);
            owl_function_adminmsg("BarnOwl Quickstart", result);
            owl_free(result);
            owl_free(perlquickstart);
            return;
        }
    }
    owl_function_adminmsg("BarnOwl Quickstart", message);
}


/* Create an admin message, append it to the global list of messages
 * and redisplay if necessary.
 */
void owl_function_adminmsg(const char *header, const char *body)
{
  owl_message *m;

  m=owl_malloc(sizeof(owl_message));
  owl_message_create_admin(m, header, body);
  
  /* add it to the global list and current view */
  owl_messagelist_append_element(owl_global_get_msglist(&g), m);
  owl_view_consider_message(owl_global_get_current_view(&g), m);

  /* do followlast if necessary */
  if (owl_global_should_followlast(&g)) owl_function_lastmsg_noredisplay();

  /* redisplay etc. */
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

/* Create an outgoing zephyr message and return a pointer to it.  Does
 * not put it on the global queue, use owl_global_messagequeue_addmsg() for
 * that.
 */
owl_message *owl_function_make_outgoing_zephyr(const owl_zwrite *z)
{
  owl_message *m;

  /* create the message */
  m=owl_malloc(sizeof(owl_message));
  owl_message_create_from_zwrite(m, z, owl_zwrite_get_message(z));

  return(m);
}

/* Create an outgoing AIM message, returns a pointer to the created
 * message or NULL if we're not logged into AIM (and thus unable to
 * create the message).  Does not put it on the global queue.  Use
 * owl_global_messagequeue_addmsg() for that .
 */
owl_message *owl_function_make_outgoing_aim(const char *body, const char *to)
{
  owl_message *m;

  /* error if we're not logged into aim */
  if (!owl_global_is_aimloggedin(&g)) return(NULL);
  
  m=owl_malloc(sizeof(owl_message));
  owl_message_create_aim(m,
			 owl_global_get_aim_screenname(&g),
			 to,
			 body,
			 OWL_MESSAGE_DIRECTION_OUT,
			 0);
  return(m);
}

/* Create an outgoing loopback message and return a pointer to it.
 * Does not append it to the global queue, use
 * owl_global_messagequeue_addmsg() for that.
 */
owl_message *owl_function_make_outgoing_loopback(const char *body)
{
  owl_message *m;

  /* create the message */
  m=owl_malloc(sizeof(owl_message));
  owl_message_create_loopback(m, body);
  owl_message_set_direction_out(m);

  return(m);
}

void owl_function_start_edit_win(const char *line, void (*callback)(owl_editwin *), void *data, void (*cleanup)(void *))
{
  owl_editwin *e;
  char *s;

  /* create and setup the editwin */
  e = owl_global_set_typwin_active(&g, OWL_EDITWIN_STYLE_MULTILINE,
                                   owl_global_get_msg_history(&g));
  owl_editwin_set_dotsend(e);
  s = owl_sprintf("----> %s\n", line);
  owl_editwin_set_locktext(e, s);
  owl_free(s);

  owl_editwin_set_cbdata(e, data, cleanup);
  owl_editwin_set_callback(e, callback);
  owl_global_push_context(&g, OWL_CTX_EDITMULTI, e, "editmulti", owl_global_get_typwin_window(&g));
}

static void owl_function_write_setup(const char *noun)
{

  if (!owl_global_get_lockout_ctrld(&g))
    owl_function_makemsg("Type your %s below.  "
			 "End with ^D or a dot on a line by itself."
			 "  ^C will quit.", noun);
  else
    owl_function_makemsg("Type your %s below.  "
			 "End with a dot on a line by itself.  ^C will quit.",
			 noun);
}

void owl_function_zwrite_setup(owl_zwrite *z)
{
  /* send a ping if necessary */
  if (owl_global_is_txping(&g)) {
    owl_zwrite_send_ping(z);
  }


  owl_function_write_setup("zephyr");
  owl_function_start_edit_win(z->zwriteline,
                              &owl_callback_zwrite,
                              z, (void(*)(void*))owl_zwrite_delete);
}

void owl_function_aimwrite_setup(const char *line)
{
  owl_function_write_setup("message");
  owl_function_start_edit_win(line,
                              &owl_callback_aimwrite,
                              owl_strdup(line),
                              owl_free);

}

void owl_function_loopwrite_setup(void)
{
  owl_function_write_setup("message");
  owl_function_start_edit_win("loopwrite",
                              &owl_callback_loopwrite,
                              "loopwrite", NULL);
}

void owl_callback_zwrite(owl_editwin *e) {
  owl_zwrite *z = owl_editwin_get_cbdata(e);
  owl_function_zwrite(z, owl_editwin_get_text(e));
}

/* send, log and display an outgoing zephyr.  If 'msg' is NULL
 * the message is expected to be set from the zwrite line itself
 */
void owl_function_zwrite(owl_zwrite *z, const char *msg)
{
  owl_message *m;
  int ret;

  if(strcmp(z->cmd, "zcrypt") == 0) {
    owl_function_zcrypt(z, msg);
    return;
  }

  /* create the zwrite and send the message */
  owl_zwrite_populate_zsig(z);
  if (msg) {
    owl_zwrite_set_message(z, msg);
  }
  ret = owl_zwrite_send_message(z);
  if (ret != 0) {
    owl_function_makemsg("Error sending zephyr: %s", error_message(ret));
    return;
  }
  owl_function_makemsg("Waiting for ack...");

  /* If it's personal */
  if (owl_zwrite_is_personal(z)) {
    /* create the outgoing message */
    m=owl_function_make_outgoing_zephyr(z);

    if (m) {
      owl_global_messagequeue_addmsg(&g, m);
    } else {
      owl_function_error("Could not create outgoing zephyr message");
    }
  }
}

/* send, log and display an outgoing zcrypt zephyr.  If 'msg' is NULL
 * the message is expected to be set from the zwrite line itself
 */
void owl_function_zcrypt(owl_zwrite *z, const char *msg)
{
  char *cryptmsg;
  owl_message *m;
  const char *argv[7];
  char *zcrypt;
  int rv, status;
  char *old_msg;

  /* create the zwrite and send the message */
  owl_zwrite_populate_zsig(z);
  if (msg) {
    owl_zwrite_set_message(z, msg);
  }
  old_msg = owl_strdup(owl_zwrite_get_message(z));

  zcrypt = owl_sprintf("%s/zcrypt", owl_get_bindir());
  argv[0] = "zcrypt";
  argv[1] = "-E";
  argv[2] = "-c"; argv[3] = owl_zwrite_get_class(z);
  argv[4] = "-i"; argv[5] = owl_zwrite_get_instance(z);
  argv[6] = NULL;

  rv = call_filter(zcrypt, argv, owl_zwrite_get_message(z), &cryptmsg, &status);

  owl_free(zcrypt);

  if (rv || status) {
    if(cryptmsg) owl_free(cryptmsg);
    owl_free(old_msg);
    owl_function_error("Error in zcrypt, possibly no key found.  Message not sent.");
    owl_function_beep();
    return;
  }

  owl_zwrite_set_message_raw(z, cryptmsg);
  owl_zwrite_set_opcode(z, "crypt");

  owl_zwrite_send_message(z);
  owl_function_makemsg("Waiting for ack...");

  /* If it's personal */
  if (owl_zwrite_is_personal(z)) {
    /* Create the outgoing message. Restore the un-crypted message for display. */
    owl_zwrite_set_message_raw(z, old_msg);
    m=owl_function_make_outgoing_zephyr(z);
    if (m) {
      owl_global_messagequeue_addmsg(&g, m);
    } else {
      owl_function_error("Could not create outgoing zephyr message");
    }
  }

  /* free the zwrite */
  owl_free(cryptmsg);
}

void owl_callback_aimwrite(owl_editwin *e) {
  char *command = owl_editwin_get_cbdata(e);
  owl_function_aimwrite(command,
                        owl_editwin_get_text(e));
}

void owl_function_aimwrite(const char *line, const char *msg)
{
  int ret;
  const char *to;
  char *format_msg;
  owl_message *m;

  to = line + 9;

  /* make a formatted copy of the message */
  format_msg=owl_strdup(msg);
  owl_text_wordunwrap(format_msg);
  
  /* send the message */
  ret=owl_aim_send_im(to, format_msg);
  if (!ret) {
    owl_function_makemsg("AIM message sent.");
  } else {
    owl_function_error("Could not send AIM message.");
  }

  /* create the outgoing message */
  m=owl_function_make_outgoing_aim(msg, to);

  if (m) {
    owl_global_messagequeue_addmsg(&g, m);
  } else {
    owl_function_error("Could not create outgoing AIM message");
  }

  owl_free(format_msg);
}

void owl_function_send_aimawymsg(const char *to, const char *msg)
{
  int ret;
  char *format_msg;
  owl_message *m;

  /* make a formatted copy of the message */
  format_msg=owl_strdup(msg);
  owl_text_wordunwrap(format_msg);
  
  /* send the message */
  ret=owl_aim_send_awaymsg(to, format_msg);
  if (!ret) {
    /* owl_function_makemsg("AIM message sent."); */
  } else {
    owl_function_error("Could not send AIM message.");
  }

  /* create the message */
  m=owl_function_make_outgoing_aim(msg, to);
  if (m) {
    owl_global_messagequeue_addmsg(&g, m);
  } else {
    owl_function_error("Could not create AIM message");
  }
  owl_free(format_msg);
}

void owl_callback_loopwrite(owl_editwin *e) {
  owl_function_loopwrite(owl_editwin_get_text(e));
}

void owl_function_loopwrite(const char *msg)
{
  owl_message *min, *mout;

  /* create a message and put it on the message queue.  This simulates
   * an incoming message */
  min=owl_malloc(sizeof(owl_message));
  mout=owl_function_make_outgoing_loopback(msg);

  if (owl_global_is_displayoutgoing(&g)) {
    owl_global_messagequeue_addmsg(&g, mout);
  } else {
    owl_message_delete(mout);
  }

  owl_message_create_loopback(min, msg);
  owl_message_set_direction_in(min);
  owl_global_messagequeue_addmsg(&g, min);

  /* fake a makemsg */
  owl_function_makemsg("loopback message sent");
}

/* If filter is non-null, looks for the next message matching
 * that filter.  If skip_deleted, skips any deleted messages. 
 * If last_if_none, will stop at the last message in the view
 * if no matching messages are found.  */
void owl_function_nextmsg_full(const char *filter, int skip_deleted, int last_if_none)
{
  int curmsg, i, viewsize, found;
  const owl_view *v;
  const owl_filter *f = NULL;
  const owl_message *m;

  v=owl_global_get_current_view(&g);

  if (filter) {
    f=owl_global_get_filter(&g, filter);
    if (!f) {
      owl_function_error("No %s filter defined", filter);
      return;
    }
  }

  curmsg=owl_global_get_curmsg(&g);
  viewsize=owl_view_get_size(v);
  found=0;

  /* just check to make sure we're in bounds... */
  if (curmsg>viewsize-1) curmsg=viewsize-1;
  if (curmsg<0) curmsg=0;

  for (i=curmsg+1; i<viewsize; i++) {
    m=owl_view_get_element(v, i);
    if (skip_deleted && owl_message_is_delete(m)) continue;
    if (f && !owl_filter_message_match(f, m)) continue;
    found = 1;
    break;
  }

  if (i>owl_view_get_size(v)-1) i=owl_view_get_size(v)-1;
  if (i<0) i=0;

  if (!found) {
    owl_function_makemsg("already at last%s message%s%s%s",
			 skip_deleted?" non-deleted":"",
			 filter?" in ":"", filter?filter:"",
			 owl_mainwin_is_curmsg_truncated(owl_global_get_mainwin(&g)) ?
			 ", press Enter to scroll" : "");
    /* if (!skip_deleted) owl_function_beep(); */
  }

  if (last_if_none || found) {
    owl_global_set_curmsg(&g, i);
    owl_function_calculate_topmsg(OWL_DIRECTION_DOWNWARDS);
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
    owl_global_set_direction_downwards(&g);
  }
}

void owl_function_prevmsg_full(const char *filter, int skip_deleted, int first_if_none)
{
  int curmsg, i, found;
  const owl_view *v;
  const owl_filter *f = NULL;
  const owl_message *m;

  v=owl_global_get_current_view(&g);

  if (filter) {
    f=owl_global_get_filter(&g, filter);
    if (!f) {
      owl_function_error("No %s filter defined", filter);
      return;
    }
  }

  curmsg=owl_global_get_curmsg(&g);
  found=0;

  /* just check to make sure we're in bounds... */
  if (curmsg<0) curmsg=0;

  for (i=curmsg-1; i>=0; i--) {
    m=owl_view_get_element(v, i);
    if (skip_deleted && owl_message_is_delete(m)) continue;
    if (f && !owl_filter_message_match(f, m)) continue;
    found = 1;
    break;
  }

  if (i<0) i=0;

  if (!found) {
    owl_function_makemsg("already at first%s message%s%s",
			 skip_deleted?" non-deleted":"",
			 filter?" in ":"", filter?filter:"");
    /* if (!skip_deleted) owl_function_beep(); */
  }

  if (first_if_none || found) {
    owl_global_set_curmsg(&g, i);
    owl_function_calculate_topmsg(OWL_DIRECTION_UPWARDS);
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
    owl_global_set_direction_upwards(&g);
  }
}

void owl_function_nextmsg(void)
{
  owl_function_nextmsg_full(NULL, 0, 1);
}

void owl_function_prevmsg(void)
{
  owl_function_prevmsg_full(NULL, 0, 1);
}

void owl_function_nextmsg_notdeleted(void)
{
  owl_function_nextmsg_full(NULL, 1, 1);
}

void owl_function_prevmsg_notdeleted(void)
{
  owl_function_prevmsg_full(NULL, 1, 1);
}

/* if move_after is 1, moves after the delete */
void owl_function_deletecur(int move_after)
{
  int curmsg;
  owl_view *v;

  v=owl_global_get_current_view(&g);

  /* bail if there's no current message */
  if (owl_view_get_size(v) < 1) {
    owl_function_error("No current message to delete");
    return;
  }

  /* mark the message for deletion */
  curmsg=owl_global_get_curmsg(&g);
  owl_view_delete_element(v, curmsg);

  if (move_after) {
    /* move the poiner in the appropriate direction 
     * to the next undeleted msg */
    if (owl_global_get_direction(&g)==OWL_DIRECTION_UPWARDS) {
      owl_function_prevmsg_notdeleted();
    } else {
      owl_function_nextmsg_notdeleted();
    }
  }
}

void owl_function_undeletecur(int move_after)
{
  int curmsg;
  owl_view *v;

  v=owl_global_get_current_view(&g);
  
  if (owl_view_get_size(v) < 1) {
    owl_function_error("No current message to undelete");
    return;
  }
  curmsg=owl_global_get_curmsg(&g);

  owl_view_undelete_element(v, curmsg);

  if (move_after) {
    if (owl_global_get_direction(&g)==OWL_DIRECTION_UPWARDS) {
      if (curmsg>0) {
	owl_function_prevmsg();
      } else {
	owl_function_nextmsg();
      }
    } else {
      owl_function_nextmsg();
    }
  }

  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

void owl_function_expunge(void)
{
  int curmsg;
  const owl_message *m;
  owl_messagelist *ml;
  owl_view *v;
  int lastmsgid=0;

  curmsg=owl_global_get_curmsg(&g);
  v=owl_global_get_current_view(&g);
  ml=owl_global_get_msglist(&g);

  m=owl_view_get_element(v, curmsg);
  if (m) lastmsgid = owl_message_get_id(m);

  /* expunge the message list */
  owl_messagelist_expunge(ml);

  /* update all views (we only have one right now) */
  owl_view_recalculate(v);

  /* find where the new position should be
     (as close as possible to where we last where) */
  curmsg = owl_view_get_nearest_to_msgid(v, lastmsgid);
  if (curmsg>owl_view_get_size(v)-1) curmsg = owl_view_get_size(v)-1;
  if (curmsg<0) curmsg = 0;
  owl_global_set_curmsg(&g, curmsg);
  owl_function_calculate_topmsg(OWL_DIRECTION_NONE);
  /* if there are no messages set the direction to down in case we
     delete everything upwards */
  owl_global_set_direction_downwards(&g);
  
  owl_function_makemsg("Messages expunged");
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

void owl_function_firstmsg(void)
{
  owl_global_set_curmsg(&g, 0);
  owl_global_set_topmsg(&g, 0);
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  owl_global_set_direction_downwards(&g);
}

void owl_function_lastmsg_noredisplay(void)
{
  int oldcurmsg, curmsg;
  const owl_view *v;

  v=owl_global_get_current_view(&g);
  oldcurmsg=owl_global_get_curmsg(&g);
  curmsg=owl_view_get_size(v)-1;  
  if (curmsg<0) curmsg=0;
  owl_global_set_curmsg(&g, curmsg);
  if (oldcurmsg < curmsg) {
    owl_function_calculate_topmsg(OWL_DIRECTION_DOWNWARDS);
  } else if (curmsg<owl_view_get_size(v)) {
    /* If already at the end, blank the screen and move curmsg
     * past the end of the messages. */
    owl_global_set_topmsg(&g, curmsg+1);
    owl_global_set_curmsg(&g, curmsg+1);
  } 
  /* owl_mainwin_redisplay(owl_global_get_mainwin(&g)); */
  owl_global_set_direction_downwards(&g);
}

void owl_function_lastmsg(void)
{
  owl_function_lastmsg_noredisplay();
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));  
}

void owl_function_shift_right(void)
{
  owl_global_set_rightshift(&g, owl_global_get_rightshift(&g)+10);
}

void owl_function_shift_left(void)
{
  int shift;

  shift=owl_global_get_rightshift(&g);
  if (shift > 0) {
    owl_global_set_rightshift(&g, MAX(shift - 10, 0));
  } else {
    owl_function_beep();
    owl_function_makemsg("Already full left");
  }
}

void owl_function_unsuball(void)
{
  unsuball();
  owl_function_makemsg("Unsubscribed from all messages.");
}


/* Load zephyr subscriptions from the named 'file' and load zephyr's
 * default subscriptions as well.  An error message is printed if
 * 'file' can't be opened or if zephyr reports an error in
 * subscribing.
 *
 * If 'file' is NULL, this look for the default filename
 * $HOME/.zephyr.subs.  If the file can not be opened in this case
 * only, no error message is printed.
 */
void owl_function_loadsubs(const char *file)
{
  int ret, ret2;
  const char *foo;
  char *path;

  if (file==NULL) {
    ret=owl_zephyr_loadsubs(NULL, 0);
  } else {
    path = owl_util_makepath(file);
    ret=owl_zephyr_loadsubs(path, 1);
    owl_free(path);
  }

  /* for backwards compatibility for now */
  ret2=owl_zephyr_loaddefaultsubs();

  if (!owl_context_is_interactive(owl_global_get_context(&g))) return;

  foo=file?file:"file";
  if (ret==0 && ret2==0) {
    if (!file) {
      owl_function_makemsg("Subscribed to messages.");
    } else {
      owl_function_makemsg("Subscribed to messages from %s", file);
    }
  } else if (ret==-1) {
    owl_function_error("Could not read %s", foo);
  } else {
    owl_function_error("Error subscribing to messages");
  }
}

void owl_function_loadloginsubs(const char *file)
{
  int ret;

  ret=owl_zephyr_loadloginsubs(file);

  if (!owl_context_is_interactive(owl_global_get_context(&g))) return;
  if (ret==0) {
  } else if (ret==-1) {
    owl_function_error("Could not open file for login subscriptions.");
  } else {
    owl_function_error("Error subscribing to login messages from file.");
  }
}

void owl_callback_aimlogin(owl_editwin *e) {
  char *user = owl_editwin_get_cbdata(e);
  owl_function_aimlogin(user,
                        owl_editwin_get_text(e));
}

void owl_function_aimlogin(const char *user, const char *passwd) {
  int ret;

  /* clear the buddylist */
  owl_buddylist_clear(owl_global_get_buddylist(&g));

  /* try to login */
  ret=owl_aim_login(user, passwd);
  if (ret) owl_function_makemsg("Warning: login for %s failed.\n", user);
}

void owl_function_suspend(void)
{
  endwin();
  printf("\n");
  kill(getpid(), SIGSTOP);

  /* resize to reinitialize all the windows when we come back */
  owl_command_resize();
}

void owl_function_zaway_toggle(void)
{
  if (!owl_global_is_zaway(&g)) {
    owl_global_set_zaway_msg(&g, owl_global_get_zaway_msg_default(&g));
    owl_function_zaway_on();
  } else {
    owl_function_zaway_off();
  }
}

void owl_function_zaway_on(void)
{
  owl_global_set_zaway_on(&g);
  owl_function_makemsg("zaway set (%s)", owl_global_get_zaway_msg(&g));
}

void owl_function_zaway_off(void)
{
  owl_global_set_zaway_off(&g);
  owl_function_makemsg("zaway off");
}

void owl_function_aaway_toggle(void)
{
  if (!owl_global_is_aaway(&g)) {
    owl_global_set_aaway_msg(&g, owl_global_get_aaway_msg_default(&g));
    owl_function_aaway_on();
  } else {
    owl_function_aaway_off();
  }
}

void owl_function_aaway_on(void)
{
  owl_global_set_aaway_on(&g);
  /* owl_aim_set_awaymsg(owl_global_get_zaway_msg(&g)); */
  owl_function_makemsg("AIM away set (%s)", owl_global_get_aaway_msg(&g));
}

void owl_function_aaway_off(void)
{
  owl_global_set_aaway_off(&g);
  /* owl_aim_set_awaymsg(""); */
  owl_function_makemsg("AIM away off");
}

void owl_function_quit(void)
{
  char *ret;
  
  /* zlog out if we need to */
  if (owl_global_is_havezephyr(&g) &&
      owl_global_is_shutdownlogout(&g)) {
    owl_zephyr_zlog_out();
  }

  /* execute the commands in shutdown */
  ret = owl_perlconfig_execute("BarnOwl::Hooks::_shutdown();");
  if (ret) owl_free(ret);

  /* signal our child process, if any */
  if (owl_global_get_newmsgproc_pid(&g)) {
    kill(owl_global_get_newmsgproc_pid(&g), SIGHUP);
  }
  
  /* Quit AIM */
  if (owl_global_is_aimloggedin(&g)) {
    owl_aim_logout();
  }

  owl_function_debugmsg("Quitting Owl");
  owl_select_quit_loop();
}

void owl_function_calculate_topmsg(int direction)
{
  int recwinlines, topmsg, curmsg;
  const owl_view *v;

  v=owl_global_get_current_view(&g);
  curmsg=owl_global_get_curmsg(&g);
  topmsg=owl_global_get_topmsg(&g);
  recwinlines=owl_global_get_recwin_lines(&g);

  /*
  if (owl_view_get_size(v) < 1) {
    return;
  }
  */

  switch (owl_global_get_scrollmode(&g)) {
  case OWL_SCROLLMODE_TOP:
    topmsg = owl_function_calculate_topmsg_top(direction, v, curmsg, topmsg, recwinlines);
    break;
  case OWL_SCROLLMODE_NEARTOP:
    topmsg = owl_function_calculate_topmsg_neartop(direction, v, curmsg, topmsg, recwinlines);
    break;
  case OWL_SCROLLMODE_CENTER:
    topmsg = owl_function_calculate_topmsg_center(direction, v, curmsg, topmsg, recwinlines);
    break;
  case OWL_SCROLLMODE_PAGED:
    topmsg = owl_function_calculate_topmsg_paged(direction, v, curmsg, topmsg, recwinlines, 0);
    break;
  case OWL_SCROLLMODE_PAGEDCENTER:
    topmsg = owl_function_calculate_topmsg_paged(direction, v, curmsg, topmsg, recwinlines, 1);
    break;
  case OWL_SCROLLMODE_NORMAL:
  default:
    topmsg = owl_function_calculate_topmsg_normal(direction, v, curmsg, topmsg, recwinlines);
  }
  owl_function_debugmsg("Calculated a topmsg of %i", topmsg);
  owl_global_set_topmsg(&g, topmsg);
}

/* Returns what the new topmsg should be.  
 * Passed the last direction of movement, 
 * the current view,
 * the current message number in the view,
 * the top message currently being displayed,
 * and the number of lines in the recwin.
 */
int owl_function_calculate_topmsg_top(int direction, const owl_view *v, int curmsg, int topmsg, int recwinlines)
{
  return(curmsg);
}

int owl_function_calculate_topmsg_neartop(int direction, const owl_view *v, int curmsg, int topmsg, int recwinlines)
{
  if (curmsg>0 
      && (owl_message_get_numlines(owl_view_get_element(v, curmsg-1))
	  <  recwinlines/2)) {
    return(curmsg-1);
  } else {
    return(curmsg);
  }
}
  
int owl_function_calculate_topmsg_center(int direction, const owl_view *v, int curmsg, int topmsg, int recwinlines)
{
  int i, last, lines;

  last = curmsg;
  lines = 0;
  for (i=curmsg-1; i>=0; i--) {
    lines += owl_message_get_numlines(owl_view_get_element(v, i));
    if (lines > recwinlines/2) break;
    last = i;
  }
  return(last);
}
  
int owl_function_calculate_topmsg_paged(int direction, const owl_view *v, int curmsg, int topmsg, int recwinlines, int center_on_page)
{
  int i, last, lines, savey;
  
  /* If we're off the top of the screen, scroll up such that the 
   * curmsg is near the botton of the screen. */
  if (curmsg < topmsg) {
    last = curmsg;
    lines = 0;
    for (i=curmsg; i>=0; i--) {
      lines += owl_message_get_numlines(owl_view_get_element(v, i));
      if (lines > recwinlines) break;
    last = i;
    }
    if (center_on_page) {
      return(owl_function_calculate_topmsg_center(direction, v, curmsg, 0, recwinlines));
    } else {
      return(last);
    }
  }

  /* Find number of lines from top to bottom of curmsg (store in savey) */
  savey=0;
  for (i=topmsg; i<=curmsg; i++) {
    savey+=owl_message_get_numlines(owl_view_get_element(v, i));
  }

  /* if we're off the bottom of the screen, scroll down */
  if (savey > recwinlines) {
    if (center_on_page) {
      return(owl_function_calculate_topmsg_center(direction, v, curmsg, 0, recwinlines));
    } else {
      return(curmsg);
    }
  }

  /* else just stay as we are... */
  return(topmsg);
}

int owl_function_calculate_topmsg_normal(int direction, const owl_view *v, int curmsg, int topmsg, int recwinlines)
{
  int savey, i, foo, y;

  if (curmsg<0) return(topmsg);
    
  /* If we're off the top of the screen then center */
  if (curmsg<topmsg) {
    topmsg=owl_function_calculate_topmsg_center(direction, v, curmsg, 0, recwinlines);
  }

  /* If curmsg is so far past topmsg that there are more messages than
     lines, skip the line counting that follows because we're
     certainly off screen.  */
  savey=curmsg-topmsg;
  if (savey <= recwinlines) {
    /* Find number of lines from top to bottom of curmsg (store in savey) */
    savey = 0;
    for (i=topmsg; i<=curmsg; i++) {
      savey+=owl_message_get_numlines(owl_view_get_element(v, i));
    }
  }

  /* If we're off the bottom of the screen, set the topmsg to curmsg
   * and scroll upwards */
  if (savey > recwinlines) {
    topmsg=curmsg;
    savey=owl_message_get_numlines(owl_view_get_element(v, curmsg));
    direction=OWL_DIRECTION_UPWARDS;
  }
  
  /* If our bottom line is less than 1/4 down the screen then scroll up */
  if (direction == OWL_DIRECTION_UPWARDS || direction == OWL_DIRECTION_NONE) {
    if (savey < (recwinlines / 4)) {
      y=0;
      for (i=curmsg; i>=0; i--) {
	foo=owl_message_get_numlines(owl_view_get_element(v, i));
	/* will we run the curmsg off the screen? */
	if ((foo+y) >= recwinlines) {
	  i++;
	  if (i>curmsg) i=curmsg;
	  break;
	}
	/* have saved 1/2 the screen space? */
	y+=foo;
	if (y > (recwinlines / 2)) break;
      }
      if (i<0) i=0;
      return(i);
    }
  }

  if (direction == OWL_DIRECTION_DOWNWARDS || direction == OWL_DIRECTION_NONE) {
    /* If curmsg bottom line is more than 3/4 down the screen then scroll down */
    if (savey > ((recwinlines * 3)/4)) {
      y=0;
      /* count lines from the top until we can save 1/2 the screen size */
      for (i=topmsg; i<curmsg; i++) {
	y+=owl_message_get_numlines(owl_view_get_element(v, i));
	if (y > (recwinlines / 2)) break;
      }
      if (i==curmsg) {
	i--;
      }
      return(i+1);
    }
  }

  return(topmsg);
}

void owl_function_resize(void)
{
  owl_global_set_resize_pending(&g);
}

void owl_function_debugmsg(const char *fmt, ...)
{
  FILE *file;
  time_t now;
  va_list ap;
  va_start(ap, fmt);

  if (!owl_global_is_debug_fast(&g))
    return;

  file = owl_global_get_debug_file_handle(&g);
  if (!file) /* XXX should report this */
    return;

  now = time(NULL);

  fprintf(file, "[%d -  %.24s - %lds]: ",
	  (int) getpid(), ctime(&now), now - owl_global_get_starttime(&g));
  vfprintf(file, fmt, ap);
  putc('\n', file);
  fflush(file);

  va_end(ap);
}

void owl_function_beep(void)
{
  if (owl_global_is_bell(&g)) {
    beep();
  }
}

int owl_function_subscribe(const char *class, const char *inst, const char *recip)
{
  int ret;

  ret=owl_zephyr_sub(class, inst, recip);
  if (ret) {
    owl_function_error("Error subscribing.");
  } else {
    owl_function_makemsg("Subscribed.");
  }
  return(ret);
}

void owl_function_unsubscribe(const char *class, const char *inst, const char *recip)
{
  int ret;

  ret=owl_zephyr_unsub(class, inst, recip);
  if (ret) {
    owl_function_error("Error subscribing.");
  } else {
    owl_function_makemsg("Unsubscribed.");
  }
}

static void _dirty_everything(owl_window *w) {
  if (!owl_window_is_realized(w))
    return;
  owl_window_dirty(w);
  owl_window_children_foreach(w, (GFunc)_dirty_everything, NULL);
}

void owl_function_full_redisplay(void)
{
  _dirty_everything(owl_window_get_screen());
}

void owl_function_popless_text(const char *text)
{
  owl_popwin *pw;
  owl_viewwin *v;

  pw=owl_global_get_popwin(&g);
  v=owl_global_get_viewwin(&g);

  if (owl_popwin_up(pw) != 0) {
    owl_function_error("Popwin already in use.");
    return;
  }
  owl_global_push_context(&g, OWL_CTX_POPLESS, v, "popless", NULL);
  owl_viewwin_init_text(v, owl_popwin_get_content(pw), text);
}

void owl_function_popless_fmtext(const owl_fmtext *fm)
{
  owl_popwin *pw;
  owl_viewwin *v;

  pw=owl_global_get_popwin(&g);
  v=owl_global_get_viewwin(&g);

  if (owl_popwin_up(pw) != 0) {
    owl_function_error("Popwin already in use.");
    return;
  }
  owl_global_push_context(&g, OWL_CTX_POPLESS, v, "popless", NULL);
  owl_viewwin_init_fmtext(v, owl_popwin_get_content(pw), fm);
}

void owl_function_popless_file(const char *filename)
{
  owl_fmtext fm;
  FILE *file;
  char *s = NULL;

  file=fopen(filename, "r");
  if (!file) {
    owl_function_error("Could not open file: %s", filename);
    return;
  }

  owl_fmtext_init_null(&fm);
  while (owl_getline(&s, file))
    owl_fmtext_append_normal(&fm, s);
  owl_free(s);

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
  fclose(file);
}

void owl_function_about(void)
{
  owl_function_popless_text(
    "This is barnowl version " OWL_VERSION_STRING ".\n\n"
    "barnowl is a fork of the Owl zephyr client, written and\n"
    "maintained by Alejandro Sedeno and Nelson Elhage at the\n"
    "Massachusetts Institute of Technology. \n"
    "\n"
    "Owl was written by James Kretchmar. The first version, 0.5, was\n"
    "released in March 2002.\n"
    "\n"
    "The name 'owl' was chosen in reference to the owls in the\n"
    "Harry Potter novels, who are tasked with carrying messages\n"
    "between Witches and Wizards. The name 'barnowl' was chosen\n"
    "because we feel our owls should live closer to our ponies.\n"
    "\n"
    "Copyright (c) 2006-2010 The BarnOwl Developers. All rights reserved.\n"
    "Copyright (c) 2004 James Kretchmar. All rights reserved.\n"
    "Copyright 2002 Massachusetts Institute of Technology\n"
    "\n"
    "This program is free software. You can redistribute it and/or\n"
    "modify under the terms of the Sleepycat License. Use the \n"
    "':show license' command to display the full license\n"
  );
}

void owl_function_info(void)
{
  const owl_message *m;
  owl_fmtext fm, attrfm;
  const owl_view *v;
#ifdef HAVE_LIBZEPHYR
  const ZNotice_t *n;
#endif

  owl_fmtext_init_null(&fm);
  
  v=owl_global_get_current_view(&g);
  m=owl_view_get_element(v, owl_global_get_curmsg(&g));
  if (!m || owl_view_get_size(v)==0) {
    owl_function_error("No message selected\n");
    return;
  }

  owl_fmtext_append_bold(&fm, "General Information:\n");
  owl_fmtext_appendf_normal(&fm, "  Msg Id    : %i\n", owl_message_get_id(m));

  owl_fmtext_append_normal(&fm, "  Type      : ");
  owl_fmtext_append_bold(&fm, owl_message_get_type(m));
  owl_fmtext_append_normal(&fm, "\n");

  if (owl_message_is_direction_in(m)) {
    owl_fmtext_append_normal(&fm, "  Direction : in\n");
  } else if (owl_message_is_direction_out(m)) {
    owl_fmtext_append_normal(&fm, "  Direction : out\n");
  } else if (owl_message_is_direction_none(m)) {
    owl_fmtext_append_normal(&fm, "  Direction : none\n");
  } else {
    owl_fmtext_append_normal(&fm, "  Direction : unknown\n");
  }

  owl_fmtext_appendf_normal(&fm, "  Time      : %s\n", owl_message_get_timestr(m));

  if (!owl_message_is_type_admin(m)) {
    owl_fmtext_appendf_normal(&fm, "  Sender    : %s\n", owl_message_get_sender(m));
    owl_fmtext_appendf_normal(&fm, "  Recipient : %s\n", owl_message_get_recipient(m));
  }

  if (owl_message_is_type_zephyr(m)) {
    owl_fmtext_append_bold(&fm, "\nZephyr Specific Information:\n");
    
    owl_fmtext_appendf_normal(&fm, "  Class     : %s\n", owl_message_get_class(m));
    owl_fmtext_appendf_normal(&fm, "  Instance  : %s\n", owl_message_get_instance(m));
    owl_fmtext_appendf_normal(&fm, "  Opcode    : %s\n", owl_message_get_opcode(m));
#ifdef HAVE_LIBZEPHYR
    if (owl_message_is_direction_in(m)) {
      char *ptr, tmpbuff[1024];
      int i, j, fields, len;

      n=owl_message_get_notice(m);

      if (!owl_message_is_pseudo(m)) {
	owl_fmtext_append_normal(&fm, "  Kind      : ");
	if (n->z_kind==UNSAFE) {
	  owl_fmtext_append_normal(&fm, "UNSAFE\n");
	} else if (n->z_kind==UNACKED) {
	  owl_fmtext_append_normal(&fm, "UNACKED\n");
	} else if (n->z_kind==ACKED) {
	  owl_fmtext_append_normal(&fm, "ACKED\n");
	} else if (n->z_kind==HMACK) {
	  owl_fmtext_append_normal(&fm, "HMACK\n");
	} else if (n->z_kind==HMCTL) {
	  owl_fmtext_append_normal(&fm, "HMCTL\n");
	} else if (n->z_kind==SERVACK) {
	  owl_fmtext_append_normal(&fm, "SERVACK\n");
	} else if (n->z_kind==SERVNAK) {
	  owl_fmtext_append_normal(&fm, "SERVNACK\n");
	} else if (n->z_kind==CLIENTACK) {
	  owl_fmtext_append_normal(&fm, "CLIENTACK\n");
	} else if (n->z_kind==STAT) {
	  owl_fmtext_append_normal(&fm, "STAT\n");
	} else {
	  owl_fmtext_append_normal(&fm, "ILLEGAL VALUE\n");
	}
      }
      owl_fmtext_appendf_normal(&fm, "  Host      : %s\n", owl_message_get_hostname(m));

      if (!owl_message_is_pseudo(m)) {
	owl_fmtext_append_normal(&fm, "\n");
	owl_fmtext_appendf_normal(&fm, "  Port      : %i\n", ntohs(n->z_port));
	owl_fmtext_appendf_normal(&fm, "  Auth      : %s\n", owl_zephyr_get_authstr(n));

	/* FIXME make these more descriptive */
	owl_fmtext_appendf_normal(&fm, "  Checkd Ath: %i\n", n->z_checked_auth);
	owl_fmtext_appendf_normal(&fm, "  Multi notc: %s\n", n->z_multinotice);
	owl_fmtext_appendf_normal(&fm, "  Num other : %i\n", n->z_num_other_fields);
	owl_fmtext_appendf_normal(&fm, "  Msg Len   : %i\n", n->z_message_len);

	fields=owl_zephyr_get_num_fields(n);
	owl_fmtext_appendf_normal(&fm, "  Fields    : %i\n", fields);

	for (i=0; i<fields; i++) {
	  ptr=owl_zephyr_get_field_as_utf8(n, i+1);
	  len=strlen(ptr);
	  if (len<30) {
	    strncpy(tmpbuff, ptr, len);
	    tmpbuff[len]='\0';
	  } else {
	    strncpy(tmpbuff, ptr, 30);
	    tmpbuff[30]='\0';
	    strcat(tmpbuff, "...");
	  }
	  owl_free(ptr);

	  for (j=0; j<strlen(tmpbuff); j++) {
	    if (tmpbuff[j]=='\n') tmpbuff[j]='~';
	    if (tmpbuff[j]=='\r') tmpbuff[j]='!';
	  }

	  owl_fmtext_appendf_normal(&fm, "  Field %i   : %s\n", i+1, tmpbuff);
	}
	owl_fmtext_appendf_normal(&fm, "  Default Fm: %s\n", n->z_default_format);
      }

    }
#endif
  }

  owl_fmtext_append_bold(&fm, "\nOwl Message Attributes:\n");
  owl_message_attributes_tofmtext(m, &attrfm);
  owl_fmtext_append_fmtext(&fm, &attrfm);
  
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
  owl_fmtext_cleanup(&attrfm);
}

/* print the current message in a popup window.
 * Use the 'default' style regardless of whatever
 * style the user may be using
 */
void owl_function_curmsg_to_popwin(void)
{
  const owl_view *v;
  const owl_message *m;
  const owl_style *s;
  owl_fmtext fm;

  v=owl_global_get_current_view(&g);
  s=owl_global_get_style_by_name(&g, "default");

  m=owl_view_get_element(v, owl_global_get_curmsg(&g));

  if (!m || owl_view_get_size(v)==0) {
    owl_function_error("No current message");
    return;
  }

  owl_fmtext_init_null(&fm);
  owl_style_get_formattext(s, &fm, m);

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_page_curmsg(int step)
{
  /* scroll down or up within the current message IF the message is truncated */

  int offset, curmsg, lines;
  const owl_view *v;
  owl_message *m;

  offset=owl_global_get_curmsg_vert_offset(&g);
  v=owl_global_get_current_view(&g);
  curmsg=owl_global_get_curmsg(&g);
  m=owl_view_get_element(v, curmsg);
  if (!m || owl_view_get_size(v)==0) return;
  lines=owl_message_get_numlines(m);

  if (offset==0) {
    /* Bail if the curmsg isn't the last one displayed */
    if (curmsg != owl_mainwin_get_last_msg(owl_global_get_mainwin(&g))) {
      owl_function_makemsg("The entire message is already displayed");
      return;
    }
    
    /* Bail if we're not truncated */
    if (!owl_mainwin_is_curmsg_truncated(owl_global_get_mainwin(&g))) {
      owl_function_makemsg("The entire message is already displayed");
      return;
    }
  }
  
  
  /* don't scroll past the last line */
  if (step>0) {
    if (offset+step > lines-1) {
      owl_global_set_curmsg_vert_offset(&g, lines-1);
    } else {
      owl_global_set_curmsg_vert_offset(&g, offset+step);
    }
  }

  /* would we be before the beginning of the message? */
  if (step<0) {
    if (offset+step<0) {
      owl_global_set_curmsg_vert_offset(&g, 0);
    } else {
      owl_global_set_curmsg_vert_offset(&g, offset+step);
    }
  }
  
  /* redisplay */
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

void owl_function_resize_typwin(int newsize)
{
  owl_global_set_typwin_lines(&g, newsize);
  owl_mainpanel_layout_contents(&g.mainpanel);
}

void owl_function_mainwin_pagedown(void)
{
  int i;

  i=owl_mainwin_get_last_msg(owl_global_get_mainwin(&g));
  if (i<0) return;
  if (owl_mainwin_is_last_msg_truncated(owl_global_get_mainwin(&g))
      && (owl_global_get_curmsg(&g) < i)
      && (i>0)) {
    i--;
  }
  owl_global_set_curmsg(&g, i);
  owl_function_nextmsg();
}

void owl_function_mainwin_pageup(void)
{
  owl_global_set_curmsg(&g, owl_global_get_topmsg(&g));
  owl_function_prevmsg();
}

void owl_function_getsubs(void)
{
  char *buff;

  buff=owl_zephyr_getsubs();

  if (buff) {
    owl_function_popless_text(buff);
  } else {
    owl_function_popless_text("Error getting subscriptions");
  }
	   
  owl_free(buff);
}

void owl_function_printallvars(void)
{
  const char *name;
  char var[LINE];
  owl_list varnames;
  int i, numvarnames;
  GString *str   = g_string_new("");

  g_string_append_printf(str, "%-20s = %s\n", "VARIABLE", "VALUE");
  g_string_append_printf(str, "%-20s   %s\n",  "--------", "-----");
  owl_variable_dict_get_names(owl_global_get_vardict(&g), &varnames);
  numvarnames = owl_list_get_size(&varnames);
  for (i=0; i<numvarnames; i++) {
    name = owl_list_get_element(&varnames, i);
    if (name && name[0]!='_') {
      g_string_append_printf(str, "\n%-20s = ", name);
      owl_variable_get_tostring(owl_global_get_vardict(&g), name, var, LINE);
      g_string_append(str, var);
    }
  }
  g_string_append(str, "\n");
  owl_variable_dict_namelist_cleanup(&varnames);

  owl_function_popless_text(str->str);
  g_string_free(str, TRUE);
}

void owl_function_show_variables(void)
{
  owl_list varnames;
  owl_fmtext fm;  
  int i, numvarnames;
  const char *varname;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_bold(&fm, 
      "Variables: (use 'show variable <name>' for details)\n");
  owl_variable_dict_get_names(owl_global_get_vardict(&g), &varnames);
  numvarnames = owl_list_get_size(&varnames);
  for (i=0; i<numvarnames; i++) {
    varname = owl_list_get_element(&varnames, i);
    if (varname && varname[0]!='_') {
      owl_variable_describe(owl_global_get_vardict(&g), varname, &fm);
    }
  }
  owl_variable_dict_namelist_cleanup(&varnames);
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_show_variable(const char *name)
{
  owl_fmtext fm;  

  owl_fmtext_init_null(&fm);
  owl_variable_get_help(owl_global_get_vardict(&g), name, &fm);
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

/* note: this applies to global message list, not to view.
 * If flag is 1, deletes.  If flag is 0, undeletes. */
void owl_function_delete_by_id(int id, int flag)
{
  const owl_messagelist *ml;
  owl_message *m;
  ml = owl_global_get_msglist(&g);
  m = owl_messagelist_get_by_id(ml, id);
  if (m) {
    if (flag == 1) {
      owl_message_mark_delete(m);
    } else if (flag == 0) {
      owl_message_unmark_delete(m);
    }
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  } else {
    owl_function_error("No message with id %d: unable to mark for (un)delete",id);
  }
}

void owl_function_delete_automsgs(void)
{
  /* mark for deletion all messages in the current view that match the
   * 'trash' filter */

  int i, j, count;
  owl_message *m;
  const owl_view *v;
  const owl_filter *f;

  /* get the trash filter */
  f=owl_global_get_filter(&g, "trash");
  if (!f) {
    owl_function_error("No trash filter defined");
    return;
  }

  v=owl_global_get_current_view(&g);

  count=0;
  j=owl_view_get_size(v);
  for (i=0; i<j; i++) {
    m=owl_view_get_element(v, i);
    if (owl_filter_message_match(f, m)) {
      count++;
      owl_message_mark_delete(m);
    }
  }
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  owl_function_makemsg("%i messages marked for deletion", count);
}

void owl_function_status(void)
{
  char buff[MAXPATHLEN+1];
  time_t start;
  int up, days, hours, minutes;
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);

  start=owl_global_get_starttime(&g);

  owl_fmtext_append_normal(&fm, "General Information:\n");

  owl_fmtext_append_normal(&fm, "  Version: ");
  owl_fmtext_append_normal(&fm, OWL_VERSION_STRING);
  owl_fmtext_append_normal(&fm, "\n");

  owl_fmtext_append_normal(&fm, "  Startup Arguments: ");
  owl_fmtext_append_normal(&fm, owl_global_get_startupargs(&g));
  owl_fmtext_append_normal(&fm, "\n");

  owl_fmtext_append_normal(&fm, "  Current Directory: ");
  if(getcwd(buff, MAXPATHLEN) == NULL) {
    owl_fmtext_append_normal(&fm, "<Error in getcwd>");
  } else {
    owl_fmtext_append_normal(&fm, buff);
  }
  owl_fmtext_append_normal(&fm, "\n");

  owl_fmtext_appendf_normal(&fm, "  Startup Time: %s", ctime(&start));

  up=owl_global_get_runtime(&g);
  days=up/86400;
  up-=days*86400;
  hours=up/3600;
  up-=hours*3600;
  minutes=up/60;
  up-=minutes*60;
  owl_fmtext_appendf_normal(&fm, "  Run Time: %i days %2.2i:%2.2i:%2.2i\n", days, hours, minutes, up);

  owl_fmtext_append_normal(&fm, "\nProtocol Options:\n");
  owl_fmtext_append_normal(&fm, "  Zephyr included    : ");
  if (owl_global_is_havezephyr(&g)) {
    owl_fmtext_append_normal(&fm, "yes\n");
  } else {
    owl_fmtext_append_normal(&fm, "no\n");
  }
  owl_fmtext_append_normal(&fm, "  AIM included       : yes\n");
  owl_fmtext_append_normal(&fm, "  Loopback included  : yes\n");


  owl_fmtext_append_normal(&fm, "\nBuild Options:\n");
  owl_fmtext_append_normal(&fm, "  Stderr redirection : ");
#if OWL_STDERR_REDIR
  owl_fmtext_append_normal(&fm, "yes\n");
#else
  owl_fmtext_append_normal(&fm, "no\n");
#endif
  

  owl_fmtext_append_normal(&fm, "\nAIM Status:\n");
  owl_fmtext_append_normal(&fm, "  Logged in: ");
  if (owl_global_is_aimloggedin(&g)) {
    owl_fmtext_append_normal(&fm, owl_global_get_aim_screenname(&g));
    owl_fmtext_append_normal(&fm, "\n");
  } else {
    owl_fmtext_append_normal(&fm, "(not logged in)\n");
  }

  owl_fmtext_append_normal(&fm, "  Processing events: ");
  if (owl_global_is_doaimevents(&g)) {
    owl_fmtext_append_normal(&fm, "yes\n");
  } else {
    owl_fmtext_append_normal(&fm, "no\n");
  }

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_show_term(void)
{
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);
  owl_fmtext_appendf_normal(&fm, "Terminal Lines: %i\nTerminal Columns: %i\n",
	  owl_global_get_lines(&g),
	  owl_global_get_cols(&g));

  if (owl_global_get_hascolors(&g)) {
    owl_fmtext_append_normal(&fm, "Color: Yes\n");
    owl_fmtext_appendf_normal(&fm, "Number of color pairs: %i\n", owl_global_get_colorpairs(&g));
    owl_fmtext_appendf_normal(&fm, "Can change colors: %s\n", can_change_color() ? "yes" : "no");
  } else {
    owl_fmtext_append_normal(&fm, "Color: No\n");
  }

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

/* if type = 0 then normal reply.
 * if type = 1 then it's a reply to sender
 * if enter = 0 then allow the command to be edited
 * if enter = 1 then don't wait for editing
 */
void owl_function_reply(int type, int enter)
{
  char *buff=NULL;
  const owl_message *m;
  const owl_filter *f;
  
  if (owl_view_get_size(owl_global_get_current_view(&g))==0) {
    owl_function_error("No message selected");
  } else {
    char *cmd;
    
    m=owl_view_get_element(owl_global_get_current_view(&g), owl_global_get_curmsg(&g));
    if (!m) {
      owl_function_error("No message selected");
      return;
    }

    /* first check if we catch the reply-lockout filter */
    f=owl_global_get_filter(&g, "reply-lockout");
    if (f) {
      if (owl_filter_message_match(f, m)) {
	owl_function_error("Sorry, replies to this message have been disabled by the reply-lockout filter");
	return;
      }
    }

    /* then check if it's a question and just bring up the command prompt */
    if (owl_message_is_question(m)) {
      owl_function_start_command("");
      return;
    }

    if((type == 0 &&
        (cmd=owl_perlconfig_message_call_method(m, "replycmd", 0, NULL))) ||
       (type == 1 &&
        (cmd=owl_perlconfig_message_call_method(m, "replysendercmd", 0, NULL)))) {
      buff = cmd;
    }

    if(!buff) {
        owl_function_error("I don't know how to reply to that message.");
        return;
    }

    if (enter) {
      owl_history *hist = owl_global_get_cmd_history(&g);
      owl_history_store(hist, buff);
      owl_history_reset(hist);
      owl_function_command_norv(buff);
    } else {
      owl_function_start_command(buff);
    }
    owl_free(buff);
  }
}

void owl_function_zlocate(int argc, const char *const *argv, int auth)
{
  owl_fmtext fm;
  char *ptr;
  char *result;
  int i;

  owl_fmtext_init_null(&fm);

  for (i=0; i<argc; i++) {
    ptr = long_zuser(argv[i]);
    result = owl_zephyr_zlocate(ptr, auth);
    owl_fmtext_append_normal(&fm, result);
    owl_free(result);
    owl_free(ptr);
  }

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_callback_command(owl_editwin *e)
{
  char *rv;
  const char *line = owl_editwin_get_text(e);

  rv = owl_function_command(line);
   if (rv) {
    owl_function_makemsg("%s", rv);
    owl_free(rv);
  }
}

void owl_function_start_command(const char *line)
{
  owl_editwin *tw;

  tw = owl_global_set_typwin_active(&g, OWL_EDITWIN_STYLE_ONELINE, owl_global_get_cmd_history(&g));

  owl_editwin_set_locktext(tw, "command: ");

  owl_editwin_insert_string(tw, line);

  owl_global_push_context(&g, OWL_CTX_EDITLINE, tw, "editline", owl_global_get_typwin_window(&g));
  owl_editwin_set_callback(tw, owl_callback_command);
}

owl_editwin *owl_function_start_question(const char *line)
{
  owl_editwin *tw;

  tw = owl_global_set_typwin_active(&g, OWL_EDITWIN_STYLE_ONELINE, owl_global_get_cmd_history(&g));

  owl_editwin_set_locktext(tw, line);

  owl_global_push_context(&g, OWL_CTX_EDITRESPONSE, tw, "editresponse", owl_global_get_typwin_window(&g));
  return tw;
}

owl_editwin *owl_function_start_password(const char *line)
{
  owl_editwin *tw;

  tw = owl_global_set_typwin_active(&g, OWL_EDITWIN_STYLE_ONELINE, NULL);

  owl_editwin_set_echochar(tw, '*');

  owl_editwin_set_locktext(tw, line);

  owl_global_push_context(&g, OWL_CTX_EDITRESPONSE, tw, "editresponse", owl_global_get_typwin_window(&g));
  return tw;
}

char *owl_function_exec(int argc, const char *const *argv, const char *buff, int type)
{
  /* if type == 1 display in a popup
   * if type == 2 display an admin messages
   * if type == 0 return output
   * else display in a popup
   */
  const char *redirect = " 2>&1 < /dev/null";
  char *newbuff;
  char *out;
  FILE *p;

#if OWL_STDERR_REDIR
  redirect = " < /dev/null";
#endif 

  if (argc<2) {
    owl_function_error("Wrong number of arguments to the exec command");
    return NULL;
  }

  buff = skiptokens(buff, 1);
  newbuff = owl_sprintf("%s%s", buff, redirect);

  if (type == OWL_OUTPUT_POPUP) {
    owl_popexec_new(newbuff);
  } else {
    p = popen(newbuff, "r");
    out = owl_slurp(p);
    pclose(p);
    
    if (type == OWL_OUTPUT_RETURN) {
      owl_free(newbuff);
      return out;
    } else if (type == OWL_OUTPUT_ADMINMSG) {
      owl_function_adminmsg(buff, out);
    }
    owl_free(out);
  }
  owl_free(newbuff);
  return NULL;
}

char *owl_function_perl(int argc, const char *const *argv, const char *buff, int type)
{
  /* if type == 1 display in a popup
   * if type == 2 display an admin messages
   * if type == 0 return output
   * else display in a popup
   */
  char *perlout;

  if (argc<2) {
    owl_function_error("Wrong number of arguments to perl command");
    return NULL;
  }

  /* consume first token (argv[0]) */
  buff = skiptokens(buff, 1);

  perlout = owl_perlconfig_execute(buff);
  if (perlout) { 
    if (type == OWL_OUTPUT_POPUP) {
      owl_function_popless_text(perlout);
    } else if (type == OWL_OUTPUT_ADMINMSG) {
      owl_function_adminmsg(buff, perlout);
    } else if (type == OWL_OUTPUT_RETURN) {
      return perlout;
    }
    owl_free(perlout);
  }
  return NULL;
}

/* Change the filter associated with the current view.
 * This also figures out which message in the new filter
 * should have the pointer.
 */
void owl_function_change_currentview_filter(const char *filtname)
{
  owl_view *v;
  owl_filter *f;
  int curid=-1, newpos, curmsg;
  const owl_message *curm=NULL;

  v=owl_global_get_current_view(&g);

  curmsg=owl_global_get_curmsg(&g);
  if (curmsg==-1) {
    owl_function_debugmsg("Hit the curmsg==-1 case in change_view");
  } else {
    curm=owl_view_get_element(v, curmsg);
    if (curm) {
      curid=owl_message_get_id(curm);
      owl_view_save_curmsgid(v, curid);
    }
  }

  f=owl_global_get_filter(&g, filtname);
  if (!f) {
    owl_function_error("Unknown filter %s", filtname);
    return;
  }

  owl_view_new_filter(v, f);

  /* Figure out what to set the current message to.
   * - If the view we're leaving has messages in it, go to the closest message
   *   to the last message pointed to in that view. 
   * - If the view we're leaving is empty, try to restore the position
   *   from the last time we were in the new view.  */
  if (curm) {
    newpos = owl_view_get_nearest_to_msgid(v, curid);
  } else {
    newpos = owl_view_get_nearest_to_saved(v);
  }

  owl_global_set_curmsg(&g, newpos);
  owl_function_calculate_topmsg(OWL_DIRECTION_DOWNWARDS);
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  owl_global_set_direction_downwards(&g);
}

/* Create a new filter, or replace an existing one
 * with a new definition.
 */
void owl_function_create_filter(int argc, const char *const *argv)
{
  owl_filter *f;
  const owl_view *v;
  int inuse = 0;

  if (argc < 2) {
    owl_function_error("Wrong number of arguments to filter command");
    return;
  }

  owl_function_debugmsg("owl_function_create_filter: starting to create filter named %s", argv[1]);

  v=owl_global_get_current_view(&g);

  /* don't touch the all filter */
  if (!strcmp(argv[1], "all")) {
    owl_function_error("You may not change the 'all' filter.");
    return;
  }

  /* deal with the case of trying change the filter color */
  if (argc==4 && !strcmp(argv[2], "-c")) {
    f=owl_global_get_filter(&g, argv[1]);
    if (!f) {
      owl_function_error("The filter '%s' does not exist.", argv[1]);
      return;
    }
    if (owl_util_string_to_color(argv[3])==OWL_COLOR_INVALID) {
      owl_function_error("The color '%s' is not available.", argv[3]);
      return;
    }
    owl_filter_set_fgcolor(f, owl_util_string_to_color(argv[3]));
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
    return;
  }
  if (argc==4 && !strcmp(argv[2], "-b")) {
    f=owl_global_get_filter(&g, argv[1]);
    if (!f) {
      owl_function_error("The filter '%s' does not exist.", argv[1]);
      return;
    }
    if (owl_util_string_to_color(argv[3])==OWL_COLOR_INVALID) {
      owl_function_error("The color '%s' is not available.", argv[3]);
      return;
    }
    owl_filter_set_bgcolor(f, owl_util_string_to_color(argv[3]));
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
    return;
  }

  /* create the filter and check for errors */
  f = owl_filter_new(argv[1], argc-2, argv+2);
  if (f == NULL) {
    owl_function_error("Invalid filter");
    return;
  }

  /* if the named filter is in use by the current view, remember it */
  if (!strcmp(owl_view_get_filtname(v), argv[1])) {
    inuse=1;
  }

  /* if the named filter already exists, nuke it */
  if (owl_global_get_filter(&g, argv[1])) {
    owl_global_remove_filter(&g, argv[1]);
  }

  /* add the filter */
  owl_global_add_filter(&g, f);

  /* if it was in use by the current view then update */
  if (inuse) {
    owl_function_change_currentview_filter(argv[1]);
  }
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

/* If 'filtername' does not start with 'not-' create a filter named
 * 'not-<filtername>' defined as "not filter <filtername>".  If the
 * filter 'not-<filtername>' already exists, do not overwrite it.  If
 * 'filtername' begins with 'not-' and a filter 'filtername' already
 * exists, then do nothing.  If the filter 'filtername' does not
 * exist, create it and define it as 'not filter <filtername>'
 *
 * Returns the name of the negated filter, which the caller must free.
 */
char *owl_function_create_negative_filter(const char *filtername)
{
  char *newname;
  const owl_filter *tmpfilt;
  const char *argv[5];

  owl_function_debugmsg("owl_function_create_negative_filter");
  
  if (!strncmp(filtername, "not-", 4)) {
    newname=owl_strdup(filtername+4);
  } else {
    newname=owl_sprintf("not-%s", filtername);
  }

  tmpfilt=owl_global_get_filter(&g, newname);
  if (!tmpfilt) {
    argv[0]="filter"; /* anything is fine here */
    argv[1]=newname;
    argv[2]="not";
    argv[3]="filter";
    argv[4]=filtername;
    owl_function_create_filter(5, argv);
  }

  owl_function_debugmsg("owl_function_create_negative_filter: returning with %s", newname);
  return(newname);
}

void owl_function_show_filters(void)
{
  const owl_filter *f;
  GList *fl;
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);

  owl_fmtext_append_bold(&fm, "Filters:\n");

  for (fl = g.filterlist; fl; fl = g_list_next(fl)) {
    f = fl->data;
    owl_fmtext_append_normal(&fm, "   ");
    if (owl_global_get_hascolors(&g)) {
      owl_fmtext_append_normal_color(&fm, owl_filter_get_name(f), owl_filter_get_fgcolor(f), owl_filter_get_bgcolor(f));
    } else {
      owl_fmtext_append_normal(&fm, owl_filter_get_name(f));
    }
    owl_fmtext_append_normal(&fm, "\n");
  }
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_show_filter(const char *name)
{
  const owl_filter *f;
  char *buff, *tmp;

  f=owl_global_get_filter(&g, name);
  if (!f) {
    owl_function_error("There is no filter named %s", name);
    return;
  }
  tmp = owl_filter_print(f);
  buff = owl_sprintf("%s: %s", owl_filter_get_name(f), tmp);
  owl_function_popless_text(buff);
  owl_free(buff);
  owl_free(tmp);
}

void owl_function_show_zpunts(void)
{
  const owl_filter *f;
  const owl_list *fl;
  char buff[5000];
  char *tmp;
  owl_fmtext fm;
  int i, j;

  owl_fmtext_init_null(&fm);

  fl=owl_global_get_puntlist(&g);
  j=owl_list_get_size(fl);
  owl_fmtext_append_bold(&fm, "Active zpunt filters:\n");

  for (i=0; i<j; i++) {
    f=owl_list_get_element(fl, i);
    snprintf(buff, sizeof(buff), "[% 2d] ", i+1);
    owl_fmtext_append_normal(&fm, buff);
    tmp = owl_filter_print(f);
    owl_fmtext_append_normal(&fm, tmp);
    owl_free(tmp);
  }
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

/* Create a filter for a class, instance if one doesn't exist.  If
 * instance is NULL then catch all messgaes in the class.  Returns the
 * name of the filter, which the caller must free.
 * If 'related' is nonzero, encompass unclasses and .d classes as well.
 */
char *owl_function_classinstfilt(const char *c, const char *i, int related) 
{
  owl_filter *f;
  char *argbuff, *filtname;
  char *tmpclass, *tmpinstance = NULL;
  char *class, *instance = NULL;

  if (related) {
    class = owl_util_baseclass(c);
    if (i) {
      instance = owl_util_baseclass(i);
    }
  } else {
    class = owl_strdup(c);
    if (i) {
      instance = owl_strdup(i);
    }
  }

  /* name for the filter */
  if (!instance) {
    filtname = owl_sprintf("%sclass-%s", related ? "related-" : "", class);
  } else {
    filtname = owl_sprintf("%sclass-%s-instance-%s", related ? "related-" : "", class, instance);
  }
  /* downcase it */
  {
    char *temp = g_utf8_strdown(filtname, -1);
    if (temp) {
      owl_free(filtname);
      filtname = temp;
    }
  }
  /* turn spaces, single quotes, and double quotes into dots */
  owl_text_tr(filtname, ' ', '.');
  owl_text_tr(filtname, '\'', '.');
  owl_text_tr(filtname, '"', '.');
  
  /* if it already exists then go with it.  This lets users override */
  if (owl_global_get_filter(&g, filtname)) {
    goto done;
  }

  /* create the new filter */
  tmpclass=owl_text_quote(class, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);
  owl_text_tr(tmpclass, ' ', '.');
  owl_text_tr(tmpclass, '\'', '.');
  owl_text_tr(tmpclass, '"', '.');
  if (instance) {
    tmpinstance=owl_text_quote(instance, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);
    owl_text_tr(tmpinstance, ' ', '.');
    owl_text_tr(tmpinstance, '\'', '.');
    owl_text_tr(tmpinstance, '"', '.');
  }

  argbuff = owl_sprintf(related ? "class ^(un)*%s(\\.d)*$" : "class ^%s$", tmpclass);
  if (tmpinstance) {
    char *tmp = argbuff;
    argbuff = owl_sprintf(related ? "%s and ( instance ^(un)*%s(\\.d)*$ )" : "%s and instance ^%s$", tmp, tmpinstance);
    owl_free(tmp);
  }
  owl_free(tmpclass);
  if (tmpinstance) owl_free(tmpinstance);

  f = owl_filter_new_fromstring(filtname, argbuff);

  /* add it to the global list */
  owl_global_add_filter(&g, f);

  owl_free(argbuff);
done:
  owl_free(class);
  if (instance) {
    owl_free(instance);
  }
  return(filtname);
}

/* Create a filter for personal zephyrs to or from the specified
 * zephyr user.  Includes login/logout notifications for the user.
 * The name of the filter will be 'user-<user>'.  If a filter already
 * exists with this name, no new filter will be created.  This allows
 * the configuration to override this function.  Returns the name of
 * the filter, which the caller must free.
 */
char *owl_function_zuserfilt(const char *user)
{
  owl_filter *f;
  char *argbuff, *longuser, *esclonguser, *shortuser, *filtname;

  /* stick the local realm on if it's not there */
  longuser=long_zuser(user);
  shortuser=short_zuser(user);

  /* name for the filter */
  filtname=owl_sprintf("user-%s", shortuser);

  /* if it already exists then go with it.  This lets users override */
  if (owl_global_get_filter(&g, filtname)) {
    return filtname;
  }

  /* create the new-internal filter */
  esclonguser = owl_text_quote(longuser, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);

  argbuff=owl_sprintf("( type ^zephyr$ and filter personal and "
      "( ( direction ^in$ and sender ^%1$s$ ) or ( direction ^out$ and "
      "recipient ^%1$s$ ) ) ) or ( ( class ^login$ ) and ( sender ^%1$s$ ) )",
      esclonguser);

  f = owl_filter_new_fromstring(filtname, argbuff);

  /* add it to the global list */
  owl_global_add_filter(&g, f);

  /* free stuff */
  owl_free(argbuff);
  owl_free(longuser);
  owl_free(esclonguser);
  owl_free(shortuser);

  return(filtname);
}

/* Create a filter for AIM IM messages to or from the specified
 * screenname.  The name of the filter will be 'aimuser-<user>'.  If a
 * filter already exists with this name, no new filter will be
 * created.  This allows the configuration to override this function.
 * Returns the name of the filter, which the caller must free.
 */
char *owl_function_aimuserfilt(const char *user)
{
  owl_filter *f;
  char *argbuff, *filtname;
  char *escuser;

  /* name for the filter */
  filtname=owl_sprintf("aimuser-%s", user);

  /* if it already exists then go with it.  This lets users override */
  if (owl_global_get_filter(&g, filtname)) {
    return(owl_strdup(filtname));
  }

  /* create the new-internal filter */
  escuser = owl_text_quote(user, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);

  argbuff = owl_sprintf(
      "( type ^aim$ and ( ( sender ^%1$s$ and recipient ^%2$s$ ) or "
      "( sender ^%2$s$ and recipient ^%1$s$ ) ) )",
      escuser, owl_global_get_aim_screenname_for_filters(&g));

  f = owl_filter_new_fromstring(filtname, argbuff);

  /* add it to the global list */
  owl_global_add_filter(&g, f);

  /* free stuff */
  owl_free(argbuff);
  owl_free(escuser);

  return(filtname);
}

char *owl_function_typefilt(const char *type)
{
  owl_filter *f;
  char *argbuff, *filtname, *esctype;

  /* name for the filter */
  filtname=owl_sprintf("type-%s", type);

  /* if it already exists then go with it.  This lets users override */
  if (owl_global_get_filter(&g, filtname)) {
    return filtname;
  }

  /* create the new-internal filter */
  esctype = owl_text_quote(type, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);

  argbuff = owl_sprintf("type ^%s$", esctype);

  f = owl_filter_new_fromstring(filtname, argbuff);

  /* add it to the global list */
  owl_global_add_filter(&g, f);

  /* free stuff */
  owl_free(argbuff);
  owl_free(esctype);

  return filtname;
}

/* If flag is 1, marks for deletion.  If flag is 0,
 * unmarks for deletion. */
void owl_function_delete_curview_msgs(int flag)
{
  const owl_view *v;
  int i, j;

  v=owl_global_get_current_view(&g);
  j=owl_view_get_size(v);
  for (i=0; i<j; i++) {
    if (flag == 1) {
      owl_message_mark_delete(owl_view_get_element(v, i));
    } else if (flag == 0) {
      owl_message_unmark_delete(owl_view_get_element(v, i));
    }
  }

  owl_function_makemsg("%i messages marked for %sdeletion", j, flag?"":"un");

  owl_mainwin_redisplay(owl_global_get_mainwin(&g));  
}

static char *owl_function_smartfilter_cc(const owl_message *m) {
  const char *ccs;
  char *filtname;
  char *text;
  owl_filter *f;

  ccs = owl_message_get_attribute_value(m, "zephyr_ccs");

  filtname = owl_sprintf("conversation-%s", ccs);
  owl_text_tr(filtname, ' ', '-');

  if (owl_global_get_filter(&g, filtname)) {
    return filtname;
  }

  text = owl_sprintf("type ^zephyr$ and filter personal and "
                     "zephyr_ccs ^%s%s%s$",
                     owl_getquoting(ccs), ccs, owl_getquoting(ccs));

  f = owl_filter_new_fromstring(filtname, text);

  owl_global_add_filter(&g, f);

  owl_free(text);

  return filtname;
}

/* Create a filter based on the current message.  Returns the name of
 * a filter or null.  The caller must free this name.
 *
 * if the curmsg is a personal zephyr return a filter name
 *    to the zephyr conversation with that user.
 * If the curmsg is a zephyr class message, instance foo, recip *,
 *    return a filter name to the class, inst.
 * If the curmsg is a zephyr class message and type==0 then 
 *    return a filter name for just the class.
 * If the curmsg is a zephyr class message and type==1 then 
 *    return a filter name for the class and instance.
 * If the curmsg is a personal AIM message returna  filter
 *    name to the AIM conversation with that user 
 */
char *owl_function_smartfilter(int type, int invert_related)
{
  const owl_view *v;
  const owl_message *m;
  char *zperson, *filtname=NULL;
  const char *argv[2];
  int related = owl_global_is_narrow_related(&g) ^ invert_related;

  v=owl_global_get_current_view(&g);
  m=owl_view_get_element(v, owl_global_get_curmsg(&g));

  if (!m || owl_view_get_size(v)==0) {
    owl_function_error("No message selected\n");
    return(NULL);
  }

  /* very simple handling of admin messages for now */
  if (owl_message_is_type_admin(m)) {
    return(owl_function_typefilt("admin"));
  }

  /* very simple handling of loopback messages for now */
  if (owl_message_is_type_loopback(m)) {
    return(owl_function_typefilt("loopback"));
  }

  /* aim messages */
  if (owl_message_is_type_aim(m)) {
    if (owl_message_is_direction_in(m)) {
      filtname=owl_function_aimuserfilt(owl_message_get_sender(m));
    } else if (owl_message_is_direction_out(m)) {
      filtname=owl_function_aimuserfilt(owl_message_get_recipient(m));
    }
    return(filtname);
  }

  /* narrow personal and login messages to the sender or recip as appropriate */
  if (owl_message_is_type_zephyr(m)) {
    if (owl_message_is_personal(m) || owl_message_is_loginout(m)) {
      if (owl_message_get_attribute_value(m, "zephyr_ccs") != NULL) {
        return owl_function_smartfilter_cc(m);
      }

      if (owl_message_is_direction_in(m)) {
        zperson=short_zuser(owl_message_get_sender(m));
      } else {
        zperson=short_zuser(owl_message_get_recipient(m));
      }
      filtname=owl_function_zuserfilt(zperson);
      owl_free(zperson);
      return(filtname);
    }

    /* narrow class MESSAGE, instance foo, recip * messages to class, inst */
    if (!strcasecmp(owl_message_get_class(m), "message")) {
      filtname=owl_function_classinstfilt(owl_message_get_class(m), owl_message_get_instance(m), related);
      return(filtname);
    }

    /* otherwise narrow to the class */
    if (type==0) {
      filtname=owl_function_classinstfilt(owl_message_get_class(m), NULL, related);
    } else if (type==1) {
      filtname=owl_function_classinstfilt(owl_message_get_class(m), owl_message_get_instance(m), related);
    }
    return(filtname);
  }

  /* pass it off to perl */
  argv[0] = type ? "1" : "0";
  argv[1] = related ? "1" : "0";
  return owl_perlconfig_message_call_method(m, "smartfilter", 2, argv);
}

void owl_function_smartzpunt(int type)
{
  /* Starts a zpunt command based on the current class,instance pair. 
   * If type=0, uses just class.  If type=1, uses instance as well. */
  const owl_view *v;
  const owl_message *m;
  const char *cmdprefix, *mclass, *minst;
  char *cmd;
  
  v=owl_global_get_current_view(&g);
  m=owl_view_get_element(v, owl_global_get_curmsg(&g));

  if (!m || owl_view_get_size(v)==0) {
    owl_function_error("No message selected\n");
    return;
  }

  /* for now we skip admin messages. */
  if (owl_message_is_type_admin(m)
      || owl_message_is_loginout(m)
      || !owl_message_is_type_zephyr(m)) {
    owl_function_error("smartzpunt doesn't support this message type.");
    return;
  }

  mclass = owl_message_get_class(m);
  minst = owl_message_get_instance(m);
  if (!mclass || !*mclass || *mclass==' '
      || (!strcasecmp(mclass, "message") && !strcasecmp(minst, "personal"))
      || (type && (!minst || !*minst|| *minst==' '))) {
    owl_function_error("smartzpunt can't safely do this for <%s,%s>",
			 mclass, minst);
  } else {
    cmdprefix = "start-command zpunt ";
    cmd = owl_malloc(strlen(cmdprefix)+strlen(mclass)+strlen(minst)+10);
    strcpy(cmd, cmdprefix);
    strcat(cmd, owl_getquoting(mclass));
    strcat(cmd, mclass);
    strcat(cmd, owl_getquoting(mclass));
    if (type) {
      strcat(cmd, " ");
      strcat(cmd, owl_getquoting(minst));
      strcat(cmd, minst);
      strcat(cmd, owl_getquoting(minst));
    } else {
      strcat(cmd, " *");
    }
    owl_function_command(cmd);
    owl_free(cmd);
  }
}

/* Set the color of the current view's filter to
 * be 'color'
 */
void owl_function_color_current_filter(const char *fgcolor, const char *bgcolor)
{
  const char *name;

  name=owl_view_get_filtname(owl_global_get_current_view(&g));
  owl_function_color_filter(name, fgcolor, bgcolor);
}

/* Set the color of the filter 'filter' to be 'color'.  If the color
 * name does not exist, return -1, if the filter does not exist or is
 * the "all" filter, return -2.  Return 0 on success
 */
int owl_function_color_filter(const char *filtname, const char *fgcolor, const char *bgcolor)
{
  owl_filter *f;

  f=owl_global_get_filter(&g, filtname);
  if (!f) {
    owl_function_error("Unknown filter");
    return(-2);
  }

  /* don't touch the all filter */
  if (!strcmp(filtname, "all")) {
    owl_function_error("You may not change the 'all' filter.");
    return(-2);
  }

  if (owl_util_string_to_color(fgcolor)==OWL_COLOR_INVALID) {
    owl_function_error("No color named '%s' avilable.", fgcolor);
    return(-1);
  }


  if (bgcolor != NULL) {
    if (owl_util_string_to_color(bgcolor)==OWL_COLOR_INVALID) {
      owl_function_error("No color named '%s' avilable.", bgcolor);
      return(-1);
    }
    owl_filter_set_bgcolor(f, owl_util_string_to_color(bgcolor));
  }
  owl_filter_set_fgcolor(f, owl_util_string_to_color(fgcolor));
  
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  return(0);
}

void owl_function_show_colors(void)
{
  owl_fmtext fm;
  int i; 
  
  owl_fmtext_init_null(&fm);
  owl_fmtext_append_normal(&fm, "default: ");
  owl_fmtext_append_normal_color(&fm, "default\n", OWL_COLOR_DEFAULT, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"red:      ");
  owl_fmtext_append_normal_color(&fm, "red\n", OWL_COLOR_RED, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"green:    ");
  owl_fmtext_append_normal_color(&fm, "green\n", OWL_COLOR_GREEN, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"yellow:   ");
  owl_fmtext_append_normal_color(&fm, "yellow\n", OWL_COLOR_YELLOW, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"blue:     ");
  owl_fmtext_append_normal_color(&fm, "blue\n", OWL_COLOR_BLUE, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"magenta:  ");
  owl_fmtext_append_normal_color(&fm, "magenta\n", OWL_COLOR_MAGENTA, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"cyan:     ");
  owl_fmtext_append_normal_color(&fm, "cyan\n", OWL_COLOR_CYAN, OWL_COLOR_DEFAULT);

  owl_fmtext_append_normal(&fm,"white:    ");
  owl_fmtext_append_normal_color(&fm, "white\n", OWL_COLOR_WHITE, OWL_COLOR_DEFAULT);

  for(i = 8; i < COLORS; ++i) {
    char* str1 = owl_sprintf("%4i:     ",i);
    char* str2 = owl_sprintf("%i\n",i);
    owl_fmtext_append_normal(&fm,str1);
    owl_fmtext_append_normal_color(&fm, str2, i, OWL_COLOR_DEFAULT);
    owl_free(str1);
     owl_free(str2);
  }
  
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

/* add the given class, inst, recip to the punt list for filtering.
 *   if direction==0 then punt
 *   if direction==1 then unpunt
 */
void owl_function_zpunt(const char *class, const char *inst, const char *recip, int direction)
{
  char *puntexpr, *classexpr, *instexpr, *recipexpr;
  char *quoted;

  if (!strcmp(class, "*")) {
    classexpr = owl_sprintf("class .*");
  } else {
    quoted=owl_text_quote(class, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);
    owl_text_tr(quoted, ' ', '.');
    owl_text_tr(quoted, '\'', '.');
    owl_text_tr(quoted, '"', '.');
    classexpr = owl_sprintf("class ^(un)*%s(\\.d)*$", quoted);
    owl_free(quoted);
  }
  if (!strcmp(inst, "*")) {
    instexpr = owl_sprintf(" and instance .*");
  } else {
    quoted=owl_text_quote(inst, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);
    owl_text_tr(quoted, ' ', '.');
    owl_text_tr(quoted, '\'', '.');
    owl_text_tr(quoted, '"', '.');
    instexpr = owl_sprintf(" and instance ^(un)*%s(\\.d)*$", quoted);
    owl_free(quoted);
  }
  if (!strcmp(recip, "*")) {
    recipexpr = owl_sprintf("");
  } else {
    if(!strcmp(recip, "%me%")) {
      recip = owl_zephyr_get_sender();
    }
    quoted=owl_text_quote(recip, OWL_REGEX_QUOTECHARS, OWL_REGEX_QUOTEWITH);
    owl_text_tr(quoted, ' ', '.');
    owl_text_tr(quoted, '\'', '.');
    owl_text_tr(quoted, '"', '.');
    recipexpr = owl_sprintf(" and recipient ^%s$", quoted);
    owl_free(quoted);
  }

  puntexpr = owl_sprintf("%s %s %s", classexpr, instexpr, recipexpr);
  owl_function_punt(puntexpr, direction);
  owl_free(puntexpr);
  owl_free(classexpr);
  owl_free(instexpr);
  owl_free(recipexpr);
}

void owl_function_punt(const char *filter, int direction)
{
  owl_filter *f;
  owl_list *fl;
  int i, j;
  fl=owl_global_get_puntlist(&g);

  /* first, create the filter */
  owl_function_debugmsg("About to filter %s", filter);
  f = owl_filter_new_fromstring("punt-filter", filter);
  if (f == NULL) {
    owl_function_error("Error creating filter for zpunt");
    return;
  }

  /* Check for an identical filter */
  j=owl_list_get_size(fl);
  for (i=0; i<j; i++) {
    if (owl_filter_equiv(f, owl_list_get_element(fl, i))) {
      owl_function_debugmsg("found an equivalent punt filter");
      /* if we're punting, then just silently bow out on this duplicate */
      if (direction==0) {
	owl_filter_delete(f);
	return;
      }

      /* if we're unpunting, then remove this filter from the puntlist */
      if (direction==1) {
	owl_filter_delete(owl_list_get_element(fl, i));
	owl_list_remove_element(fl, i);
	owl_filter_delete(f);
	return;
      }
    }
  }

  owl_function_debugmsg("punting");
  /* If we're punting, add the filter to the global punt list */
  if (direction==0) {
    owl_list_append_element(fl, f);
  }
}

void owl_function_show_keymaps(void)
{
  owl_list l;
  owl_fmtext fm;
  const owl_keymap *km;
  const owl_keyhandler *kh;
  int i, numkm;
  const char *kmname;

  kh = owl_global_get_keyhandler(&g);
  owl_fmtext_init_null(&fm);
  owl_fmtext_append_bold(&fm, "Keymaps:   ");
  owl_fmtext_append_normal(&fm, "(use 'show keymap <name>' for details)\n");
  owl_keyhandler_get_keymap_names(kh, &l);
  owl_fmtext_append_list(&fm, &l, "\n", owl_function_keymap_summary);
  owl_fmtext_append_normal(&fm, "\n");

  numkm = owl_list_get_size(&l);
  for (i=0; i<numkm; i++) {
    kmname = owl_list_get_element(&l, i);
    km = owl_keyhandler_get_keymap(kh, kmname);
    owl_fmtext_append_bold(&fm, "\n\n----------------------------------------------------------------------------------------------------\n\n");
    owl_keymap_get_details(km, &fm, 0);
  }
  owl_fmtext_append_normal(&fm, "\n");
  
  owl_function_popless_fmtext(&fm);
  owl_keyhandler_keymap_namelist_cleanup(&l);
  owl_fmtext_cleanup(&fm);
}

char *owl_function_keymap_summary(const char *name)
{
  const owl_keymap *km 
    = owl_keyhandler_get_keymap(owl_global_get_keyhandler(&g), name);
  if (km) return owl_keymap_summary(km);
  else return(NULL);
}

/* TODO: implement for real */
void owl_function_show_keymap(const char *name)
{
  owl_fmtext fm;
  const owl_keymap *km;

  owl_fmtext_init_null(&fm);
  km = owl_keyhandler_get_keymap(owl_global_get_keyhandler(&g), name);
  if (km) {
    owl_keymap_get_details(km, &fm, 1);
  } else {
    owl_fmtext_append_normal(&fm, "No such keymap...\n");
  }  
  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

void owl_function_help_for_command(const char *cmdname)
{
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);
  owl_cmd_get_help(owl_global_get_cmddict(&g), cmdname, &fm);
  owl_function_popless_fmtext(&fm);  
  owl_fmtext_cleanup(&fm);
}

void owl_function_search_start(const char *string, int direction)
{
  /* direction is OWL_DIRECTION_DOWNWARDS or OWL_DIRECTION_UPWARDS or
   * OWL_DIRECTION_NONE */
  owl_regex re;

  if (string && owl_regex_create_quoted(&re, string) == 0) {
    owl_global_set_search_re(&g, &re);
    owl_regex_cleanup(&re);
  } else {
    owl_global_set_search_re(&g, NULL);
  }

  if (direction == OWL_DIRECTION_NONE)
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  else
    owl_function_search_helper(0, direction);
}

void owl_function_search_continue(int direction)
{
  /* direction is OWL_DIRECTION_DOWNWARDS or OWL_DIRECTION_UPWARDS */
  owl_function_search_helper(1, direction);
}

void owl_function_search_helper(int mode, int direction)
{
  /* move to a message that contains the string.  If direction is
   * OWL_DIRECTION_DOWNWARDS then search fowards, if direction is
   * OWL_DIRECTION_UPWARDS then search backwards.
   *
   * If mode==0 then it will stay on the current message if it
   * contains the string.
   */

  const owl_view *v;
  int viewsize, i, curmsg, start;
  owl_message *m;

  v=owl_global_get_current_view(&g);
  viewsize=owl_view_get_size(v);
  curmsg=owl_global_get_curmsg(&g);
  
  if (viewsize==0) {
    owl_function_error("No messages present");
    return;
  }

  if (mode==0) {
    start=curmsg;
  } else if (direction==OWL_DIRECTION_DOWNWARDS) {
    start=curmsg+1;
  } else {
    start=curmsg-1;
  }

  /* bounds check */
  if (start>=viewsize || start<0) {
    owl_function_error("No further matches found");
    return;
  }

  for (i=start; i<viewsize && i>=0;) {
    m=owl_view_get_element(v, i);
    if (owl_message_search(m, owl_global_get_search_re(&g))) {
      owl_global_set_curmsg(&g, i);
      owl_function_calculate_topmsg(direction);
      owl_mainwin_redisplay(owl_global_get_mainwin(&g));
      if (direction==OWL_DIRECTION_DOWNWARDS) {
	owl_global_set_direction_downwards(&g);
      } else {
	owl_global_set_direction_upwards(&g);
      }
      return;
    }
    if (direction==OWL_DIRECTION_DOWNWARDS) {
      i++;
    } else {
      i--;
    }
    owl_function_mask_sigint(NULL);
    if(owl_global_is_interrupted(&g)) {
      owl_global_unset_interrupted(&g);
      owl_function_unmask_sigint(NULL);
      owl_function_makemsg("Search interrupted!");
      owl_mainwin_redisplay(owl_global_get_mainwin(&g));
      return;
    }
    owl_function_unmask_sigint(NULL);
  }
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  owl_function_error("No matches found");
}

/* strips formatting from ztext and returns the unformatted text. 
 * caller is responsible for freeing. */
char *owl_function_ztext_stylestrip(const char *zt)
{
  owl_fmtext fm;
  char *plaintext;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_ztext(&fm, zt);
  plaintext = owl_fmtext_print_plain(&fm);
  owl_fmtext_cleanup(&fm);
  return(plaintext);
}

/* Popup a buddylisting.  If filename is NULL use the default .anyone */
void owl_function_buddylist(int aim, int zephyr, const char *filename)
{
  int i, j, idle;
  int interrupted = 0;
  owl_fmtext fm;
  const owl_buddylist *bl;
  const owl_buddy *b;
  char *timestr;
#ifdef HAVE_LIBZEPHYR
  int x;
  owl_list anyone;
  const char *user;
  char *tmp;
  ZLocations_t location[200];
  int numlocs, ret;
#endif

  owl_fmtext_init_null(&fm);

  /* AIM first */
  if (aim && owl_global_is_aimloggedin(&g)) {
    bl=owl_global_get_buddylist(&g);

    owl_fmtext_append_bold(&fm, "AIM users logged in:\n");
    /* we're assuming AIM for now */
    j=owl_buddylist_get_size(bl);
    for (i=0; i<j; i++) {
      b=owl_buddylist_get_buddy_n(bl, i);
      idle=owl_buddy_get_idle_time(b);
      if (idle!=0) {
	timestr=owl_util_minutes_to_timestr(idle);
      } else {
	timestr=owl_strdup("");
      }
      owl_fmtext_appendf_normal(&fm, "  %-20.20s %-12.12s\n", owl_buddy_get_name(b), timestr);
      owl_free(timestr);
    }
  }

#ifdef HAVE_LIBZEPHYR
  if (zephyr) {
    if(!owl_global_is_havezephyr(&g)) {
      owl_function_error("Zephyr currently not available.");
    } else {
      owl_fmtext_append_bold(&fm, "Zephyr users logged in:\n");
      owl_list_create(&anyone);
      ret=owl_zephyr_get_anyone_list(&anyone, filename);
      if (ret) {
        if (errno == ENOENT) {
          owl_fmtext_append_normal(&fm, " You have not added any zephyr buddies.  Use the\n");
          owl_fmtext_append_normal(&fm, " command ':addbuddy zephyr ");
          owl_fmtext_append_bold(  &fm, "<username>");
          owl_fmtext_append_normal(&fm, "'.\n");
        } else {
          owl_fmtext_append_normal(&fm, " Could not read zephyr buddies from the .anyone file.\n");
        }
      } else {
        j=owl_list_get_size(&anyone);
        for (i=0; i<j; i++) {
          user=owl_list_get_element(&anyone, i);
          ret=ZLocateUser(zstr(user), &numlocs, ZAUTH);

          owl_function_mask_sigint(NULL);
          if(owl_global_is_interrupted(&g)) {
            interrupted = 1;
            owl_global_unset_interrupted(&g);
            owl_function_unmask_sigint(NULL);
            owl_function_makemsg("Interrupted!");
            break;
          }

          owl_function_unmask_sigint(NULL);

          if (ret!=ZERR_NONE) {
            owl_function_error("Error getting location for %s", user);
            continue;
          }

          numlocs=200;
          ret=ZGetLocations(location, &numlocs);
          if (ret==0) {
            for (x=0; x<numlocs; x++) {
              tmp=short_zuser(user);
              owl_fmtext_appendf_normal(&fm, "  %-10.10s %-24.24s %-12.12s  %20.20s\n",
                                        tmp,
                                        location[x].host,
                                        location[x].tty,
                                        location[x].time);
              owl_free(tmp);
            }
            if (numlocs>=200) {
              owl_fmtext_append_normal(&fm, "  Too many locations found for this user, truncating.\n");
            }
          }
        }
      }
      owl_list_cleanup(&anyone, owl_free);
    }
  }
#endif

  if (aim && zephyr) {
    if (owl_perlconfig_is_function("BarnOwl::Hooks::_get_blist")) {
      char * perlblist = owl_perlconfig_execute("BarnOwl::Hooks::_get_blist()");
      if (perlblist) {
        owl_fmtext_append_ztext(&fm, perlblist);
        owl_free(perlblist);
      }
    }
  }

  if(!interrupted) {
    owl_function_popless_fmtext(&fm);
  }
  owl_fmtext_cleanup(&fm);
}

/* Dump messages in the current view to the file 'filename'. */
void owl_function_dump(const char *filename) 
{
  int i, j;
  owl_message *m;
  const owl_view *v;
  FILE *file;
  char *plaintext;

  v=owl_global_get_current_view(&g);

  /* in the future make it ask yes/no */
  /*
  ret=stat(filename, &sbuf);
  if (!ret) {
    ret=owl_function_askyesno("File exists, continue? [Y/n]");
    if (!ret) return;
  }
  */

  file=fopen(filename, "w");
  if (!file) {
    owl_function_error("Error opening file");
    return;
  }

  j=owl_view_get_size(v);
  for (i=0; i<j; i++) {
    m=owl_view_get_element(v, i);
    plaintext = owl_strip_format_chars(owl_message_get_text(m));
    if (plaintext) {
      fputs(plaintext, file);
      owl_free(plaintext);
    }
  }
  fclose(file);
  owl_function_makemsg("Messages dumped to %s", filename);
}

void owl_function_do_newmsgproc(void)
{
  if (owl_global_get_newmsgproc(&g) && strcmp(owl_global_get_newmsgproc(&g), "")) {
    /* if there's a process out there, we need to check on it */
    if (owl_global_get_newmsgproc_pid(&g)) {
      owl_function_debugmsg("Checking on newmsgproc pid==%i", owl_global_get_newmsgproc_pid(&g));
      owl_function_debugmsg("Waitpid return is %i", waitpid(owl_global_get_newmsgproc_pid(&g), NULL, WNOHANG));
      waitpid(owl_global_get_newmsgproc_pid(&g), NULL, WNOHANG);
      if (waitpid(owl_global_get_newmsgproc_pid(&g), NULL, WNOHANG)==-1) {
	/* it exited */
	owl_global_set_newmsgproc_pid(&g, 0);
	owl_function_debugmsg("newmsgproc exited");
      } else {
	owl_function_debugmsg("newmsgproc did not exit");
      }
    }
    
    /* if it exited, fork & exec a new one */
    if (owl_global_get_newmsgproc_pid(&g)==0) {
      pid_t i;
      int myargc;
      i=fork();
      if (i) {
	/* parent set the child's pid */
	owl_global_set_newmsgproc_pid(&g, i);
	owl_function_debugmsg("I'm the parent and I started a new newmsgproc with pid %i", i);
      } else {
	/* child exec's the program */
	char **parsed;
	parsed=owl_parseline(owl_global_get_newmsgproc(&g), &myargc);
	if (myargc < 0) {
	  owl_function_debugmsg("Could not parse newmsgproc '%s': unbalanced quotes?", owl_global_get_newmsgproc(&g));
	}
	if (myargc <= 0) {
	  _exit(127);
	}
	parsed=owl_realloc(parsed, sizeof(*parsed) * (myargc+1));
	parsed[myargc] = NULL;
	
	owl_function_debugmsg("About to exec \"%s\" with %d arguments", parsed[0], myargc);
	
	execvp(parsed[0], parsed);
	
	
	/* was there an error exec'ing? */
	owl_function_debugmsg("Cannot run newmsgproc '%s': cannot exec '%s': %s", 
			      owl_global_get_newmsgproc(&g), parsed[0], strerror(errno));
	_exit(127);
      }
    }
  }
}

/* print the xterm escape sequence to raise the window */
void owl_function_xterm_raise(void)
{
  printf("\033[5t");
}

/* print the xterm escape sequence to deiconify the window */
void owl_function_xterm_deiconify(void)
{
  printf("\033[1t");
}

/* Add the specified command to the startup file.  Eventually this
 * should be clever, and rewriting settings that will obviosly
 * override earlier settings with 'set' 'bindkey' and 'alias'
 * commands.  For now though we just remove any line that would
 * duplicate this one and then append this line to the end of
 * startupfile.
 */
void owl_function_addstartup(const char *buff)
{
  FILE *file;
  const char *filename;

  filename=owl_global_get_startupfile(&g);

  /* delete earlier copies */
  owl_util_file_deleteline(filename, buff, 1);

  file=fopen(filename, "a");
  if (!file) {
    owl_function_error("Error opening startupfile for new command");
    return;
  }

  /* add this line */
  fprintf(file, "%s\n", buff);

  fclose(file);
}

/* Remove the specified command from the startup file. */
void owl_function_delstartup(const char *buff)
{
  const char *filename;
  filename=owl_global_get_startupfile(&g);
  owl_util_file_deleteline(filename, buff, 1);
}

/* Execute owl commands from the given filename.  If the filename
 * is NULL, use the default owl startup commands file.
 */
void owl_function_source(const char *filename)
{
  char *path;
  FILE *file;
  char *s = NULL;
  int fail_silent = 0;

  if (!filename) {
    fail_silent = 1;
    path = owl_strdup(owl_global_get_startupfile(&g));
  } else {
    path = owl_util_makepath(filename);
  }
  file = fopen(path, "r");
  owl_free(path);
  if (!file) {
    if (!fail_silent) {
      owl_function_error("Error opening file: %s", filename);
    }
    return;
  }
  while (owl_getline_chomp(&s, file)) {
    if (s[0] == '\0' || s[0] == '#')
      continue;
    owl_function_command(s);
  }

  owl_free(s);
  fclose(file);
}

void owl_function_change_style(owl_view *v, const char *stylename)
{
  const owl_style *s;

  s=owl_global_get_style_by_name(&g, stylename);
  if (!s) {
    owl_function_error("No style named %s", stylename);
    return;
  }
  owl_view_set_style(v, s);
  owl_messagelist_invalidate_formats(owl_global_get_msglist(&g));
  owl_function_calculate_topmsg(OWL_DIRECTION_DOWNWARDS);
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

void owl_function_toggleoneline(void)
{
  owl_view *v;
  const owl_style *s;

  v=owl_global_get_current_view(&g);
  s=owl_view_get_style(v);

  if (!owl_style_matches_name(s, "oneline")) {
    owl_function_change_style(v, "oneline");
  } else {
    owl_function_change_style(v, owl_global_get_default_style(&g));
  }

  owl_messagelist_invalidate_formats(owl_global_get_msglist(&g));
  owl_function_calculate_topmsg(OWL_DIRECTION_DOWNWARDS);
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
}

void owl_function_error(const char *fmt, ...)
{
  static int in_error = 0;
  va_list ap;
  char *buff;
  const char *nl;

  if (++in_error > 2) {
    /* More than two nested errors, bail immediately. */
    in_error--;
    return;
  }

  va_start(ap, fmt);
  buff = g_strdup_vprintf(fmt, ap);
  va_end(ap);

  owl_function_debugmsg("ERROR: %s", buff);
  owl_function_log_err(buff);

  nl = strchr(buff, '\n');

  /*
    Showing admin messages triggers a lot of code. If we have a
    recursive error call, that's the most likely candidate, so
    suppress the call in that case, to try to avoid infinite looping.
  */

  if(nl && *(nl + 1) && in_error == 1) {
    /* Multiline error */
    owl_function_adminmsg("ERROR", buff);
  } else {
    owl_function_makemsg("[Error] %s", buff);
  }

  owl_free(buff);

  in_error--;
}

void owl_function_log_err(const char *string)
{
  char *date;
  time_t now;
  char *buff;

  now=time(NULL);
  date=owl_strdup(ctime(&now));
  date[strlen(date)-1]='\0';

  buff = owl_sprintf("%s %s", date, string);

  owl_errqueue_append_err(owl_global_get_errqueue(&g), buff);

  owl_free(buff);
  owl_free(date);
}

void owl_function_showerrs(void)
{
  owl_fmtext fm;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_normal(&fm, "Errors:\n\n");
  owl_errqueue_to_fmtext(owl_global_get_errqueue(&g), &fm);
  owl_function_popless_fmtext(&fm);
}

void owl_function_makemsg(const char *fmt, ...)
{
  va_list ap;
  char *str;

  va_start(ap, fmt);
  str = g_strdup_vprintf(fmt, ap);
  va_end(ap);

  owl_function_debugmsg("makemsg: %s", str);
  owl_msgwin_set_text(&g.msgwin, str);
}

/* get locations for everyone in .anyone.  If 'notify' is '1' then
 * send a pseudo login or logout message for everyone not in sync with
 * the global zephyr buddy list.  The list is updated regardless of
 * the status of 'notify'.
 */
void owl_function_zephyr_buddy_check(int notify)
{
#ifdef HAVE_LIBZEPHYR
  int i, j;
  owl_list anyone;
  owl_zbuddylist *zbl;
  GList **zaldlist;
  GList *zaldptr;
  ZAsyncLocateData_t *zald;
  const char *user;

  if (!owl_global_is_havezephyr(&g)) return;
  owl_global_set_pseudologin_notify(&g, notify);
  zbl = owl_global_get_zephyr_buddylist(&g);
  zaldlist = owl_global_get_zaldlist(&g);

  /* Clear the existing ZALDs first. */
  zaldptr = g_list_first(*zaldlist);
  while (zaldptr) {
    ZFreeALD(zaldptr->data);
    owl_free(zaldptr->data);
    zaldptr = g_list_next(zaldptr);
  }
  g_list_free(*zaldlist);
  *zaldlist = NULL;

  owl_list_create(&anyone);
  owl_zephyr_get_anyone_list(&anyone, NULL);
  j = owl_list_get_size(&anyone);
  for (i = 0; i < j; i++) {
    user = owl_list_get_element(&anyone, i);
    zald = owl_malloc(sizeof(ZAsyncLocateData_t));
    if (ZRequestLocations(zstr(user), zald, UNACKED, ZAUTH) == ZERR_NONE) {
      *zaldlist = g_list_append(*zaldlist, zald);
    } else {
      owl_free(zald);
    }
  }

  owl_list_cleanup(&anyone, owl_free);
#endif
}

void owl_function_aimsearch_results(const char *email, owl_list *namelist)
{
  owl_fmtext fm;
  int i, j;

  owl_fmtext_init_null(&fm);
  owl_fmtext_append_normal(&fm, "AIM screennames associated with ");
  owl_fmtext_append_normal(&fm, email);
  owl_fmtext_append_normal(&fm, ":\n");

  j=owl_list_get_size(namelist);
  for (i=0; i<j; i++) {
    owl_fmtext_append_normal(&fm, "  ");
    owl_fmtext_append_normal(&fm, owl_list_get_element(namelist, i));
    owl_fmtext_append_normal(&fm, "\n");
  }

  owl_function_popless_fmtext(&fm);
  owl_fmtext_cleanup(&fm);
}

int owl_function_get_color_count(void)
{
     return COLORS;
}

void owl_function_mask_sigint(sigset_t *oldmask) {
  sigset_t intr;

  sigemptyset(&intr);
  sigaddset(&intr, SIGINT);
  sigprocmask(SIG_BLOCK, &intr, oldmask);
}

void owl_function_unmask_sigint(sigset_t *oldmask) {
  sigset_t intr;

  sigemptyset(&intr);
  sigaddset(&intr, SIGINT);
  sigprocmask(SIG_UNBLOCK, &intr, oldmask);
}

void _owl_function_mark_message(const owl_message *m)
{
  if (m) {
    owl_global_set_markedmsgid(&g, owl_message_get_id(m));
    owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  }
}

void owl_function_mark_message(void)
{
  const owl_message *m;
  const owl_view *v;

  v=owl_global_get_current_view(&g);

  /* bail if there's no current message */
  if (owl_view_get_size(v) < 1) {
    owl_function_error("No messages to mark");
    return;
  }

  /* mark the message */
  m=owl_view_get_element(v, owl_global_get_curmsg(&g));
  _owl_function_mark_message(m);
  owl_function_makemsg("Mark set");
}

void owl_function_swap_cur_marked(void)
{
  int marked_id;
  const owl_message *m;
  const owl_view *v;

  marked_id=owl_global_get_markedmsgid(&g);
  if (marked_id == -1) {
    owl_function_error("Mark not set.");
    return;
  }

  v=owl_global_get_current_view(&g);
  /* bail if there's no current message */
  if (owl_view_get_size(v) < 1) {
    return;
  }

  m=owl_view_get_element(v, owl_global_get_curmsg(&g));
  _owl_function_mark_message(m);
  owl_global_set_curmsg(&g, owl_view_get_nearest_to_msgid(v, marked_id));
  owl_function_calculate_topmsg(OWL_DIRECTION_NONE);
  owl_mainwin_redisplay(owl_global_get_mainwin(&g));
  owl_global_set_direction_downwards(&g);
}
