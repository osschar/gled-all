// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_OptoStructs
#define Gled_OptoStructs

#include <Glasses/ZGlass.h>
#include <GledView/GledViewNS.h>

class ZGlass;
class Eye;
class Ray;
class A_Rnr; class RnrScheme;
class Fl_Window;

namespace OptoStructs {

  struct ZGlassView;

  struct A_View;
  typedef list<A_View*>			lpA_View_t;
  typedef list<A_View*>::iterator	lpA_View_i;

  struct A_GlassView;
  typedef list<A_GlassView*>		lpA_GlassView_t;
  typedef list<A_GlassView*>::iterator	lpA_GlassView_i;

  class ZLinkView;
  typedef list<ZLinkView*>		lpZLinkView_t;
  typedef list<ZLinkView*>::iterator	lpZLinkView_i;

  struct ZLinkDatum;
  typedef list<ZLinkDatum>		lZLinkDatum_t;
  typedef list<ZLinkDatum>::iterator	lZLinkDatum_i;

  /**************************************************************************/
  // ZGlassImg: complete image of a glass in an eye
  /**************************************************************************/

  struct ZGlassImg {
    Eye*		fEye;
    ZGlass*		fGlass;
    GledNS::ClassInfo*	fClassInfo;

    bool		fIsList;
    ZGlassView*		fDefView;
    lZLinkDatum_t	fLinkData;

    Fl_Window*		fFullMTW_View;
    lpA_View_t		fFullViews;
    lpA_View_t		fLinkViews;

    ZGlassImg(Eye* e, ZGlass* g);
    ~ZGlassImg();

    void CheckInFullView(A_View* v)  { fFullViews.push_back(v); }
    void CheckOutFullView(A_View* v) { fFullViews.remove(v); }
    void CheckInLinkView(A_View* v)  { fLinkViews.push_back(v); }
    void CheckOutLinkView(A_View* v) { fLinkViews.remove(v); }

    void CreateDefView();
    void AssertDefView();
  };

  typedef list<ZGlassImg*>			  lpZGlassImg_t;
  typedef list<ZGlassImg*>::iterator		  lpZGlassImg_i;
  typedef hash_map<ZGlass*, ZGlassImg*>		  hpZGlass2pZGlassImg_t;
  typedef hash_map<ZGlass*, ZGlassImg*>::iterator hpZGlass2pZGlassImg_i;

  /**************************************************************************/
  // ZLinkDatum: information provided for links of each lens
  /**************************************************************************/

  struct ZLinkDatum {
    GledNS::LinkMemberInfo*	fLinkInfo;
    ZGlassImg*			fOwnerImg;
    ZGlass*& 			fLinkRef;

    ZLinkDatum(GledNS::LinkMemberInfo* lmi, ZGlassImg* i, ZGlass*& lr) :
      fLinkInfo(lmi), fOwnerImg(i), fLinkRef(lr) {}
  };

  /**************************************************************************/
  // A_View: base of all views
  /**************************************************************************/

  struct A_View {
    ZGlassImg*	fImg;

    A_View(ZGlassImg* i) : fImg(i) {}
    virtual ~A_View() {}

    virtual void AbsorbRay(Ray& ray) {}

    virtual void InvalidateRnrScheme() {}
  };

  /**************************************************************************/
  // A_GlassView: abstract interface for all Glass views
  /**************************************************************************/

  struct A_GlassView : public A_View {
    // abstract interface to link/list info
    // also owns rnr and rnr_scheme ptrs
    // ctor/dtor do check-in / check-out

    A_Rnr*	fRnr;
    RnrScheme*	fRnrScheme;

    A_GlassView(ZGlassImg* i);
    virtual ~A_GlassView();

    virtual void AssertDependantViews() = 0;

    virtual void CopyLinkViews(lpZLinkView_t& v) {}
    virtual void CopyListViews(lpA_GlassView_t& v) {}

    virtual const GledViewNS::RnrCtrl& GetRnrCtrl()
    { return fImg->fClassInfo->fViewPart->fDefRnrCtrl; }

    virtual const GledViewNS::RnrBits& GetRnrBits()
    { return GetRnrCtrl().fRnrBits; }

    virtual A_Rnr* GetRnr(const string& rnr)
    { return fRnr; }

    virtual void   SpawnRnr(const string& rnr);
    virtual void   AssertRnr(const string& rnr)
    { if(fRnr == 0) SpawnRnr(rnr); }

    virtual void   AssertListRnrs(const string& rnr) {}

    virtual void InvalidateRnrScheme();
  };

  typedef list<A_GlassView*>		lpA_GlassView_t;
  typedef list<A_GlassView*>::iterator	lpA_GlassView_i;

  /**************************************************************************/
  // ZGlassView: default view for glasses
  /**************************************************************************/

  struct ZGlassView : public A_GlassView {
    // ctor creates LinkViews
    // dtor wipes them
    lpZLinkView_t	fLinkViews;

    ZGlassView(ZGlassImg* img);
    virtual ~ZGlassView();

    virtual void AssertDependantViews();
    virtual void CopyLinkViews(lpZLinkView_t& v);

    virtual void AbsorbRay(Ray& ray);
  };

  typedef list<ZGlassView*>		lpZGlassView_t;
  typedef list<ZGlassView*>::iterator	lpZGlassView_i;

  /**************************************************************************/
  // ZListView: default view for Lists (those that have no links)
  /**************************************************************************/

  struct ZListView : virtual public ZGlassView {
    // ctor creates/obtains daugter imgs
    lpZGlassImg_t		fDaughterImgs;
    void build_daughters();

    ZListView(ZGlassImg* img);
    virtual ~ZListView() {}
    virtual void AssertDependantViews();
    virtual void CopyListViews(lpA_GlassView_t& v);

    virtual void AbsorbRay(Ray& ray);

    virtual void AssertListRnrs(const string& rnr);
  };

  /**************************************************************************/
  // ZLinkView: all about a link
  /**************************************************************************/

  struct ZLinkView : public A_View {
    // Update asserts GlassImg of lens pointed to exits
    // Check in/out from img pointed to also done here
    A_View*	fMasterView;
    ZLinkDatum*	fLinkDatum;
    ZGlassImg*	fToImg;		// image of dude the link is pointing to
    ZGlass*	fToGlass;	// Glass pointing to (or at least pointed to last)

    ZLinkView(ZGlassImg* from, A_View* master, ZLinkDatum* ld) :
      A_View(from), fMasterView(master), fLinkDatum(ld),
      fToImg(0),  fToGlass(0) { Update(); }
    virtual ~ZLinkView();

    virtual bool NeedsUpdate() { return fToGlass != fLinkDatum->fLinkRef; }
    virtual void Update();

    virtual A_GlassView* GetView() { return fToImg->fDefView; }
    // The following method never used
    // virtual A_Rnr*       GetRnr()  { return fToImg->->fDefView->fRnr; }
    
    virtual const GledViewNS::RnrBits& GetRnrBits() {
      return fLinkDatum->fLinkInfo->fViewPart->fDefRnrBits;
    }

    virtual void InvalidateRnrScheme() { fMasterView->InvalidateRnrScheme(); }
  };

}

#endif
