// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TSPupilInfo_GL_Rnr.h"

#include <Eye/Eye.h>
#include <RnrBase/Fl_Event_Enums.h>

#include <GL/glew.h>

//==============================================================================

void TSPupilInfo_GL_Rnr::_init()
{}

TSPupilInfo_GL_Rnr::TSPupilInfo_GL_Rnr(TSPupilInfo* idol) :
  PupilInfo_GL_Rnr(idol),
  mTSPupilInfo(idol)
{
  _init();
}

TSPupilInfo_GL_Rnr::~TSPupilInfo_GL_Rnr()
{}

//==============================================================================

int TSPupilInfo_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // Handle event -- just copy some data over to the other side.
  // Always return 0 so that the event will be processed further.

  TSPupilInfo& TPI = * mTSPupilInfo;

  if (ev.fEvent == FL_KEYDOWN && ev.fKey == FL_Escape)
  {
    auto_ptr<ZMIR> m(TPI.S_SetCurrentSpiritio(0));
    fImg->fEye->Send(*m);
    return 1;
  }

  return PupilInfo_GL_Rnr::Handle(rd, ev);
}

//==============================================================================

// void TSPupilInfo_GL_Rnr::PreDraw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::Draw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::PostDraw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::Render(RnrDriver* rd) {}
