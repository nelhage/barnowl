#include "owl.h"

static const char fileIdent[] = "$Id$";

void owl_mainwin_init(owl_mainwin *mw)
{
  mw->curtruncated=0;
  mw->top = owl_view_iterator_new();
  mw->end = owl_view_iterator_new();
  mw->current = owl_view_iterator_new();
  mw->mark = -1;
  mw->cur_scroll = -1;
}

static void reframe_top(owl_mainwin *mw)
{
  owl_view_iterator_clone(mw->top, mw->current);
}

static void reframe_neartop(owl_mainwin *mw)
{
  owl_view_iterator *it;
  it = owl_view_iterator_free_later(owl_view_iterator_new());
  owl_view_iterator_clone(it, mw->current);
  owl_view_iterator_prev(it);
  if (owl_message_get_numlines(owl_view_iterator_get_message(it))
      <  mw->win.lines/2) {
    owl_view_iterator_clone(mw->top, it);
  }
}

static void reframe_center(owl_mainwin *mw)
{
  int lines;
  owl_view_iterator *it;
  it = owl_view_iterator_free_later(owl_view_iterator_new());
  owl_view_iterator_clone(it, mw->current);

  lines = 0;
  for (owl_view_iterator_prev(it);
       !owl_view_iterator_is_at_start(it);
       owl_view_iterator_prev(it)) {
    lines += owl_message_get_numlines(owl_view_iterator_get_message(it));
    if (lines > mw->win.lines/2) break;
  }
  if(owl_view_iterator_is_at_start(it))
    owl_view_iterator_next(it);
  owl_view_iterator_clone(mw->top, it);
}
  
static void reframe_paged(owl_mainwin *mw, int center_on_page)
{
  int lines;
  owl_view_iterator *it;

  it = owl_view_iterator_free_later(owl_view_iterator_new());
  
  /* If we're off the top of the screen, scroll up such that the
   * mw->current is near the botton of the screen. */
  if (owl_view_iterator_cmp(mw->current, mw->top) < 0) {
    lines = 0;
    owl_view_iterator_clone(mw->top, mw->current);
    for (owl_view_iterator_clone(it, mw->current);
         !owl_view_iterator_is_at_start(it);
         owl_view_iterator_prev(it)) {
      lines += owl_message_get_numlines(owl_view_iterator_get_message(it));
      if (lines > mw->win.lines) break;
      owl_view_iterator_clone(mw->top, it);
    }
    if (center_on_page) {
      reframe_center(mw);
    }
    return;
  }

  if(!owl_view_iterator_is_valid(mw->end))
    return;

  owl_view_iterator_clone(it, mw->end);
  if(mw->lasttruncated) {
    owl_view_iterator_prev(it);
  }

  if (owl_view_iterator_cmp(mw->current, it) >= 0) {
    if(center_on_page) {
      reframe_center(mw);
    } else {
      owl_view_iterator_clone(mw->top, mw->current);
    }
  }
}

static void reframe_normal(owl_mainwin *mw)
{
  int savey, lines, y;
  owl_view_iterator *it;
  it = owl_view_iterator_free_later(owl_view_iterator_new());

  if (!owl_view_iterator_is_valid(mw->current)) return;

  /* If we're off the top of the screen then center */
  if (owl_view_iterator_cmp(mw->current, mw->top) < 0) {
      owl_view_iterator_clone(mw->top, mw->current);
      reframe_center(mw);
  }

  /* Find number of lines from top to bottom of mw->current (store in savey) */
  savey = 0;
  for (owl_view_iterator_clone(it, mw->top);
       owl_view_iterator_cmp(it, mw->current) <= 0
         /* If we ever find we're off-screen, we can stop */
         && savey <= mw->win.lines
         && !owl_view_iterator_is_at_end(it);
       owl_view_iterator_next(it)) {
    savey += owl_message_get_numlines(owl_view_iterator_get_message(it));
  }

  /* If we're off the bottom of the screen, set the topmsg to mw->current
   * and scroll upwards */
  if (savey > mw->win.lines) {
    owl_view_iterator_clone(mw->top, mw->current);
    savey=owl_message_get_numlines(owl_view_iterator_get_message(mw->current));
  }

  /* If our bottom line is less than 1/4 down the screen then scroll up */
  if (savey < (mw->win.lines / 4)) {
    y=0;
    for (owl_view_iterator_clone(it, mw->current);
         !owl_view_iterator_is_at_start(it);
         owl_view_iterator_prev(it)) {
      lines = owl_message_get_numlines(owl_view_iterator_get_message(it));
      /* will we run the mw->current off the screen? */
      if ( lines+y >= mw->win.lines ) {
        owl_view_iterator_next(it);
        if(owl_view_iterator_cmp(it, mw->current) > 0)
          owl_view_iterator_clone(it, mw->current);
        break;
      }
      /* have saved 1/2 the screen space? */
      y += lines;
      if (y > (mw->win.lines / 2)) break;
    }
    owl_view_iterator_clone(mw->top, it);
  }
  /* If mw->current bottom line is more than 3/4 down the screen then scroll down */
  if (savey > ((mw->win.lines * 3)/4)) {
    y=0;
    /* count lines from the top until we can save 1/2 the screen size */
    for (owl_view_iterator_clone(it, mw->top);
         owl_view_iterator_cmp(it, mw->current) < 0;
         owl_view_iterator_next(it)) {
      y+=owl_message_get_numlines(owl_view_iterator_get_message(it));
      if (y > (mw->win.lines / 2)) break;
    }
    if (owl_view_iterator_cmp(it,mw->current)) {
      owl_view_iterator_next(it);
    }
    owl_view_iterator_clone(mw->top, it);
  }
}

static void owl_mainwin_reframe(owl_mainwin *mw)
{
  if(!owl_view_iterator_is_valid( mw->top)) {
    owl_view_iterator_clone(mw->top, mw->current);
  }
  if(!owl_view_iterator_same_view(mw->current, mw->top)) {
    if(owl_view_iterator_is_at_end(mw->top)) {
      owl_view_iterator_init_end(mw->top,
                                 mw->view);
    } else {
      int id = owl_message_get_id(owl_view_iterator_get_message(mw->top));
      owl_view_iterator_init_id(mw->top,
                                mw->view,
                                id);
    }
    owl_view_iterator_invalidate(mw->end);
  }

  switch (owl_global_get_scrollmode(&g)) {
  case OWL_SCROLLMODE_TOP:
    reframe_top(mw);
    break;
  case OWL_SCROLLMODE_NEARTOP:
    reframe_neartop(mw);
    break;
  case OWL_SCROLLMODE_CENTER:
    reframe_center(mw);
    break;
  case OWL_SCROLLMODE_PAGED:
    reframe_paged(mw, 0);
    break;
  case OWL_SCROLLMODE_PAGEDCENTER:
    reframe_paged(mw, 1);
    break;
  case OWL_SCROLLMODE_NORMAL:
  default:
    reframe_normal(mw);
  }
}

void owl_mainwin_redisplay(owl_mainwin *mw)
{
  owl_message *m;
  int p, q, lines, isfull;
  int x, y, savey, start;
  int fgcolor, bgcolor;
  int curid;
  owl_view_iterator *iter;
  owl_view *v;
  owl_list *filtlist;
  owl_filter *f;

  owl_mainwin_reframe(mw);

  iter = owl_view_iterator_free_later(owl_view_iterator_new());

  v=owl_global_get_current_view(&g);
  owl_fmtext_reset_colorpairs();

  if(owl_view_iterator_is_at_end(mw->current)
     && !owl_view_iterator_is_at_start(mw->current)) {
    owl_function_error("WARNING: curmsg is-at-end. Please report this bug to bug-barnowl@mit.edu");
    owl_view_iterator_prev(mw->current);
  }

  if (v==NULL) {
    owl_function_debugmsg("Hit a null window in owl_mainwin_redisplay.");
    return;
  }

  werase(mw->win.win);

  /* if there are no messages, * just draw a blank screen */
  if (owl_view_is_empty(v)) {
    mw->curtruncated=0;
    owl_view_iterator_invalidate(mw->end);
    wnoutrefresh(mw->win.win);
    owl_global_set_needrefresh(&g);
    return;
  }

  /* write the messages out */
  isfull=0;
  mw->curtruncated=0;
  mw->lasttruncated=0;

  if(owl_view_iterator_get_message(mw->current)) {
    curid = owl_message_get_id(owl_view_iterator_get_message(mw->current));
  } else {
    curid = -1;
  }

  for(owl_view_iterator_clone(iter, mw->top);
      !owl_view_iterator_is_at_end(iter);
      owl_view_iterator_next(iter)) {
    if (isfull) break;
    m = owl_view_iterator_get_message(iter);
    int iscurrent = owl_message_get_id(m) == curid;

    /* hold on to y in case this is the current message or deleted */
    getyx(mw->win.win, y, x);
    savey=y;

    /* if it's the current message, account for a vert_offset */
    if (iscurrent) {
      start=mw->cur_scroll;
      lines=owl_message_get_numlines(m)-start;
    } else {
      start=0;
      lines=owl_message_get_numlines(m);
    }

    /* if we match filters set the color */
    fgcolor=OWL_COLOR_DEFAULT;
    bgcolor=OWL_COLOR_DEFAULT;
    filtlist=owl_global_get_filterlist(&g);
    q=owl_list_get_size(filtlist);
    for (p=0; p<q; p++) {
      f=owl_list_get_element(filtlist, p);
      if ((owl_filter_get_fgcolor(f)!=OWL_COLOR_DEFAULT) ||
          (owl_filter_get_bgcolor(f)!=OWL_COLOR_DEFAULT)) {
        if (owl_filter_message_match(f, m)) {
          if (owl_filter_get_fgcolor(f)!=OWL_COLOR_DEFAULT) fgcolor=owl_filter_get_fgcolor(f);
          if (owl_filter_get_bgcolor(f)!=OWL_COLOR_DEFAULT) bgcolor=owl_filter_get_bgcolor(f);
	}
      }
    }

    /* if we'll fill the screen print a partial message */
    if (y+lines > mw->win.lines) {
      mw->lasttruncated=1;
      if(iscurrent) mw->curtruncated = 1;
    }
    if (y+lines > mw->win.lines-1) {
      isfull=1;
      owl_message_curs_waddstr(m, mw->win.win,
			       start,
			       start+mw->win.lines-y,
			       owl_global_get_rightshift(&g),
			       owl_global_get_cols(&g)+owl_global_get_rightshift(&g)-1,
			       fgcolor, bgcolor);
    } else {
      /* otherwise print the whole thing */
      owl_message_curs_waddstr(m, mw->win.win,
			       start,
			       start+mw->win.lines,
			       owl_global_get_rightshift(&g),
			       owl_global_get_cols(&g)+owl_global_get_rightshift(&g)-1,
			       fgcolor, bgcolor);
    }


    /* is it the current message and/or deleted? */
    getyx(mw->win.win, y, x);
    wattrset(mw->win.win, A_NORMAL);
    if (owl_global_get_rightshift(&g)==0) {   /* this lame and should be fixed */
      if (iscurrent) {
        wmove(mw->win.win, savey, 0);
        wattron(mw->win.win, A_BOLD);
        if (mw->cur_scroll > 0) {
          waddstr(mw->win.win, "+");
        } else {
          waddstr(mw->win.win, "-");
        }
        if (!owl_message_is_delete(m)) {
          waddstr(mw->win.win, ">");
        } else {
          waddstr(mw->win.win, "D");
        }
        wmove(mw->win.win, y, x);
        wattroff(mw->win.win, A_BOLD);
      } else if (owl_message_is_delete(m)) {
        wmove(mw->win.win, savey, 0);
        waddstr(mw->win.win, " D");
        wmove(mw->win.win, y, x);
      }
    }
    wattroff(mw->win.win, A_BOLD);
  }

  /*  owl_view_iterator_prev(iter); */
  owl_view_iterator_clone(mw->end, iter);

  wnoutrefresh(mw->win.win);
  owl_global_set_needrefresh(&g);
}


int owl_mainwin_is_curmsg_truncated(owl_mainwin *mw)
{
  if (mw->curtruncated) return(1);
  return(0);
}

int owl_mainwin_is_last_msg_truncated(owl_mainwin *mw)
{
  if (mw->lasttruncated) return(1);
  return(0);
}
