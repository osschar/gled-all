// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSurf_H
#define GTS_GTSurf_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/SGTSRange.h>

class LegendreCoefs;

//#ifndef __CINT__
//#include <GTS/GTS.h>
//#endif

namespace GTS
{
  struct _GtsSurface;
  typedef struct _GtsSurface GtsSurface;
}

class GTSurf : public ZNode
{
  MAC_RNR_FRIENDS(GTSurf);

public:
  enum PostBoolOp_e { PBM_Noop, PBM_AsValues, PBM_AsFractions };

private:
  void	_init();

protected:
  GTS::GtsSurface*	pSurf;        //! X{g}

  TString		mFile;        // X{GS}   7 Filor()

  ZColor		mColor;	      // X{PGST} 7 ColorButt(-join=>1)
  ZColor		mPointColor;  // X{PGST} 7 ColorButt(-join=>1)
  bool			bRnrPoints;   // X{GST}  7 Bool()

  UInt_t		mVerts;       //! X{G}   7 ValOut(-width=>10, -join=>1)
  UInt_t		mEdges;       //! X{G}   7 ValOut(-width=>10, -join=>1)
  UInt_t		mFaces;       //! X{G}   7 ValOut(-width=>10)

  SGTSRange		mFaceQuality; //! X{RGS} 7 StoneOutput(-width=>32, Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mFaceArea;    //! X{RGS} 7 StoneOutput(-width=>32, Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mEdgeLength;  //! X{RGS} 7 StoneOutput(-width=>32, Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mEdgeAngle;   //! X{RGS} 7 StoneOutput(-width=>32, Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])

  PostBoolOp_e          mPostBoolOp;         // X{GS} 7 PhonyEnum(-join=>1)
  Double_t              mPostBoolArea;       // X{GS} 7 Value()
  Double_t              mPostBoolPerimeter;  // X{GS} 7 Value(-join=>1)
  Double_t              mPostBoolLength;     // X{GS} 7 Value()

public:
  GTSurf(const Text_t* n="GTSurf", const Text_t* t=0) : ZNode(n,t) { _init(); }

  void ReplaceSurface(GTS::GtsSurface* new_surf);
  GTS::GtsSurface* CopySurface();
  GTS::GtsSurface* DisownSurface();

  Double_t GetArea() const;
  Double_t GetXYArea() const;
  Double_t GetVolume() const;

  void Load(const TString& file=""); //  X{E} 7 MCWButt(-join=>1)
  void Save(const TString& file=""); //  X{E} 7 MCWButt()

  void CalcStats();           //! X{E} 7 MButt(-join=>1)
  void PrintStats();          //! X{E} 7 MButt()

  void Destroy();             //  X{E} 7 MButt(-join=>1)
  void Invert();              //  X{E} 7 MButt()

  void Rescale(Double_t s=2); // X{E} 7 MCWButt(-join=>1)
  void RescaleXYZ(Double_t sx=2, Double_t sy=2, Double_t sz=2); // X{E} 7 MCWButt()

  void TransformAndResetTrans(); // X{E} 7 MButt(-join=>1)
  void RotateAndResetRot();      // X{E} 7 MButt()

  void Tessellate(UInt_t order=1, Bool_t mid_edge=true); // X{E} 7 MCWButt()

  void Merge       (GTSurf* a, GTSurf* b); // X{E} C{2} 7 MCWButt(-join=>1)
  void Union       (GTSurf* a, GTSurf* b); // X{E} C{2} 7 MCWButt()
  void Intersection(GTSurf* a, GTSurf* b); // X{E} C{2} 7 MCWButt(-join=>1)
  void Difference  (GTSurf* a, GTSurf* b); // X{E} C{2} 7 MCWButt()

  void GenerateSphere(UInt_t order=1); // X{E} 7 MCWButt(-join=>1)
  void GenerateTriangle(Double_t s=1); // X{E} 7 MCWButt()

  void LegendrofyAdd  (LegendreCoefs* lc, Double_t scale=1, Int_t l_max=-1); // X{E} C{1} 7 MCWButt()
  void LegendrofyScale(LegendreCoefs* lc, Double_t scale=1, Int_t l_max=-1); // X{E} C{1} 7 MCWButt()
  void LegendrofyScaleRandom(Int_t l_max, Double_t abs_scale=0.1,Double_t pow_scale=1.5); // X{E} 7 MCWButt()

  void ExportTring(const Text_t* fname=0);

  void MakeZSplitSurfaces(Double_t z_split=0, const TString& stem="split",
			  Bool_t save_p=false); // X{ED} 7 MCWButt()

#include "GTSurf.h7"
  ClassDef(GTSurf, 1);
}; // endclass GTSurf

#endif
