// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlSource_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <AL/alut.h>
#include <GL/glew.h>

/**************************************************************************/

//void AlSource_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void AlSource_GL_Rnr::Draw(RnrDriver* rd)
{
  if (mAlSource->mAlSrc)
  {
    ZTrans* tp = 0;
    bool deletep = false;
    switch (mAlSource->mLocationType)
    {
      case AlSource::LT_FollowParents:
	tp = &rd->ToGCS();
	break;
      case AlSource::LT_CamDelta:
	tp = new ZTrans(*rd->GetCamAbsTrans());
	tp->MultRight(mAlSource->RefTrans());
	deletep = true;
	break;
    }
    ZTrans& t = *tp;
    alSource3f(mAlSource->mAlSrc, AL_POSITION, t(1,4), t(2,4), t(3,4));
    // printf("Source: %f %f %f\n", t(1,4), t(2,4), t(3,4));
    alSource3f(mAlSource->mAlSrc, AL_DIRECTION, t(1,1), t(2,1), t(3,1));

    if (deletep)
      delete tp;
  }
}

//void AlSource_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
