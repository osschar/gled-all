// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(GTSurf);

private:
  void	_init();

protected:
#ifndef __CINT__
  GTS::GtsSurface*	pSurf;  //! X{G}
#endif

  Float_t		mScale;	// X{GST}  7 Value(-range=>[0,1000,1,1000])
  TString		mFile;	// X{GS}   7 Filor()

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

  void Replace(GTS::GtsSurface* new_surf);
  void Load();		//  X{E} 7 MButt(-join=>1)
  void Save();		//  X{E} 7 MButt()
  
  void CalcStats();	//! X{E} 7 MButt(-join=>1)
  void PrintStats();	//! X{E} 7 MButt()

  void Invert(); 	//  X{E} 7 MButt()

#include "GTSurf.h7"
  ClassDef(GTSurf, 1)
}; // endclass GTSurf

GlassIODef(GTSurf);

#endif
