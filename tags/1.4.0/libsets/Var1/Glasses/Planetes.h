// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Planetes_H
#define Var1_Planetes_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class HTriMesh;

class Planetes : public ZNode
{
  MAC_RNR_FRIENDS(Planetes);

private:
  void _init();

protected:
  ZLink<HTriMesh>   mMesh;      //  X{GE} L{}
  ZColor            mColor;     //  X{GSPR} 7 ColorButt()

  Int_t             mDrawLevel; // X{GE} 7 Value(-range=>[0, 100, 1])

public:
  Planetes(const Text_t* n="Planetes", const Text_t* t=0);
  virtual ~Planetes();

  void SetMesh(HTriMesh* mesh);
  void SetDrawLevel(Int_t l);

#include "Planetes.h7"
  ClassDef(Planetes, 1);
}; // endclass Planetes

#endif
