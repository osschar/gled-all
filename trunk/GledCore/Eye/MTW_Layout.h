// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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
    GledViewNS::MemberInfo*	fMemberInfo;
    int				fW;
    Member(GledViewNS::MemberInfo* mi, int w=0) : fMemberInfo(mi), fW(w) {}
  };
  typedef list<Member>			lMember_t;
  typedef list<Member>::iterator	lMember_i;
  struct Class {
    GledViewNS::ClassInfo*	fClassInfo;
    lMember_t			fMembers;
    int				fFullW;
    Class(GledViewNS::ClassInfo* ci) : fClassInfo(ci), fFullW(0) {}
  };
  typedef list<Class>			lClass_t;
  typedef list<Class>::iterator		lClass_i;
  
  FTW_Nest*	mNest;		// X{G}

  lClass_t	mClasses;
  bool		bIsValid;	// X{G}

  Fl_Input*	wLaySpecs;	// X{G}

public:
  MTW_Layout(FTW_Nest* nest);
  void Parse() throw (string);
  int  CountSubViews(ZGlass* glass);

  Fl_Group* CreateLabelGroup();

#include "MTW_Layout.h7"
}; // endclass MTW_Layout

#endif
