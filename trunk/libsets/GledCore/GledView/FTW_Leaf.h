// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Leaf_H
#define GledCore_FTW_Leaf_H

#include <Eye/OptoStructs.h>
#include <Glasses/AList.h>  // Need AList::ElRep
#include "FTW.h"
class FTW_Nest; class MTW_ClassView; class MTW_Layout;

#include <FL/Fl_Pack.H>
class Fl_Box; class Fl_Button;

#define FL_DARK_RED fl_color_cube(2,0,0)

class FTW_Leaf : public Fl_Pack, public OptoStructs::A_View
{
  friend class FTW_Shell;
  friend class FTW_Nest;
  friend class FTW_Branch;      // To set AList::ElRep stuff.
  friend class FTW::Locator;

private:
  Int_t			mIter;

protected:
  FTW_Nest*		mNest;	    // X{g}
  FTW_Leaf*		mParent;    // X{g}
  Int_t			mLevel;	    // X{gs}
  bool			bIsListMember;
  bool			bIsLinkDescendant;
  Int_t			mListId;    // X{gs}
  TString		mListLabel; // X{Gs}

  Fl_Pack*		wFrontPack;
  Fl_Box*		wIndentBox;
  Fl_Button*		wExpander;
  Fl_Button*		wListExpander;
  FTW::NameButton*	wName;
  FTW::ListDesignator*  wListDesignation;
  Fl_Button*		wSepBox;
  Fl_Pack*		wAntPack;

  MTW_ClassView*	wCustomView;
  Fl_Box*		wTailBox;
  // Rnr Ctrl weedget

  void resize_weeds();
  virtual void label_namebox();
  virtual void label_weeds();
  void label_designation();
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
  AList::ElRep GetElRep() { return AList::ElRep(fImg ? fImg->fLens : 0, mListId, mListLabel); }

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

  bool IsList()           { return fImg ? fImg->fIsList : false; }
  bool IsListMember()     { return bIsListMember; }
  bool IsLinkDescendant() { return bIsLinkDescendant; }

  int  	   AntPos(FTW_Ant* a);
  FTW_Ant* AntAt(int i);
  FTW_Ant* LastAnt();

  virtual void SpawnSeparatorDialog() {}

  //--------------------------------------------------------------
  // find_if structures
  //--------------------------------------------------------------

  struct leaf_eq_lens : public unary_function<FTW_Leaf*, bool> {
    ZGlass* lens;
    leaf_eq_lens(ZGlass* l) : lens(l) {}
    bool operator()(const FTW_Leaf* l) {
      return l->fImg && l->fImg->fLens == lens;
    }
  };

  struct leaf_eq_id : public unary_function<FTW_Leaf*, bool> {
    Int_t id;
    leaf_eq_id(Int_t i) : id(i) {}
    bool operator()(const FTW_Leaf* l) {
      return l->mListId == id;
    }
  };

  struct leaf_eq_label : public unary_function<FTW_Leaf*, bool> {
    TString label;
    leaf_eq_label(const TString& l) : label(l) {}
    bool operator()(const FTW_Leaf* l) {
      return l->mListLabel == label;
    }
  };

  //--------------------------------------------------------------

#include "FTW_Leaf.h7"
}; // endclass FTW_Leaf

#endif
