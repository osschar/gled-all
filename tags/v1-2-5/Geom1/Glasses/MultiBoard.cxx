// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MultiBoard
//
//

#include "MultiBoard.h"
#include "MultiBoard.c7"
#include <Glasses/ZQueen.h>

ClassImp(MultiBoard)

/**************************************************************************/

void MultiBoard::_init()
{
  // !!!! Set all links to 0 !!!!
  mSlides = 0;
}

void MultiBoard::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mSlides == 0) {
    ZHashList* l = new ZHashList("Slides", GForm("Slides of MultiBoard %s", GetName()));
    l->SetElementFID(ZImage::FID());
    mQueen->CheckIn(l);
    SetSlides(l);
  }
}

/**************************************************************************/

void MultiBoard::First()
{
  ZImage* i = dynamic_cast<ZImage*>(mSlides->First());
  if(i == 0) return;
  SetTexture(i);
}

void MultiBoard::Last()
{
  ZImage* i = dynamic_cast<ZImage*>(mSlides->Last());
  if(i == 0) return;
  SetTexture(i);
}

void MultiBoard::Prev()
{
  ZImage* i = dynamic_cast<ZImage*>(mSlides->Before(mTexture));
  if(i == 0) First();
  else	     SetTexture(i);
}

void MultiBoard::Next()
{
  ZImage* i = dynamic_cast<ZImage*>(mSlides->After(mTexture));
  if(i == 0) Last();
  else	     SetTexture(i);
}
