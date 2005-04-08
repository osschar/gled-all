// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
class A_Rnr;
class Fl_Widget;
class TMessage;

namespace GledViewNS {
  /**************************************************************************/
  // Spooky structs & typedefs
  /**************************************************************************/

  // Sub level: Rnr Bits and Ctrl

  struct RnrBits {
    // At which RenderLevel to call a specific rendering
    // Values of fX are RenderLevels at which to invoke the method
    // 0 ~ PreDraw, 1 ~ Draw, 2 ~ PostDraw; 3 ~ FullDescent
    UChar_t	fSelf[4];	// Self rendering
    UChar_t	fList[4];	// List member rendering

    RnrBits(UChar_t a=0, UChar_t b=0, UChar_t c=0, UChar_t d=0,
	    UChar_t x=0, UChar_t y=0, UChar_t w=0, UChar_t z=0) {
      fSelf[0] = a; fSelf[1] = b; fSelf[2] = c; fSelf[3] = d;
      fList[0] = x; fList[1] = y; fList[2] = w; fList[3] = z;
    }
    void SetSelf(UChar_t a=0, UChar_t b=0, UChar_t c=0, UChar_t d=0) {
      fSelf[0] = a; fSelf[1] = b; fSelf[2] = c; fSelf[3] = d;
    }
    void SetList(UChar_t x=0, UChar_t y=0, UChar_t w=0, UChar_t z=0) {
      fList[0] = x; fList[1] = y; fList[2] = w; fList[3] = z;
    }

    bool SelfOn() const { return fSelf[0]!=0 || fSelf[1]!=0 || fSelf[2]!=0 || fSelf[3]!=0; }
    bool SelfOnDirect() const { return fSelf[0]!=0 || fSelf[1]!=0 || fSelf[2]!=0; }
    bool ListOn() const { return fList[0]!=0 || fList[1]!=0 || fList[2]!=0 || fList[3]!=0; }
    bool ListOnDirect() const { return fList[0]!=0 || fList[1]!=0 || fList[2]!=0; }
  };

  struct RnrCtrl {
    RnrBits		fRnrBits;

    RnrCtrl() : fRnrBits(2,4,6,0, 0,0,0,5) {}
    RnrCtrl(const RnrBits& rb) : fRnrBits(rb) {}
  };

  // Low level: weed info

  typedef Fl_Widget*	(*WeedCreator_foo)    (MTW_SubView*);
  typedef void		(*WeedCallback_foo)   (Fl_Widget*, MTW_SubView*);
  typedef void		(*WeedUpdate_foo)     (Fl_Widget*, MTW_SubView*);
  typedef void		(*CtxCallCreator_foo) (TBuffer*);

  struct MethodInfo {};		// Null so far

  struct DataMemberInfo {};	// Null so far

  struct LinkMemberInfo {
    RnrBits		fDefRnrBits;
  };

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

    WeedInfo(const string& s) : InfoBase(s) {}
  };

  typedef list<WeedInfo*>		lpWeedInfo_t;
  typedef list<WeedInfo*>::iterator	lpWeedInfo_i;

  // Intermediate level: ClassInfo, RnrCreator and SubView info

  typedef MTW_SubView*	(*SubViewCreator_foo)(GledNS::ClassInfo* ci, MTW_View* v, ZGlass* g);

  struct ClassInfo {
    SubViewCreator_foo		fooSVCreator;
    string			fRendererGlass;
    RnrCtrl			fDefRnrCtrl;
    GledNS::ClassInfo*		fRendererCI;

    lpWeedInfo_t		fWeedList;

    //----------------------------------------------------------------

    WeedInfo*		FindWeedInfo(const string& name, bool recurse, GledNS::ClassInfo* true_class);
    GledNS::ClassInfo*	GetRendererCI();
    A_Rnr*		SpawnRnr(const string& rnr, ZGlass* g);
  };

  typedef A_Rnr*	(A_Rnr_Creator_foo)	(ZGlass*, CID_t);

  typedef hash_map<string, A_Rnr_Creator_foo*>		 hRnr2RCFoo_t;
  typedef hash_map<string, A_Rnr_Creator_foo*>::iterator hRnr2RCFoo_i;

  // High level: per LibSet information

  struct LibSetInfo {
    hRnr2RCFoo_t	Rnr2RCFoo;
  };

  /**************************************************************************/
  // Variables
  /**************************************************************************/

  extern set<string>	RnrNames;

  /**************************************************************************/
  // Functions
  /**************************************************************************/

  Int_t LoadSoSet(const string& lib_set);
  Int_t InitSoSet(const string& lib_set);
  void	BootstrapViewSet(LID_t lid, const string& libset);
  void  BootstrapClassInfo(ClassInfo* c_info);
  void  BootstrapRnrSet(const string& libset, LID_t id,
			const string& rnr,  A_Rnr_Creator_foo rfoo);

  string FabricateViewLibName(const string& libset);
  string FabricateViewInitFoo(const string& libset);
  string FabricateViewUserInitFoo(const string& libset);
  string FabricateRnrLibName(const string& libset, const string& rnr);
  string FabricateRnrInitFoo(const string& libset, const string& rnr);

  // Rnr support

  void AssertRenderers();
  void AddRenderer(const string& rnr);
  
  A_Rnr* SpawnRnr(const string& rnr, ZGlass* d, FID_t fid);

  // GUI magick

  extern int no_symbol_label;
  extern int menubar_box;

} // GledViewNS

#endif
