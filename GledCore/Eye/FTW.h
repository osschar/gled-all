// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_H
#define GledCore_FTW_H

#include "OptoStructs.h"
class FTW_Shell; class FTW_Nest;
class FTW_Leaf;  class FTW_Ant;

#include "FltkGledStuff.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>

class Pupil; class PupilInfo; class NestInfo;

namespace FTW {

  extern Fl_Color	background_color;
  extern Fl_Color	separator_color;

  extern Fl_Color	target_modcol;
  extern Fl_Color	source_modcol;
  extern Fl_Color	sink_modcol;

  extern Fl_Boxtype	separator_box;

  /**************************************************************************/

  struct Leaf_o_Img {
    OptoStructs::ZGlassImg* img;
    FTW_Leaf* leaf;
    Leaf_o_Img(OptoStructs::ZGlassImg* i, FTW_Leaf* l=0) : img(i), leaf(l) {}
  };
  typedef list<Leaf_o_Img>		lLoI_t;
  typedef list<Leaf_o_Img>::iterator	lLoI_i;

  struct Leoim_img_eq : public unary_function<Leaf_o_Img, bool> {
    OptoStructs::ZGlassImg* cmp_img;
    Leoim_img_eq(OptoStructs::ZGlassImg* i) : cmp_img(i) {}
    bool operator()(const Leaf_o_Img& lo) {
      return lo.img == cmp_img;
    }
  };

  /**************************************************************************/

  struct Locator {

    bool	 is_link;
    bool	 is_list_member;
    virtual bool has_contents();
    virtual bool is_list();

    FTW_Leaf	*leaf, *old_leaf;
    FTW_Ant	*ant,  *old_ant;
    FTW_Nest	*nest;

    Locator(FTW_Nest* n) : nest(n)
    { clear(); }
    Locator(FTW_Nest* n, FTW_Leaf* l, FTW_Ant* a=0) : nest(n)
    { clear(); set(l, a); }

    void clear();
    void clear_old();
    void set(FTW_Leaf* l, FTW_Ant* a=0);
    void revert();

    ID_t		    get_contents(); // should be get_id
    ZGlass*		    get_glass();
    OptoStructs::ZGlassImg* get_image();
    GledNS::ClassInfo*	    get_class_info();

    ZGlass*		    get_leaf_glass();
    ID_t		    get_leaf_id();

    void up();
    void down();
    void left();
    void right();

    void mod_old_box_color(Fl_Color mod, bool on_p);
    void mod_box_color(Fl_Color mod, bool on_p);
  };

  struct LocatorConsumer {
    Locator* m_base;

    LocatorConsumer() : m_base(0) {}

    virtual void set_base(Locator& loc);
    virtual void clear_base();
    virtual void destroy_base() { clear_base(); }
    virtual void locator_change(Locator& loc) = 0;
  };

  /**************************************************************************/
  /**************************************************************************/

  class NameButton : public Fl_Button {
  public:
    enum Loc_e   { L_Leaf, L_Ant };

  protected:
    Loc_e	m_loc;
    
    FTW_Leaf*	get_leaf();
    FTW_Ant*	get_ant();

    void set_nests_info_bar(FTW_Leaf* leaf, FTW_Ant* ant, const char* prefix="");

  public:
    NameButton(Loc_e l, int x, int y, int w, int h, const char* t=0);

    NameButton(FTW_Leaf* leaf, int x, int y, int w, int h, const char* t=0);
    NameButton(FTW_Ant* ant, int x, int y, int w, int h, const char* t=0);

    void set_label(const char* t) { if(t) copy_label(t); else label(0); }
    int handle(int ev);
  };

  /**************************************************************************/

  class NameBox : public OptoStructs::A_View, public Fl_Box {
  public:
    NameBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h);

    virtual void AbsorbRay(Ray& ray);

    void ChangeImage(OptoStructs::ZGlassImg* new_img);
  };

  /**************************************************************************/
  /**************************************************************************/
  // Selectors
  /**************************************************************************/
  /**************************************************************************/

  class Top_Selector {
  public:
    enum Type_e { T_Undef=-1, T_Locator, T_Inst, T_Direct, T_DevNull };

  protected:
    FTW_Shell*	m_shell;
    Type_e	m_cur_type;

  public:
    Top_Selector(FTW_Shell* s) : m_shell(s), m_cur_type(T_Undef) {}

    FTW_Shell* get_shell() const { return m_shell; }

    virtual void set_type(Type_e t) = 0;
    virtual Fl_Color get_light_color() = 0;
    virtual Fl_Color get_active_modcol() = 0;
  };


  /**************************************************************************/

  class Bot_Selector {
  protected:
    Top_Selector*	 m_top;
    Top_Selector::Type_e m_type_for_top;

  public:
    Bot_Selector(Top_Selector* ts, Top_Selector::Type_e t)
      : m_top(ts), m_type_for_top(t) {}

    virtual void set_top() { m_top->set_type(m_type_for_top); }

    virtual void activate  () {}
    virtual void deactivate() {}
  };


  /**************************************************************************/
  /**************************************************************************/

  class Inst_Selector : public Bot_Selector, public Fl_Group {
    Fl_Button* wTop;
    Fl_Button* wLid;
    Fl_Button* wCid;

  public:
    Inst_Selector(Top_Selector* ts, Top_Selector::Type_e t);
    ~Inst_Selector();

    void set_lid(UInt_t l);
    void set_cid(UInt_t c);
    void parent_fid();

    UInt_t get_lid();
    UInt_t get_cid();

    virtual void activate  ();
    virtual void deactivate();
    virtual int handle(int ev);
  };


  /**************************************************************************/

  class Locator_Selector : public Bot_Selector, public LocatorConsumer,
			   public Fl_Group
  {
  public:
    enum LS_Type_e { LST_Undef=0, LST_Point, LST_Mark };

  protected:
    LS_Type_e	m_type;
    bool	b_colored;

    Fl_Button*	wTop;
    NameBox*	wNestName;
    Fl_Choice*	wType;
    Fl_Button*	wPoint;
    Fl_Button*	wMark;

  public:
    Locator_Selector(Top_Selector* ts, Top_Selector::Type_e t);

    Locator*  get_locator()	{ return m_base; }
    FTW_Nest* get_nest()	{ return m_base ? m_base->nest : 0; }

    void set_type(LS_Type_e type);
    virtual void set_base(Locator& loc);
    virtual void clear_base();
    virtual void destroy_base();
    virtual void locator_change(Locator& loc);

    //void color_locator(Locator& loc);
    //void uncolor_locator(Locator& loc);

    virtual void activate  ();
    virtual void deactivate();
    virtual int handle(int ev);

  };

  /**************************************************************************/

  class Direct_Selector : public Bot_Selector, public Fl_Group {
  protected:
    Fl_Button*                  wTop;
    FltkGledStuff::LensNameBox*	wNameBox;

  public:
    Direct_Selector(Top_Selector* ts, Top_Selector::Type_e t);

    virtual void activate  ();
    virtual void deactivate();

    OptoStructs::ZGlassImg* get_img();
    void set_img(OptoStructs::ZGlassImg* img);
  };

  /**************************************************************************/

  class Null_Selector : public Bot_Selector, public Fl_Group {
  protected:
    Fl_Button*	wTop;
  public:
    Null_Selector(Top_Selector* ts, Top_Selector::Type_e t);

    void set_nest(FTW_Nest* n);
    
    virtual void activate  ();
    virtual void deactivate();
    virtual int handle(int ev);
  };

  /**************************************************************************/
  /**************************************************************************/

  class Source_Selector : public Top_Selector, public Fl_Pack {
    friend class FTW_Shell;
  protected:
    Locator_Selector*	wLoc_Sel;
    Inst_Selector*	wInst_Sel;
    Direct_Selector*	wDir_Sel;

    Bot_Selector*	wCurrent;

    //    ListOpsSelector*	wListOpsSel;
    
  public:
    Source_Selector(FTW_Shell* s, int x, int y, int dw, const char* t);

    virtual void set_type(Type_e t);
    virtual Fl_Color get_light_color()   { return FL_RED; }
    virtual Fl_Color get_active_modcol() { return source_modcol; }

    bool has_contents();
    ID_t get_contents();

    ZMIR* generate_MIR(GledNS::MethodInfo* mi, ZGlass* alpha, ZGlass* gamma=0);
  };

  /**************************************************************************/

  class Sink_Selector : public Top_Selector, public Fl_Pack {
    friend class FTW_Shell;
  protected:
    Locator_Selector*	wLoc_Sel;
    Direct_Selector*	wDir_Sel;
    Null_Selector*	wNull_Sel;

    Bot_Selector*	wCurrent;

    // ListOpsSelector*	wListOpsSel;

  public:
    Sink_Selector(FTW_Shell* s, int x, int y, int dw, const char* t);

    virtual void set_type(Type_e t);
    virtual Fl_Color get_light_color()   { return FL_RED; }
    virtual Fl_Color get_active_modcol() { return sink_modcol; }

    void swallow_victim(ZGlass* beta);
  };

  /**************************************************************************/

  /*
    class Target_Selector : public Top_Selector, public Fl_Pack {

    public:
    Target_Selector();

    
    };
  */

} // namespace FTW

#endif
