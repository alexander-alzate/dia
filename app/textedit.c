/* Dia -- an diagram creation/manipulation program
 * Copyright (C) 1998 Alexander Larsson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/** This file handles the display part of text edit stuff: Making text
 * edit start and stop at the right time and highlighting the edits.
 * lib/text.c and lib/focus.c handles internal things like who can have
 * focus and how to enter text.  app/disp_callbacks.c handles the actual
 * keystrokes.
 *
 * There's an invariant that all objects in the focus list must be selected.
 */

#include <config.h>

#include "object.h"
#include "focus.h"
#include "display.h"
#include "highlight.h"
#include "textedit.h"
#include "object_ops.h"
#include "text.h"

/** Move the text edit focus either backwards or forwards. */
Focus *
textedit_move_focus(DDisplay *ddisp, Focus *focus, gboolean forwards)
{
  if (focus != NULL) {
    /* Leak of focus highlight color here, but should it be handled
       by highlight or by us?
    */
    highlight_object_off(focus->obj, ddisp->diagram);
    object_add_updates(focus->obj, ddisp->diagram);
  }
  if (forwards)
    focus_next();
  else
    focus_previous();
  focus = active_focus();

  if (focus != NULL) {
    Color *focus_col = color_new_rgb(1.0, 1.0, 0.0);
    highlight_object(focus->obj, focus_col, ddisp->diagram);
    object_add_updates(focus->obj, ddisp->diagram);
  }
  diagram_flush(ddisp->diagram);
  return focus;
}

/** Call when something recieves an actual focus (not to be confused
 * with doing request_focus(), which merely puts one in the focus list).
 */
void
textedit_activate_focus(DDisplay *ddisp, Focus *focus, Point *clicked)
{
  if (active_focus()) {
    Focus *old_focus = active_focus();
    object_add_updates(old_focus->obj, ddisp->diagram);
    highlight_object_off(old_focus->obj, ddisp->diagram);
  }
  highlight_object(focus->obj, color_new_rgb(1.0, 1.0, 0.0), ddisp->diagram);
  if (clicked) {
      text_set_cursor((Text*)focus->user_data, clicked, ddisp->renderer);
  }
  object_add_updates(focus->obj, ddisp->diagram);
  give_focus(focus);
  diagram_flush(ddisp->diagram);
}

/** Call when an object is chosen for activation (e.g. due to creation).
 */
void
textedit_activate_object(DDisplay *ddisp, DiaObject *obj, Point *clicked)
{
  if (active_focus()) {
    Focus *focus = active_focus();
    highlight_object_off(focus->obj, ddisp->diagram);
    object_add_updates(focus->obj, ddisp->diagram);
  }
  if (give_focus_to_object(obj)) {
    highlight_object(obj, color_new_rgb(1.0, 1.0, 0.0), ddisp->diagram);
    if (clicked) {
      text_set_cursor((Text*)active_focus()->user_data, clicked, ddisp->renderer);
    }
    object_add_updates(obj, ddisp->diagram);
    diagram_flush(ddisp->diagram);
  }
}

/** Call when something causes the text focus to disappear.
 * Does not remove objects from the focus list, but removes the
 * focus highlight and stuff.
 * Calling remove_focus on the active object or remove_focus_all
 * implies deactivating the focus. */
void
textedit_deactivate_focus()
{
  Focus *focus = active_focus();
  if (focus != NULL) {
    highlight_object_off(focus->obj, ddisplay_active()->diagram);
    remove_focus();
  }
}

/** Call when something should be removed from the focus list */
void
textedit_remove_focus(DiaObject *obj, Diagram *diagram)
{
  if (remove_focus_object(obj)) {
    highlight_object_off(obj, ddisplay_active()->diagram);
  }
}

/** Call when the entire list of focusable texts gets reset. */
void
textedit_remove_focus_all(Diagram *diagram)
{
  Focus *focus = active_focus();
  if (focus != NULL) {
    highlight_object_off(focus->obj, diagram);
  }
  reset_foci();
}