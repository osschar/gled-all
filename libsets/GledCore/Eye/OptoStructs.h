// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_OptoStructs_H
#define GledCore_OptoStructs_H

#include <Glasses/ZGlass.h>
#include <Gled/GledNS.h>
#include <Eye/Ray.h>

class AList;
class Eye;
class A_Rnr; class RnrScheme;
class MTW_View;

namespace OptoStructs
{
  struct ZGlassImg;
  typedef list<ZGlassImg*>                   lpZGlassImg_t;
  typedef list<ZGlassImg*>::iterator         lpZGlassImg_i;
  typedef list<ZGlassImg*>::reverse_iterator lpZGlassImg_ri;

  struct A_View;
  typedef list<A_View*>                      lpA_View_t;
  typedef list<A_View*>::iterator            lpA_View_i;

  class ZLinkView;
  typedef list<ZLinkView*>                   lpZLinkView_t;
  typedef list<ZLinkView*>::iterator         lpZLinkView_i;

  struct ZLinkDatum;
  typedef list<ZLinkDatum>                   lZLinkDatum_t;
  typedef list<ZLinkDatum>::iterator         lZLinkDatum_i;
  typedef list<ZLinkDatum>::reverse_iterator lZLinkDatum_ri;


  //============================================================================
  // ZGlassImg: complete image of a glass in an eye
  //============================================================================

  class ZGlassImg
  {
  protected:
    lpZGlassImg_t      *fElementImgs; // List-of-images for ALists. Cache.

    void ClearElementImgs();

  public:
    Eye                *fEye;
    ZGlass             *fLens;

    lZLinkDatum_t	fLinkData;
    lpA_View_t		fViews;
    UInt_t              fRefCount;

    A_Rnr              *fDefRnr;

    ZGlassImg(Eye* e);
    ZGlassImg(Eye* e, ZGlass* lens);
    ~ZGlassImg();

    GledNS::ClassInfo* GetCI()   { return fLens->VGlassInfo(); }

    void PreAbsorption(Ray& ray);
    void PostAbsorption(Ray& ray);

    void CheckInView(A_View* v);
    void CheckOutView(A_View* v);

    void IncRefCount();
    void DecRefCount();

    bool HasZeroRefCount() const { return fRefCount == 0 && fViews.empty(); }

    ZLinkDatum*    GetLinkDatum(const TString& lnk);
    bool           IsList()  { return GetList() != 0; }
    AList*         GetList() { return fLens->AsAList(); }
    lpZGlassImg_t* GetElementImgs();

    void DumpLinkData();
  };

  typedef list<ZGlassImg*>			  lpZGlassImg_t;
  typedef list<ZGlassImg*>::iterator		  lpZGlassImg_i;


  //============================================================================
  // ImageConsumer: abstract base for classes that hold image maps and
  // need to be notifed of image destruction.
  //============================================================================

  class ImageConsumer
  {
  public:
    virtual ~ImageConsumer() {}
    virtual void ImageDeath(ZGlassImg* img) = 0;
  };


  //============================================================================
  // ZLinkDatum: information provided for links of each lens
  //============================================================================

  struct ZLinkDatum
  {
    ZGlassImg      *fImg;
    ZGlass::LinkRep fLinkRep;
    ZGlass         *fToGlass;

    ZLinkDatum(ZGlassImg* img, ZGlass::LinkRep& lrep) :
      fImg(img), fLinkRep(lrep), fToGlass(lrep.fLinkRef) {}

    ~ZLinkDatum();

    GledNS::LinkMemberInfo* GetLinkInfo() { return fLinkRep.fLinkInfo; }
    ZGlass*&                GetLinkRef () { return fLinkRep.fLinkRef;  }

    void       ResetToGlass();

    ZGlass*    GetToGlass() { return fToGlass; }
    ZGlassImg* GetToImg();
  };


  //============================================================================
  // A_View: base of all lens views
  //============================================================================

  struct A_View
  {
    ZGlassImg	*fImg;

    A_View(ZGlassImg* i) : fImg(i) { if(fImg) fImg->CheckInView(this);  }
    virtual ~A_View()              { if(fImg) fImg->CheckOutView(this); }

    virtual void SetImg(ZGlassImg* newimg);

    virtual void AbsorbRay(Ray& ray) {}
  };


  //============================================================================
  // ZImageHandle
  // Must be owned by A_View that handles Ray absorbtion.
  //============================================================================

  struct ZGlassImgHandle
  {
    ZGlassImg	*fImg;

    ZGlassImgHandle(ZGlassImg* i=0) : fImg(i) { if (fImg) fImg->IncRefCount(); }
    ~ZGlassImgHandle()                        { if (fImg) fImg->DecRefCount(); }

    void SetImg(ZGlassImg* newimg)
    {
      if (fImg) fImg->DecRefCount();
      fImg = newimg;
      if (fImg) fImg->IncRefCount();
    }

    bool UpdateImg(ZGlassImg* newimg)
    {
      if (fImg != newimg) {
	SetImg(newimg);
	return true;
      } else {
	return false;
      }
    }

    ZGlassImgHandle& operator=(ZGlassImg* img) { SetImg(img); return *this; }

    ZGlassImg* operator->() { return fImg; }
    ZGlassImg* operator*()  { return fImg; }

    operator bool ()              const { return fImg != 0; }
    bool operator==(ZGlassImg* i) const { return fImg == i; }
    bool operator!=(ZGlassImg* i) const { return fImg != i; }
  };


  //============================================================================
  // ZLinkView: A link representation with state.
  // Must be owned by A_View that handles Ray absorbtion.
  //============================================================================

  struct ZLinkView
  {
    ZLinkDatum	*fLinkDatum;
    ZGlass      *fToGlass; // Glass pointing to last.

    ZLinkView(ZLinkDatum* ld);
    virtual ~ZLinkView() {}

    GledNS::LinkMemberInfo* GetLinkInfo() { return fLinkDatum->GetLinkInfo(); }
    ZGlassImg*              GetToImg()    { return fLinkDatum->GetToImg(); }

    virtual bool LinkViewNeedsUpdate() { return fToGlass != fLinkDatum->fToGlass; }
    virtual void LinkViewUpdate()      { fToGlass = fLinkDatum->fToGlass; }

    // !!! This might go to ZLinkDatum::GetDefRnrBits()
    virtual const GledNS::RnrBits& GetRnrBits()
    {
      return GetLinkInfo()->fDefRnrBits;
    }
  };

}

#endif
