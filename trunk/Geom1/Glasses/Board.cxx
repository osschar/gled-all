// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Board.h"
//#include <Glasses/ZImage.h>

#include "FL/gl.h"
#include "GL/glu.h"

#include <stdio.h>

ClassImp(Board)

/**************************************************************************/

void Board::_init()
{
  mTexture = 0;
  mULen = mVLen = 1;
  mColor.rgba(1,0,0);
  pData = 0;
}

// Use sth like
// convert -antialias -crop 440x500+80+96 -geom 512x1024! a.ps a.rgb
// to get picture out of Story

Board::~Board() {
  delete [] pData;
}

/**************************************************************************/

Int_t Board::ReadFile()
{
  /*
  FILE* in = fopen(mFile.Data(), "r");
  if(!in) return 1;
  // Should wipe gl texture ...
  if(pData) delete [] pData;
  pData = new GLubyte[mW*mH*3];
  if(!pData) return 2;
  size_t ri = fread(pData, 3, mW*mH, in);
  fclose(in);
  if( ri != mW*mH ) {
    delete [] pData; pData = 0;
    return 3;
  }
  cout << "Board::ReadFile "<< mFile.Data() << ":\n";
  for(Int_t i=1;i<=12;i++)
    printf("%hu ", pData[mW*mH*3-i]);
  printf("\n");
  */
  return 0;
}

#include "Board.c7"
