// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_MTW_Layout_H
#define Gled_MTW_Layout_H

#include <Gled/GledTypes.h>
#include <GledView/GledViewNS.h>
#include <FL/Fl_SWM.H>
#include <FL/Fl_Window.H>

class FTW_Nest;

class MTW_Layout : public Fl_Window, public Fl_SWM_Client {
  friend class MTW_View;

protected:
  struct Member {
    GledViewNS::WeedInfo*	fWeedInfo;
    int				fW;
    Member(GledViewNS::WeedInfo* wi, int w=0) : fWeedInfo(wi), fW(w) {}
  };
  typedef list<Member>			lMember_t;
  typedef list<Member>::iterator	lMember_i;
  struct Class {
    GledNS::ClassInfo*		fClassInfo;
    lMember_t			fMembers;
    int				fFullW;
    Class(GledNS::ClassInfo* ci) : fClassInfo(ci), fFullW(0) {}
  };
  typedef list<Class>			lClass_t;
  typedef list<Class>::iterator		lClass_i;
  
  FTW_Nest*	mNest;		// X{g}

  lClass_t	mClasses;
  bool		bIsValid;	// X{g}

  Fl_Input*	wLaySpecs;	// X{g}

public:
  MTW_Layout(FTW_Nest* nest);
  void Parse(int cell_w=0) throw (string);
  int  CountSubViews(ZGlass* glass);

  Fl_Group* CreateLabelGroup();

#include "MTW_Layout.h7"
}; // endclass MTW_Layout

#endif
