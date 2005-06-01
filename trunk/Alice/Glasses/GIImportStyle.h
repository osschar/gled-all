// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_GIImportStyle_H
#define Alice_GIImportStyle_H

#include <Glasses/ZGlass.h>

class GIImportStyle : public ZGlass {
  MAC_RNR_FRIENDS(GIImportStyle);

 private:
  void _init();

 protected:

 public:
  Bool_t    mImportKine;   // X{GS} 7 Bool(-join=>1)
  Bool_t    mRnrKine;      // X{GS} 7 Bool()

  Bool_t    mImportHits;   // X{GS} 7 Bool(-join=>1)
  Bool_t    mRnrHits;      // X{GS} 7 Bool()

  Bool_t    mImportRec;    // X{GS} 7 Bool(-join=>1)
  Bool_t    mRnrRec;       // X{GS} 7 Bool()

  Bool_t    mImportClusters; // X{GS} 7 Bool(-join=>1)
  Bool_t    mRnrClusters;    // X{GS} 7 Bool()

  GIImportStyle(const Text_t* n="GIImportStyle", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "GIImportStyle.h7"
  ClassDef(GIImportStyle, 1)
    }; // endclass GIImportStyle

GlassIODef(GIImportStyle);

#endif
