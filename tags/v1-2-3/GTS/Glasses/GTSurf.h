// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

  friend class GTSIsoMaker;

private:
  void	_init();
#ifndef __CINT__
  GTS::Surface*		pSurf;	//! Should be GRL defed
#endif

protected:
  Float_t		mScale;	// X{GST}  7 Value(-range=>[0,1000,1,1000])
  TString		mFile;	// X{GS}   7 Filor()
  bool			bPoints;// X{GS}   7 Bool()
  ZColor		mColor;	// X{PGST} 7 ColorButt()

public:
  GTSurf(Text_t* n="GTSurf", Text_t* t=0) : ZNode(n,t) { _init(); }

  void Load();		// X{E} 7 MButt(-join=>1)
  void Invert(); 	// X{E} 7 MButt()

#include "GTSurf.h7"
  ClassDef(GTSurf, 1)
}; // endclass GTSurf

GlassIODef(GTSurf);

#endif
