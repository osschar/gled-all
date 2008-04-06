// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSSeed_H
#define Geom1_WSSeed_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/TimeMakerClient.h>

class WSPoint;
class ZImage;
class TubeTvor;

class WSSeed : public ZNode, public TimeMakerClient
{
  MAC_RNR_FRIENDS(WSSeed);

private:
  void _init();

  Float_t hTexU; //! Texture coords; used during Triangulation
  Float_t hTexV; //! and for storage of TexOrig for texture animation

protected:
  virtual void on_insert(iterator it);
  virtual void on_remove(iterator it);
  virtual void on_rebuild();

  virtual void clear_list();

  WSPoint* get_first_point();

  ZTrans* init_slide(WSPoint* f);
  void ring(ZTrans& lcf, WSPoint* f, Double_t t);

  Int_t		mTLevel;	// X{GST}  7 Value(-range=>[2,1000,1,1],-join=>1)
  Int_t		mPLevel;	// X{GST}  7 Value(-range=>[2,1000,1,1])

  Float_t	mTexUOffset;	// X{GS}   7 Value(-range=>[-1e6,1e6,1,1000], -join=>1)
  Float_t	mTexVOffset;	// X{GS}   7 Value(-range=>[-1e6,1e6,1,1000])
  Float_t	mTexUScale;	// X{GS}   7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexVScale;	// X{GS}   7 Value(-range=>[-1e3,1e3,1,1000])

  Float_t       mTrueLength;    // X{GST}  7 ValOut(-range=>[0,1000,1,100], -join=>1)
  Bool_t	bRenormLen;	// X{GST}  7 Bool(-join=>1)
  Float_t	mLength;	// X{GST}  7 Value(-range=>[0,1000,1,100])

  ZColor	mColor;		// X{PGST} 7 ColorButt(-join=>1)
  Float_t	mLineW;		// X{GST}  7 Value(-range=>[0,128,1,100],-join=>1)
  Bool_t	bFat;		// X{GST}  7 Bool()

  ZLink<ZImage>	mTexture;	// X{gS} L{} RnrBits{4,0,5,0, 0,0,0,0}

  // Texture animation
  //------------------
  Float_t	mDtexU;		// X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Float_t	mDtexV;		// X{GS} 7 Value(-range=>[-100,100,1,1000])

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!
  WSPoint*      m_first_point;	//!
  WSPoint*      m_last_point;	//!
  Int_t         m_num_points;   //!

  // Animation control
  //------------------
  Bool_t        bAnimRunning;   // X{GS} 7 BoolOut(-join=>1)
  UInt_t        mAnimSleepMS;   // X{GS} 7 Value(-range=>[0,100000,1])
  Float_t       mAnimTime;      // X{GS} 7 Value(-range=>[-1e6,1e6,1,1000], -join=>1)
  Float_t       mAnimStepFac;   // X{GS} 7 Value(-range=>[-1e6,1e6,1,1000])
  UInt_t        m_anim_tick;    //! Inner variable for overflow checks.
public:
  void StartAnimation();        // X{ED} 7 MButt(-join=>1)
  void StopAnimation();         // X{E}  7 MButt()


public:
  WSSeed(const Text_t* n="WSSeed", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~WSSeed();

  // virtuals
  virtual void Triangulate();

  Float_t Length();
  Float_t MeasureLength();
  void    MeasureAndSetLength(); // X{E} 7 MButt();

  void TransAtTime(ZTrans& lcf, Double_t time,
		   Bool_t repeat_p=false, Bool_t reinit_trans_p=false);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  // Toys
  void MakeLissajou(Double_t t_min, Double_t t_max, Int_t n_points,
		    Double_t ax, Double_t wx, Double_t dx,
		    Double_t ay, Double_t wy, Double_t dy,
		    Double_t az, Double_t wz, Double_t dz,
		    Float_t def_width=0.1);     // X{E} 7 MCWButt()
  void MakeFromFormulas(Double_t t_min, Double_t t_max, Int_t n_points,
			TString fx="cos(x)",
			TString fy="sin(x)",
			TString fz="x",
			Float_t def_width=0.1); // X{E} 7 MCWButt()

#include "WSSeed.h7"
  ClassDef(WSSeed, 1)
}; // endclass WSSeed


#endif
