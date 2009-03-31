// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_GeoUserData_H
#define RootGeo_GeoUserData_H

#include <TObject.h>
class TGLFaceSet;

class GeoUserData : public TObject {

private:
  void _init();

protected:

public:
  Bool_t	bIsImported;
  TGLFaceSet*	fFaceSet;

  GeoUserData(Bool_t impp=false, TGLFaceSet *fs=0) :
    bIsImported(impp),fFaceSet(fs)
  { _init(); }

#include "GeoUserData.h7"
  ClassDef(GeoUserData, 1);
}; // endclass GeoUserData

#endif
