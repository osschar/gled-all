// $Id$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FBO.h"

#include <TMath.h>

#include <GL/glew.h>

//______________________________________________________________________
// FBO
//
// Cut'n'pasted from examples in:
//   http://www.opengl.org/registry/specs/EXT/framebuffer_object.txt

Bool_t FBO::sRescaleToPow2 = true; // For ATI.

FBO::FBO() :
  fFrameBuffer  (0),
  fColorTexture (0),
  fDepthBuffer  (0),
  fW (-1),
  fH (-1)
{}

FBO::~FBO()
{
  release();
}

void FBO::init(int w, int h)
{
  static const Exc_t _eh("FBO::init ");

  fIsRescaled = false;
  if (sRescaleToPow2)
  {
    Int_t nw = 1 << TMath::CeilNint(TMath::Log2(w));
    Int_t nh = 1 << TMath::CeilNint(TMath::Log2(h));
    if (nw != w || nh != h)
    {
      fWScale = ((Float_t)w) / nw;
      fHScale = ((Float_t)h) / nh;
      w = nw; h = nh;
      fIsRescaled = true;
    }
  }

  if (fFrameBuffer != 0)
  {
    if (fW == w || fH == h)
      return;
    release();
  }

  fW = w; fH = h;

  glGenFramebuffersEXT (1, &fFrameBuffer);
  glGenTextures        (1, &fColorTexture);
  glGenRenderbuffersEXT(1, &fDepthBuffer);
  // glGenRenderbuffersEXT(1, &fStencilBuffer);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFrameBuffer);

  // initialize color texture
  glBindTexture(GL_TEXTURE_2D, fColorTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fW, fH, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D, fColorTexture, 0);

  // initialize depth renderbuffer
  glBindRenderbufferEXT   (GL_RENDERBUFFER_EXT, fDepthBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, fW, fH);

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                               GL_RENDERBUFFER_EXT, fDepthBuffer);

  /*
    // initialize stencil renderbuffer
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fStencilBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, fW, fH);

    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
    GL_RENDERBUFFER_EXT, fStencilBuffer);
  */

  //-------------------------

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

  glBindFramebufferEXT (GL_FRAMEBUFFER_EXT,  0);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // ? is needed
  glBindTexture        (GL_TEXTURE_2D,       0);

  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      printf("%sConstructed FBO ... all fine.\n", _eh.Data());
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      release();
      throw(_eh + "Constructed FBO not supported, choose different formats.");
      break;
    default:
      release();
      throw(_eh + "Constructed FBO is crap, fix code in FBO class.");
      break;
  }
}

void FBO::release()
{
  glDeleteFramebuffersEXT (1, &fFrameBuffer);
  glDeleteTextures        (1, &fColorTexture);
  glDeleteRenderbuffersEXT(1, &fDepthBuffer);
  //glDeleteRenderbuffersEXT(1, &fStencilBuffer);

  fColorTexture = fFrameBuffer = fDepthBuffer = 0;
  fW = fH = -1;
}

//==============================================================================

void FBO::bind()
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFrameBuffer);
}

void FBO::unbind()
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//==============================================================================

void FBO::bind_texture()
{
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture(GL_TEXTURE_2D, fColorTexture);
  glEnable(GL_TEXTURE_2D);

  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glScalef(fWScale, fHScale, 1);
  glMatrixMode(GL_MODELVIEW);
}

void FBO::unbind_texture()
{
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glPopAttrib();
}
