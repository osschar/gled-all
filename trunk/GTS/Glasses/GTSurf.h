// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSurf_H
#define GTS_GTSurf_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/SGTSRange.h>

//#ifndef __CINT__
//#include <GTS/GTS.h>
//#endif

namespace GTS {
  struct _GtsSurface;
  typedef struct _GtsSurface GtsSurface;
}

class GTSurf : public ZNode {

  MAC_RNR_FRIENDS(GTSurf);

private:
  void	_init();

protected:
  GTS::GtsSurface*	pSurf;  //! X{g}

  TString		mFile;  // X{GS}   7 Filor()

  bool			bPoints;// X{GS}   7 Bool()
  ZColor		mColor;	// X{PGST} 7 ColorButt()

  UInt_t		mVerts; //! X{G}   7 ValOut(-width=>6, -join=>1)
  UInt_t		mEdges; //! X{G}   7 ValOut(-width=>6, -join=>1)
  UInt_t		mFaces; //! X{G}   7 ValOut(-width=>6)

  SGTSRange		mFaceQuality; //! X{RGS} 7 StoneOutput(Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mFaceArea;    //! X{RGS} 7 StoneOutput(Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mEdgeLength;  //! X{RGS} 7 StoneOutput(Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])
  SGTSRange		mEdgeAngle;   //! X{RGS} 7 StoneOutput(Fmt=>"%4.2le | %4.2le : %4.2le | %4.2le", Args=>[Min, Avg, Sigma, Max])

public:
  GTSurf(Text_t* n="GTSurf", Text_t* t=0) : ZNode(n,t) { _init(); }

  void ReplaceSurface(GTS::GtsSurface* new_surf);
  GTS::GtsSurface* CopySurface();

  void Load();                //  X{E} 7 MButt(-join=>1)
  void Save();                //  X{E} 7 MButt()
  
  void CalcStats();           //! X{E} 7 MButt(-join=>1)
  void PrintStats();          //! X{E} 7 MButt()

  void Destroy();             //  X{E} 7 MButt(-join=>1)
  void Invert();              //  X{E} 7 MButt()

  void Rescale(Double_t s=2); // X{E} 7 MCWButt()

  void Tessellate(UInt_t order=1, Bool_t mid_edge=true); // X{E} 7 MCWButt()

  void GenerateSphere(UInt_t order=1); // X{E} 7 MCWButt()
  void GenerateTriangle(Double_t s=1); // X{E} 7 MCWButt()

  void Legendrofy(Int_t max_l, Double_t abs_scale=0.1,Double_t pow_scale=2.7); // X{E} 7 MCWButt()

  void ExportTring(const Text_t* fname=0);

#include "GTSurf.h7"
  ClassDef(GTSurf, 1);
}; // endclass GTSurf


#endif
