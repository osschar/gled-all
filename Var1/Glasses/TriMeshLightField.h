// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMeshLightField_H
#define Var1_TriMeshLightField_H

#include <Glasses/TriMeshField.h>

class TriMeshLightField : public TriMeshField
{
  MAC_RNR_FRIENDS(TriMeshLightField);

private:
  void _init();

protected:

public:
  TriMeshLightField(const Text_t* n="TriMeshLightField", const Text_t* t=0) :
    TriMeshField(n,t) { _init(); }


#include "TriMeshLightField.h7"
  ClassDef(TriMeshLightField, 1) // TriMeshFiled with support for light-map tracing.
}; // endclass TriMeshLightField


#endif
