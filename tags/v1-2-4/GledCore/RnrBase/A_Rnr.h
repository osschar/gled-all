// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_A_Rnr_H
#define Gled_A_Rnr_H

class ZGlass;
class RnrDriver;

class A_Rnr {

public:
  virtual ZGlass* GetGlass() = 0;

  virtual void PreDraw(RnrDriver*) = 0;
  virtual void Draw(RnrDriver*) = 0;
  virtual void PostDraw(RnrDriver*) = 0;

  virtual void CleanUp(RnrDriver*) {}
};

typedef void (A_Rnr::*RnrSubDraw_foo)(RnrDriver*); // missing throw (?)

#endif
