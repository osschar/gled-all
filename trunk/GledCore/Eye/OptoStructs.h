// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_OptoStructs_H
#define GledCore_OptoStructs_H

#include <Glasses/ZGlass.h>
#include <Gled/GledNS.h>
#include <Eye/Ray.h>

class ZGlass;
class Eye;
class A_Rnr; class RnrScheme;
class MTW_View;

namespace OptoStructs {

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

  /**************************************************************************/
  // ZGlassImg: complete image of a glass in an eye
  /**************************************************************************/

  class ZGlassImg {
  protected:
    lpZGlassImg_t*	fElementImgs; // List-of-images for ALists. Cache.

  public:
    Eye*		fEye;
    ZGlass*		fLens;

    bool		fIsList;
    lZLinkDatum_t	fLinkData;
    lpA_View_t		fViews;

    A_Rnr*		fDefRnr;
    MTW_View*		fFullMTW_View;

    ZGlassImg(Eye* e, ZGlass* lens);
    ~ZGlassImg();

    GledNS::ClassInfo* GetCI()   { return fLens->VGlassInfo(); }

    void PreAbsorption(Ray& ray);
    void PostAbsorption(Ray& ray);

    void CheckInView(A_View* v)  { fViews.push_back(v); }
    void CheckOutView(A_View* v) { fViews.remove(v); }

    ZLinkDatum*    GetLinkDatum(const TString& lnk);
    lpZGlassImg_t* GetElementImgs();
  };

  typedef list<ZGlassImg*>			  lpZGlassImg_t;
  typedef list<ZGlassImg*>::iterator		  lpZGlassImg_i;
  typedef hash_map<ZGlass*, ZGlassImg*>		  hpZGlass2pZGlassImg_t;
  typedef hash_map<ZGlass*, ZGlassImg*>::iterator hpZGlass2pZGlassImg_i;

  /**************************************************************************/
  // ImageConsumer: abstract base for str
  /**************************************************************************/

  class ImageConsumer {
  public:
    virtual ~ImageConsumer() {}
    virtual void ImageDeath(ZGlassImg* img) = 0;
  };

  typedef list<ImageConsumer*>           lpImgConsumer_t;
  typedef list<ImageConsumer*>::iterator lpImgConsumer_i;

  /**************************************************************************/
  // ZLinkDatum: information provided for links of each lens
  /**************************************************************************/

  struct ZLinkDatum {
    ZGlassImg*      fImg;
    ZGlass::LinkRep fLinkRep;
    ZGlass*         fToGlass;
    ZGlassImg*      fToImg;

    ZLinkDatum(ZGlassImg* img, ZGlass::LinkRep& lrep) :
      fImg(img),
      fLinkRep(lrep), fToGlass(lrep.fLinkRef), fToImg(0) {}

    GledNS::LinkMemberInfo* GetLinkInfo() { return fLinkRep.fLinkInfo; }
    ZGlass*&                GetLinkRef () { return fLinkRep.fLinkRef;  }

    ZGlass*    GetToGlass() { return fToGlass; }
    ZGlassImg* GetToImg();
  };

  /**************************************************************************/
  // A_View: base of all lens views
  /**************************************************************************/

  struct A_View {
    ZGlassImg*	fImg;

    A_View(ZGlassImg* i) : fImg(i) { if(fImg) fImg->CheckInView(this);  }
    virtual ~A_View()              { if(fImg) fImg->CheckOutView(this); }

    virtual void SetImg(ZGlassImg* newimg);

    virtual void AbsorbRay(Ray& ray) {}
  };

  /**************************************************************************/
  // ZLinkView: A link representation with state.
  /**************************************************************************/

  struct ZLinkView {
    ZLinkDatum*	fLinkDatum;
    ZGlass*	fToGlass; // Glass pointing to last.

    ZLinkView(ZLinkDatum* ld) : fLinkDatum(ld), fToGlass(0) { Update(); }
    virtual ~ZLinkView() {}

    GledNS::LinkMemberInfo* GetLinkInfo() { return fLinkDatum->GetLinkInfo(); }
    ZGlassImg* GetToImg() { return fLinkDatum->GetToImg(); }

    virtual bool NeedsUpdate() { return fToGlass != fLinkDatum->fToGlass; }
    virtual void Update() { fToGlass = fLinkDatum->fToGlass; }

    // !!! This might go to ZLinkDatum::GetDefRnrBits()
    virtual const GledNS::RnrBits& GetRnrBits() {
      return GetLinkInfo()->fDefRnrBits;
    }
  };

}

#endif
