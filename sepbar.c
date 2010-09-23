#include "owl.h"

static void sepbar_redraw(owl_window *w, WINDOW *sepwin, void *user_data);

void owl_sepbar_init(owl_window *w)
{
  g_signal_connect(w, "redraw", G_CALLBACK(sepbar_redraw), NULL);
  /* TODO: handle dirtiness in the sepbar */
  owl_window_dirty(w);
}

static void sepbar_redraw(owl_window *w, WINDOW *sepwin, void *user_data)
{
  const owl_messagelist *ml;
  const owl_view *v;
  owl_view_iterator *iter;
  int x, y;
  const char *foo, *appendtosepbar;

  iter = owl_view_iterator_delete_later(owl_view_iterator_new());

  ml=owl_global_get_msglist(&g);
  v=owl_global_get_current_view(&g);

  werase(sepwin);
  wattron(sepwin, A_REVERSE);
  if (owl_global_is_fancylines(&g)) {
    whline(sepwin, ACS_HLINE, owl_global_get_cols(&g));
  } else {
    whline(sepwin, '-', owl_global_get_cols(&g));
  }

  if (owl_global_is_sepbar_disable(&g)) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, owl_global_get_cols(&g)-1);
    return;
  }

  wmove(sepwin, 0, 2);

  if (owl_messagelist_get_size(ml) == 0)
    waddstr(sepwin, " (-) ");
  else
    wprintw(sepwin, " (%i) ", owl_messagelist_get_size(ml));

  foo=owl_view_get_filtname(v);
  if (strcmp(foo, owl_global_get_view_home(&g)))
      wattroff(sepwin, A_REVERSE);
  wprintw(sepwin, " %s ", owl_view_get_filtname(v));
  if (strcmp(foo, owl_global_get_view_home(&g)))
      wattron(sepwin, A_REVERSE);

  if (owl_mainwin_is_curmsg_truncated(owl_global_get_mainwin(&g))) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    wattron(sepwin, A_BOLD);
    waddstr(sepwin, " <truncated> ");
    wattroff(sepwin, A_BOLD);
  }

  owl_view_iterator_clone(iter, owl_mainwin_get_last_msg(owl_global_get_mainwin(&g)));
  if (owl_view_iterator_is_valid(iter)
      && !owl_view_iterator_is_at_end(iter)) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    wattron(sepwin, A_BOLD);
    waddstr(sepwin, " <more> ");
    wattroff(sepwin, A_BOLD);
  }

  if (owl_global_get_rightshift(&g)>0) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    wprintw(sepwin, " right: %i ", owl_global_get_rightshift(&g));
  }

  if (owl_global_is_zaway(&g) || owl_global_is_aaway(&g)) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    wattron(sepwin, A_BOLD);
    wattroff(sepwin, A_REVERSE);
    if (owl_global_is_zaway(&g) && owl_global_is_aaway(&g)) {
      waddstr(sepwin, " AWAY ");
    } else if (owl_global_is_zaway(&g)) {
      waddstr(sepwin, " Z-AWAY ");
    } else if (owl_global_is_aaway(&g)) {
      waddstr(sepwin, " A-AWAY ");
    }
    wattron(sepwin, A_REVERSE);
    wattroff(sepwin, A_BOLD);
  }

  if (owl_global_get_curmsg_vert_offset(&g)) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    wattron(sepwin, A_BOLD);
    wattroff(sepwin, A_REVERSE);
    waddstr(sepwin, " SCROLL ");
    wattron(sepwin, A_REVERSE);
    wattroff(sepwin, A_BOLD);
  }

  appendtosepbar = owl_global_get_appendtosepbar(&g);
  if (appendtosepbar && *appendtosepbar) {
    getyx(sepwin, y, x);
    wmove(sepwin, y, x+2);
    waddstr(sepwin, " ");
    waddstr(sepwin, owl_global_get_appendtosepbar(&g));
    waddstr(sepwin, " ");
  }

  getyx(sepwin, y, x);
  wmove(sepwin, y, owl_global_get_cols(&g)-1);

  wattroff(sepwin, A_BOLD);
  wattroff(sepwin, A_REVERSE);
}
