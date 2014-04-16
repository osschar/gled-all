// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SGlUniform_GL_RNR_H
#define Geom1_SGlUniform_GL_RNR_H

#include <Stones/SGlUniform.h>

#include <GL/glew.h>

class RnrDriver;
class TPMERegexp;

class SGlUniform_GL_Rnr
{
protected:
  SGlUniform            *mUni;
  SGlUniform::DataPtr_u  mData;

  void alloc();
  void dealloc();
  void realloc();

  static TPMERegexp *s_valuesep_re;

public:
  SGlUniform_GL_Rnr(SGlUniform* u);
  SGlUniform_GL_Rnr(const SGlUniform_GL_Rnr &a);
  virtual ~SGlUniform_GL_Rnr();

  void parse(const TString& vals);
  void parse_defaults();

  void apply();

  static const char* unitype_to_name(GLenum t);
  static Int_t       unitype_to_size(GLenum t);
  static Bool_t      unitype_is_float(GLenum t);

}; // endclass SGlUniform_GL_Rnr

#endif
