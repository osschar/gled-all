// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSurf_H
#define GTS_GTSurf_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#ifndef __CINT__
#include <GTS/GTS.h>
#endif

class GTSurf : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(GTSurf);

private:
  void	_Init();
#ifndef __CINT__
  GTS::Surface*		pSurf;	//! Should be GRL defed
#endif
protected:
  Real_t		mScale;	// X{GS}	7 Value(-range=>[0,1000,1,1000])
  TString		mFile;	// X{GS} 7 Filor()
  bool			bPoints;// X{GS} 7 Bool()
  ZColor		mColor;	// X{PGS} 7 ColorButt()
public:
  GTSurf(Text_t* n="GTSurf", Text_t* t=0) : ZNode(n,t) { _Init(); }

  void Load();		// X{e} 77 MButt()
  void Invert(); 	// X{e} 77 MButt()

#include "GTSurf.h7"
  ClassDef(GTSurf, 1)
}; // endclass GTSurf

GlassIODef(GTSurf);

#endif
