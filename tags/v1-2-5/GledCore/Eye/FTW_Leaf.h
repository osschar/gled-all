// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Leaf_H
#define GledCore_FTW_Leaf_H

#include "OptoStructs.h"
#include "FTW.h"
class FTW_Nest; class MTW_ClassView; class MTW_Layout;

#include <FL/Fl_Pack.H>
class Fl_Box; class Fl_Button;

#define FL_DARK_RED fl_color_cube(2,0,0)

class FTW_Leaf : public Fl_Pack, public OptoStructs::A_View
{
  friend class FTW_Shell;
  friend class FTW_Nest;
  friend class FTW::Locator;

private:
  Int_t			mIter;

protected:
  FTW_Nest*		mNest;	 // X{g}
  FTW_Leaf*		mParent; // X{g}
  Int_t			mLevel;	 // X{gs}
  bool			bIsListMember;
  bool			bIsLinkDescendant;

  Fl_Pack*		wFrontPack;
  Fl_Box*		wIndentBox;
  Fl_Button*		wExpander;
  Fl_Button*		wListExpander;
  FTW::NameButton*	wName;
  Fl_Box*		wSepBox;
  Fl_Pack*		wAntPack;

  MTW_ClassView*	wCustomView;
  Fl_Box*		wTailBox;
  // Rnr Ctrl weedget

  void resize_weeds();
  virtual void label_namebox();
  virtual void label_weeds();
  void recolor_name();
  void modify_box_color(Fl_Color mod, bool on_p);

  void create_custom_view(MTW_Layout* layout);
  void show_custom_view();
  void hide_custom_view();
  void wipe_custom_view();

  void reverse_ants();

public:
  static FTW_Leaf* Construct(FTW_Nest* nest, FTW_Leaf* parent,
			     OptoStructs::ZGlassImg* img,
			     bool is_list_member, bool is_link_desc);
  FTW_Leaf(FTW_Nest* nest, FTW_Leaf* parent,
	   OptoStructs::ZGlassImg* img,
	   bool is_list_member, bool is_link_desc);
  virtual ~FTW_Leaf();

  virtual void AbsorbRay(Ray& ray);

  // General foonctionality, partially overridden in FTW_Branch.
  virtual void ExpandList() {}
  virtual void CollapseList() {}
  virtual void CopyListLeaves(list<FTW_Leaf*>& list) {}

  virtual void ExpandLink(FTW_Ant* ant);
  virtual void CollapseLink(FTW_Ant* ant);
  virtual void ExpandLinks();
  virtual void CollapseLinks();
  virtual void CollExp();
  float	       FractionOfExpandedAnts();

  virtual void ShowRecursively();
  virtual void HideRecursively();
  virtual void ShowLinks();
  virtual void HideLinks();
  virtual void ShowListMembers() {}
  virtual void HideListMembers() {}

  bool IsList()       { return fImg->fIsList; }
  bool IsListMember() { return bIsListMember; }

  int  	   AntPos(FTW_Ant* a);
  FTW_Ant* AntAt(int i);
  FTW_Ant* LastAnt();

#include "FTW_Leaf.h7"
}; // endclass FTW_Leaf

#endif