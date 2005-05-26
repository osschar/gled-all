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
  Bool_t    mImportHits;
  Bool_t    mImportClusters;
  Bool_t    mImportKine;

  Bool_t    mRnrKine;
  Bool_t    mRnrHits;
  
  Bool_t    mRnrClusters;

  Bool_t    mImportRec;
  Bool_t    mRnrRec;

  GIImportStyle(const Text_t* n="GIImportStyle", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "GIImportStyle.h7"
  ClassDef(GIImportStyle, 1)
    }; // endclass GIImportStyle

GlassIODef(GIImportStyle);

#endif
