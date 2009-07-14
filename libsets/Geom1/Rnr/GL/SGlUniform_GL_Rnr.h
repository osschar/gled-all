// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SGlUniform_GL_RNR_H
#define Geom1_SGlUniform_GL_RNR_H

#include <Stones/SGlUniform.h>

class RnrDriver;

class SGlUniform_GL_Rnr
{
private:
  void _init();

protected:
  SGlUniform            *mUni;
  SGlUniform::DataPtr_u  mData;

public:
  SGlUniform_GL_Rnr(SGlUniform* u);
  virtual ~SGlUniform_GL_Rnr();

  void alloc();
  void dealloc();

  void parse_and_apply(const TString& vals);

}; // endclass SGlUniform_GL_Rnr

#endif
