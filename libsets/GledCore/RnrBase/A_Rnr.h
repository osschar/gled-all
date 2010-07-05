// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_A_Rnr_H
#define GledCore_A_Rnr_H

#include <Eye/OptoStructs.h>

class ZGlass;
class RnrDriver;
class RnrScheme;

class A_Rnr : public OptoStructs::A_View
{
  friend class RnrDriver;

public:
  struct NameStackEntry
  {
    A_Rnr* fRnr;
    void*  fUserData;
  };
  typedef NameStackEntry                 NSE_t;
  typedef list<NameStackEntry>           lNSE_t;
  typedef list<NameStackEntry>::iterator lNSE_i;
  typedef vector<NameStackEntry>         vNSE_t;

  struct Fl_Event
  {
    int     fEvent;
    int     fState;
    int     fKey;
    int     fButton;
    int     fButtons;
    int     fClicks;
    int     fIsClick;
    int     fX, fY;
    int     fDX, fDY;
    TString fText;

    bool    fIsKey;
    bool    fIsMouse;

    bool    fIsOverlay;

    lNSE_t  fNameStack;
    lNSE_i  fCurrentNSE;
    int     fNameStackSize;

    float   fZMin, fZMax;

    bool ns_empty() { return fNameStackSize > 0; }
    int  ns_size()  { return fNameStackSize;  }

    void dump();
  };

protected:
  RnrScheme* mRnrScheme;
  bool       bOnePerRnrDriver;

public:
  void crs_links(RnrDriver* rd, RnrScheme* sch);
  void crs_self(RnrDriver* rd, RnrScheme* sch);
  void crs_elements(RnrDriver* rd, RnrScheme* sch);

public:
  A_Rnr() : OptoStructs::A_View(0), mRnrScheme(0), bOnePerRnrDriver(false)
  {}

  virtual void AbsorbRay(Ray& ray);

  virtual void CreateRnrScheme(RnrDriver* rd);
  virtual void DropRnrScheme();

  virtual void PreDraw(RnrDriver* rd) = 0;
  virtual void Draw(RnrDriver* rd) = 0;
  virtual void PostDraw(RnrDriver* rd) = 0;

  virtual void CleanUp(RnrDriver* rd) {}

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev) { return 0; }

  static const int sMaxRnrLevel;
};

typedef void (A_Rnr::*RnrSubDraw_foo)(RnrDriver*); // missing throw (?)

/**************************************************************************/
// ListChangeObserver
/**************************************************************************/

struct ListChangeObserver : public OptoStructs::A_View
{
  A_Rnr* mRnr;

  ListChangeObserver(OptoStructs::ZGlassImg* list, A_Rnr* client) :
    OptoStructs::A_View(list), mRnr(client) {}

  virtual void AbsorbRay(Ray& ray);
};

/**************************************************************************/
// RnrElement
/**************************************************************************/

struct RnrElement
{
  A_Rnr*		    fRnr;
  RnrSubDraw_foo	    fRnrFoo; // If 0, do full descent.

  RnrElement(A_Rnr* r, RnrSubDraw_foo f=0) : fRnr(r), fRnrFoo(f) {}
};

typedef list<RnrElement>		lRnrElement_t;
typedef list<RnrElement>::iterator	lRnrElement_i;

typedef vector<lRnrElement_t>		vlRnrElement_t;
typedef vector<lRnrElement_t>::iterator	vlRnrElement_i;

/**************************************************************************/
// RnrScheme
/**************************************************************************/

// For optimization of updates, the fScheme could be split
// into fSelfScheme, fListScheme, fLinksScheme.
// then different rays would invalidate only the pertinent sub-schemes.

class RnrScheme
{
public:
  list<OptoStructs::A_View*>  fListObservers;
  vlRnrElement_t              fScheme;

  RnrScheme() : fScheme(A_Rnr::sMaxRnrLevel + 1) {}
  ~RnrScheme();

  void AddListChangeObserver(OptoStructs::ZGlassImg* list, A_Rnr* client);

  lRnrElement_t& operator[](int i) { return fScheme[i]; }
}; // endclass RnrScheme

/**************************************************************************/
// RnrMod & RnrModStore
/**************************************************************************/

struct RnrMod
{
  ZGlass      *fLens;
  A_Rnr       *fRnr;
  TimeStamp_t  fTringTS;

  RnrMod(ZGlass* l=0, A_Rnr* r=0, TimeStamp_t ts=0) :
    fLens(l), fRnr(r), fTringTS(ts) {}
};

struct RnrModStore
{
  RnrMod      *fRnrMod;
  TimeStamp_t  fTringTS;
  FID_t	       fFid;

  RnrModStore(FID_t f) : fRnrMod(0), fTringTS(0), fFid(f) {}

  ZGlass* lens() { return fRnrMod->fLens; }
  A_Rnr*  rnr()  { return fRnrMod->fRnr; }
};

template<class TL, class TR=A_Rnr>
struct RnrModStoreT : public RnrModStore
{
  RnrModStoreT()        : RnrModStore(TL::FID()) {}
  RnrModStoreT(FID_t f) : RnrModStore(f)         {}

  TL* lens() { return (TL*) fRnrMod->fLens; }
  TR* rnr()  { return (TR*) fRnrMod->fRnr; }
};

#endif
