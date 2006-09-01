// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MTW_Layout.h"
#include "FTW_Nest.h"
#include "FltkGledStuff.h"
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>

#include <cstdlib>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace FGS  = FltkGledStuff;

/**************************************************************************/

namespace {

  /**************************************************************************/
  // Callbacks
  /**************************************************************************/

  void enact_cb(Fl_Widget* w, FTW_Nest* n) {
    n->EnactLayout();
  }

  void menu_cb(Fl_Widget* w, char* layout) {
    MTW_Layout* ml = dynamic_cast<MTW_Layout*>(w->parent());
    ml->GetLaySpecs()->value(layout);
    ml->GetNest()->EnactLayout();
  }

} // end namespace

/**************************************************************************/

MTW_Layout::MTW_Layout(FTW_Nest* nest) :
  Fl_Window(48, 2, "MTW_Layout"),
  mNest(nest), bIsValid(false)
{
  // This should be a beautifull widget with history, several named layouts
  // and with the possibility of loading/appending/saving contents.

  wLaySpecs = new Fl_Input(0,0,48,1);
  wLaySpecs->value("ZGlass(Name,Title)");
  wLaySpecs->callback((Fl_Callback*) enact_cb, nest);
  wLaySpecs->when(FL_WHEN_ENTER_KEY);
  Parse();

  Fl_Button* b = new Fl_Button(0,1,6,1, "Enact");
  b->callback((Fl_Callback*) enact_cb, nest);

  end();
}

/**************************************************************************/

void MTW_Layout::Parse(int cell_w) throw (Exc_t)
{
  static const Exc_t _eh("MTW_Layout::Parse ");

  mClasses.clear(); bIsValid = false;
  lClass_t new_classes;

  TString line(wLaySpecs->value());
  GNS::remove_whitespace(line);
  lStr_t classes;
  GNS::split_string(line, classes, ':');
  if(classes.size()==0) throw _eh + "got void.";
  for(lStr_i c=classes.begin(); c!=classes.end(); ++c) {
    TString cls_name, mmb_args;
    GNS::deparen_string(*c, cls_name, mmb_args, "(");
    // !! could catch, add last ok and rethrow
    FID_t fid = GNS::FindClassID(cls_name);
    if(fid.is_null()) {
      throw _eh + "class "+cls_name+" not found.";
    }
    lStr_t members;
    GNS::split_string(mmb_args, members, ',');
    GNS::ClassInfo* ci = GNS::FindClassInfo(fid);
    if(members.size() == 1 && members.front() == "*") {
      members.clear();
      GVNS::lpWeedInfo_t& wis = ci->fViewPart->fWeedList;
      GVNS::lpWeedInfo_i  wi  = wis.begin();
      while(wi != wis.end()) {
	members.push_back((*wi)->fName);
	++wi;
      }
    }
    new_classes.push_back( Class(ci) );
    Class& cls = new_classes.back();
    for(lStr_i m=members.begin(); m!=members.end(); ++m) {
      TString mmb_name, width_arg;
      GNS::deparen_string(*m, mmb_name, width_arg, "{[", true);
      GVNS::WeedInfo* mi = ci->fViewPart->FindWeedInfo(mmb_name);
      if(mi==0) {
	throw _eh + "member '"+mmb_name+"' not found for class '"+cls_name+"'";
      }
      int w = atoi(width_arg.Data());
      if(w == 0) w = TMath::Max(mi->fWidth, FGS::swm_label_width(*m, cell_w));
      cls.fMembers.push_back(Member(mi, w));
      cls.fFullW += w;
    }
  }
  bIsValid = true;
  mClasses.swap(new_classes);
}

int MTW_Layout::CountSubViews(ZGlass* lens)
{
  if(!lens) return 0;
  int cnt = 0;
  for(lClass_i i=mClasses.begin(); i!=mClasses.end(); ++i) {
    if(GNS::IsA(lens, i->fClassInfo->fFid)) ++cnt;
  }
  return cnt;
}

/**************************************************************************/

Fl_Group* MTW_Layout::CreateLabelGroup()
{
  Fl_Group* g = new Fl_Group(0,0,1,1);
  int x=0;
  for(lClass_i c=mClasses.begin(); c!=mClasses.end(); ++c) {
    Fl_Box* b = new Fl_Box(x, 0, c->fFullW, 1, c->fClassInfo->fName.Data());
    b->box(FL_EMBOSSED_BOX);
    if((int)(c->fClassInfo->fName.Length()) >= c->fFullW)
      b->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
    else
      b->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    for(lMember_i m=c->fMembers.begin(); m!=c->fMembers.end(); ++m) {
      Fl_Box* b = new Fl_Box(x, 1, m->fW, 1, m->fWeedInfo->fName.Data());
      b->box(FL_EMBOSSED_BOX);
      if((int)(m->fWeedInfo->fName.Length()) >= m->fW)
	b->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
      else
	b->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
      if(m->fWeedInfo->fName.Length() > m->fW) {
	b->tooltip(m->fWeedInfo->fName.Data());
      }
      x += m->fW;
    }
  }
  g->end();
  g->resizable(0);
  g->size(x,2);
  return g;
}
