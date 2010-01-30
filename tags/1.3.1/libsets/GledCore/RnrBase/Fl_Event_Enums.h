//
// "$Id$"
//
// Enumerations for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// M.Tadel, 20.6.2005: Stripped down verion of fltk event structures for
// forwarding of events into Gled-rendering infrastructure.

#ifndef Fl_Enumerations_H
#define Fl_Enumerations_H

enum Fl_Event {	// events
  FL_NO_EVENT		= 0,
  FL_PUSH		= 1,
  FL_RELEASE		= 2,
  FL_ENTER		= 3,
  FL_LEAVE		= 4,
  FL_DRAG		= 5,
  FL_FOCUS		= 6,
  FL_UNFOCUS		= 7,
  FL_KEYDOWN		= 8,
  FL_KEYUP		= 9,
  FL_CLOSE		= 10,
  FL_MOVE		= 11,
  FL_SHORTCUT		= 12,
  FL_DEACTIVATE		= 13,
  FL_ACTIVATE		= 14,
  FL_HIDE		= 15,
  FL_SHOW		= 16,
  FL_PASTE		= 17,
  FL_SELECTIONCLEAR	= 18,
  FL_MOUSEWHEEL		= 19,
  FL_DND_ENTER		= 20,
  FL_DND_DRAG		= 21,
  FL_DND_LEAVE		= 22,
  FL_DND_RELEASE	= 23
};
#define FL_KEYBOARD FL_KEYDOWN

// Fl::event_key() and Fl::get_key(n) (use ascii letters for all other keys):
#define FL_Button	0xfee8 // use Fl_Button+FL_*_MOUSE
#define FL_BackSpace	0xff08
#define FL_Tab		0xff09
#define FL_Enter	0xff0d
#define FL_Pause	0xff13
#define FL_Scroll_Lock	0xff14
#define FL_Escape	0xff1b
#define FL_Home		0xff50
#define FL_Left		0xff51
#define FL_Up		0xff52
#define FL_Right	0xff53
#define FL_Down		0xff54
#define FL_Page_Up	0xff55
#define FL_Page_Down	0xff56
#define FL_End		0xff57
#define FL_Print	0xff61
#define FL_Insert	0xff63
#define FL_Menu		0xff67 // the "menu/apps" key on XFree86
#define FL_Help		0xff68 // the 'help' key on Mac keyboards
#define FL_Num_Lock	0xff7f
#define FL_KP		0xff80 // use FL_KP+'x' for 'x' on numeric keypad
#define FL_KP_Enter	0xff8d // same as Fl_KP+'\r'
#define FL_KP_Last	0xffbd // use to range-check keypad
#define FL_F		0xffbd // use FL_F+n for function key n
#define FL_F_Last	0xffe0 // use to range-check function keys
#define FL_Shift_L	0xffe1
#define FL_Shift_R	0xffe2
#define FL_Control_L	0xffe3
#define FL_Control_R	0xffe4
#define FL_Caps_Lock	0xffe5
#define FL_Meta_L	0xffe7 // the left MSWindows key on XFree86
#define FL_Meta_R	0xffe8 // the right MSWindows key on XFree86
#define FL_Alt_L	0xffe9
#define FL_Alt_R	0xffea
#define FL_Delete	0xffff

// Fl::event_button():
#define FL_LEFT_MOUSE	1
#define FL_MIDDLE_MOUSE	2
#define FL_RIGHT_MOUSE	3

// Fl::event_state():
#define FL_SHIFT	0x00010000
#define FL_CAPS_LOCK	0x00020000
#define FL_CTRL		0x00040000
#define FL_ALT		0x00080000
#define FL_NUM_LOCK	0x00100000 // most X servers do this?
#define FL_META		0x00400000 // correct for XFree86
#define FL_SCROLL_LOCK	0x00800000 // correct for XFree86
#define FL_BUTTON1	0x01000000
#define FL_BUTTON2	0x02000000
#define FL_BUTTON3	0x04000000
#define FL_BUTTONS	0x7f000000 // All possible buttons
#define FL_BUTTON(n)	(0x00800000<<(n))

#ifdef __APPLE__
#  define FL_COMMAND	FL_META
#else
#  define FL_COMMAND	FL_CTRL
#endif // __APPLE__

#endif
