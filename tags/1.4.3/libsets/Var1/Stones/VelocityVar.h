// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_VelocityVar_H
#define Var1_VelocityVar_H

#include <Gled/GledTypes.h>
#include <TObject.h>

class VelocityVar : public TObject
{
public:
  enum DesireMode_e
  {
    DM_DesiredVelocity,
    DM_DesiredDeltaVelocity, // goes to DesiredVelocity when reached
    DM_DesiredDeltaValue,    // goes to DesiredVelocity 0 when reached

    // The following are composite ones. Should split them off? Next level control?
    DM_RandomRange, // Chooses delta, then keeps it for some time, goes back
    DM_RandomWalk
  };

protected:
  DesireMode_e fMode;
  Float_t      fDesire;

  Float_t      fValue;
  // Hmmh, do we need it here? Now the angular velocities are packed
  // in a vector of some sort. While this is ok calculations of the
  // analyitical mechanics stuff, it encumbers the connection.
  // Could:
  //   a) Have a reference to the vector value.
  //   b) Instantiate vector quantities where they are needed.
  //      This is also good, as they can be in any format there.

private:
  void _init();

protected:

public:
  VelocityVar();
  virtual ~VelocityVar();

#include "VelocityVar.h7"
  ClassDef(VelocityVar, 1);
}; // endclass VelocityVar

#endif
