// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MIR_Priest_H
#define GledCore_MIR_Priest_H

class ZMIR;

class MIR_Priest {
  friend class Saturn;
protected:
  virtual void BlessMIR(ZMIR& mir) = 0;
};

#endif
