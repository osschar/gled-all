// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MTW_Layout.h"
#include "FTW_Nest.h"
#include <Gled/GledNS.h>
#include <Glasses/ZGlass.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>

#include <cstdlib>

/**************************************************************************/

namespace {

  /**************************************************************************/
  // Simple Parser
  /**************************************************************************/

  int split_string(Str_ci start, Str_ci end, lStr_t& l, char c=0)
  {
    int cnt=0;
    string g;
    for(Str_ci i=start; i!=end; ++i) {
      if(c==0 && isspace(*i) && g.size()>0) {
	++cnt; l.push_back(g); g.erase(); continue;
      }
      if(isspace(*i)) continue;
      if(*i==c) {
	++cnt; l.push_back(g); g.erase(); continue;
      }
      g += *i;
    }
    if(g.size()>0) { ++cnt; l.push_back(g); }
    return cnt;
  }

  int split_string(const string& s, lStr_t& l, char c=0)
  {
    return split_string(s.begin(), s.end(), l, c);
  }

  void deparen_string(const string& in, string& n, string& a,
		      const string& ops="([{", bool no_parens_ok=false)
    throw (string)
  {
    // expects back parens to be the last char ... could as well grep it
    unsigned int op_pos = in.find_first_of(ops);
    if(op_pos==string::npos) {
      if(no_parens_ok) {
	if(in.size()==0) throw string("missing name");
	n = in;
	return;
      } else {
	throw string("no open paren");
      }
    }
    int cp_pos = in.size()-1;
    char o = in[op_pos];
    char c = in[in.size()-1];
    if(o=='('&&c!=')' || o=='['&&c!=']' || o=='{'&&c!='}') {
      throw string("no close paren");
    }
    n = in;
    n.replace(op_pos, cp_pos-op_pos+1, "");
    a = in.substr(op_pos+1, cp_pos-op_pos-1);
    if(n.size()==0) throw string("missing name");
    if(a.size()==0) throw string("no args for " + n);
  }

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

void MTW_Layout::Parse() throw (string)
{
  mClasses.clear(); bIsValid = false;
  lClass_t new_classes;

  string line(wLaySpecs->value());
  lStr_t classes;
  split_string(line, classes, ':');
  if(classes.size()==0) throw string("got void");
  for(lStr_i c=classes.begin(); c!=classes.end(); ++c) {
    string cls_name, mmb_args;
    deparen_string(*c, cls_name, mmb_args, "(");
    // !! could catch, add last ok and rethrow
    FID_t fid = GledNS::FindClass(cls_name);
    if(fid.is_null()) {
      throw string("class "+cls_name+" not found");
    }
    lStr_t members;
    split_string(mmb_args, members, ',');
    if(members.size()==0) {
      // Could as well silently skip ... or dump the whole lot
      throw string("no members for class "+cls_name);
    }
    GledViewNS::ClassInfo* ci = GledViewNS::FindClassInfo(fid);
    new_classes.push_back( Class(ci) );
    Class& cls = new_classes.back();
    for(lStr_i m=members.begin(); m!=members.end(); ++m) {
      string mmb_name, width_arg;
      deparen_string(*m, mmb_name, width_arg, "{[", true);
      GledViewNS::MemberInfo* mi = ci->FindMemberInfo(mmb_name);
      if(mi==0) {
	throw string("member '"+mmb_name+"' not found for class '"+cls_name+"'");
      }
      int w = atoi(width_arg.c_str());
      if(w == 0) w = mi->fWidth >? (int)(mi->fName.size());
      cls.fMembers.push_back(Member(mi, w));
      cls.fFullW += w;
    }
  }
  bIsValid = true;
  mClasses.swap(new_classes);
}

int MTW_Layout::CountSubViews(ZGlass* glass)
{
  int cnt = 0;
  for(lClass_i i=mClasses.begin(); i!=mClasses.end(); ++i) {
    if(GledNS::IsA(glass, i->fClassInfo->fFid)) ++cnt;
  }
  return cnt;
}

/**************************************************************************/

Fl_Group* MTW_Layout::CreateLabelGroup()
{
  Fl_Group* g = new Fl_Group(0,0,1,1);
  int x=0;
  for(lClass_i c=mClasses.begin(); c!=mClasses.end(); ++c) {
    Fl_Box* b = new Fl_Box(x, 0, c->fFullW, 1, c->fClassInfo->fClassName.c_str());
    b->box(FL_EMBOSSED_BOX);
    if((int)(c->fClassInfo->fClassName.size()) >= c->fFullW)
      b->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
    else
      b->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
    for(lMember_i m=c->fMembers.begin(); m!=c->fMembers.end(); ++m) {
      Fl_Box* b = new Fl_Box(x, 1, m->fW, 1, m->fMemberInfo->fName.c_str());
      b->box(FL_EMBOSSED_BOX);
      if((int)(m->fMemberInfo->fName.size()) >= m->fW)
	b->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
      else
	b->align(FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
      x += m->fW;
    }
  }
  g->end();
  g->resizable(0);
  g->size(x,2);
  return g;
}
