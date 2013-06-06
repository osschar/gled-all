// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlFont_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <GL/glew.h>

// Use ftgl-2.1.2 shipped with root. In 2.1.3 one should include "FTGL/ftgl.h"
#include "FTFont.h"
#include "FTGLExtrdFont.h"
#include "FTGLOutlineFont.h"
#include "FTGLPolygonFont.h"
#include "FTGLTextureFont.h"
#include "FTGLPixmapFont.h"
#include "FTGLBitmapFont.h"

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void ZRlFont_GL_Rnr::_init()
{
  mFont = 0;
  LoadFont();
}

ZRlFont_GL_Rnr::~ZRlFont_GL_Rnr()
{
  delete mFont;
}

/**************************************************************************/

void ZRlFont_GL_Rnr::AbsorbRay(Ray& ray)
{
  static const Exc_t _eh("ZRlFont_GL_Rnr::AbsorbRay ");

  if (ray.fFID == ZRlFont::FID())
  {
    switch (ray.fRQN)
    {
      case ZRlFont::PRQN_font_change:
	if (LoadFont() == false)
	  throw _eh + "loading of font '" + mZRlFont->mFontFile + "' failed.";
	break;
      case ZRlFont::PRQN_size_change:
	if (mFont)
	  mFont->FaceSize(mZRlFont->mSize);
	break;
      case ZRlFont::PRQN_depth_change:
	if (mFont)
	  mFont->Depth(mZRlFont->mDepthFac * mZRlFont->mSize);
	break;
    }
  }
}

/**************************************************************************/

void ZRlFont_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(mFont) {
    PARENT::PreDraw(rd);
    update_tring_stamp(rd);
    rd->PushRnrMod(ZRlFont::FID(), mRnrMod);
  }
}

void ZRlFont_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mFont) {
    update_tring_stamp(rd);
    rd->SetDefRnrMod(ZRlFont::FID(), mRnrMod);
  }
}

void ZRlFont_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(mFont) {
    rd->PopRnrMod(ZRlFont::FID());
    PARENT::PostDraw(rd);
  }
}

/**************************************************************************/

bool ZRlFont_GL_Rnr::LoadFont()
{
  static const Exc_t _eh("ZRlFont_GL_Rnr::LoadFont ");

  if (mFont)
  {
    delete mFont;
    mFont = 0;
  }

  ZRlFont& x = *mZRlFont;
  if (x.mFontFile.IsNull())
    return false;

  switch (x.mMode)
  {
    case ZRlFont::FM_Bitmap:
      mFont = new FTGLBitmapFont(x.mFontFile);
      break;
    case ZRlFont::FM_Pixmap:
      mFont = new FTGLPixmapFont(x.mFontFile);
      break;
    case ZRlFont::FM_Texture:
      mFont = new FTGLTextureFont(x.mFontFile);
      break;
    case ZRlFont::FM_Outline:
      mFont = new FTGLOutlineFont(x.mFontFile);
      break;
    case ZRlFont::FM_Polygon:
      mFont = new FTGLPolygonFont(x.mFontFile);
      break;
    case ZRlFont::FM_Extrude:
      mFont = new FTGLExtrdFont(x.mFontFile);
      mFont->Depth(x.mDepthFac * x.mSize);
      break;
    default:
      ISerr(_eh + "invalid FTGL type.");
      return false;
      break;
  }
  if (mFont->Error())
  {
    ISerr(_eh + "failed loading font from '" + x.mFontFile + "'.");
    delete mFont;
    mFont = 0;
    return false;
  }
  mFont->UseDisplayList(false); 
  mFont->FaceSize(x.mSize);
  return true;
}
