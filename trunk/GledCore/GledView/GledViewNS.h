// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledViewNS_H
#define GledCore_GledViewNS_H

#include <Gled/GledNS.h>

class ZGlass;
class ZMIR;
class Eye;
class MTW_View;
class MTW_SubView;
class FTW_Ant;
class Fl_Widget;
class TMessage;

namespace GledViewNS {

  // Low level: weed info

  typedef Fl_Widget*	(*WeedCreator_foo)    (MTW_SubView*);
  typedef void		(*WeedCallback_foo)   (Fl_Widget*, MTW_SubView*);
  typedef void		(*WeedUpdate_foo)     (Fl_Widget*, MTW_SubView*);
  typedef void		(*CtxCallCreator_foo) (TBuffer*);

  struct MethodInfo {};		// Null so far

  struct DataMemberInfo {};	// Null so far

  struct LinkMemberInfo {};

  struct WeedInfo : public GledNS::InfoBase {
    bool		bIsLinkWeed;
    Int_t		fWidth;
    Int_t		fHeight;
    bool		bLabel;
    bool		bLabelInside;
    bool		bCanResize;
    bool		bJoinNext;
    WeedCreator_foo	fooWCreator;
    WeedCallback_foo	fooWCallback;
    WeedUpdate_foo	fooWUpdate;

    WeedInfo(const TString& s) : InfoBase(s) {}
  };

  typedef list<WeedInfo*>		lpWeedInfo_t;
  typedef list<WeedInfo*>::iterator	lpWeedInfo_i;

  // Intermediate level: ClassInfo, RnrCreator and SubView info

  typedef MTW_SubView*	(*SubViewCreator_foo)(GledNS::ClassInfo* ci, MTW_View* v, ZGlass* g);

  struct ClassInfo {
    SubViewCreator_foo		fooSVCreator;
    lpWeedInfo_t		fWeedList;

    //----------------------------------------------------------------

    WeedInfo*		FindWeedInfo(const TString& name, bool recurse=false,
				     GledNS::ClassInfo* true_class=0);
  };

  // High level: per LibSet information

  struct LibSetInfo {};	// Now empty ... rnr support in base.

  /**************************************************************************/
  // Variables
  /**************************************************************************/


  /**************************************************************************/
  // Functions
  /**************************************************************************/

  Int_t LoadSoSet(const TString& lib_set);
  Int_t InitSoSet(const TString& lib_set);
  void	BootstrapViewSet(LID_t lid, const TString& libset);
  void  BootstrapClassInfo(ClassInfo* c_info);

  TString FabricateViewLibName(const TString& libset);
  TString FabricateViewInitFoo(const TString& libset);
  TString FabricateViewUserInitFoo(const TString& libset);

  // GUI magick

  extern int no_symbol_label;
  extern int menubar_box;

} // GledViewNS

#endif
