// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FBO_H
#define GledCore_FBO_H

#include <Gled/GledTypes.h>

class FBO {
public:
  UInt_t  fFrameBuffer;
  UInt_t  fColorTexture;
  UInt_t  fDepthBuffer;
  // UInt_t  fStencilBuffer;
  Int_t   fW, fH;

public:
  FBO();
  ~FBO();

  void init(int w, int h);
  void release();

  void bind();
  void unbind();

  void bind_texture();
  void unbind_texture();

}; // endclass fbo

#endif
