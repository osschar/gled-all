// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZFireKing
//
//

#include "ZFireKing.h"
#include <Glasses/ZQueen.h>
#include "ZFireKing.c7"

ClassImp(ZFireKing)

/**************************************************************************/

void ZFireKing::_init()
{
  // !!!! Set all links to 0 !!!!
}

/**************************************************************************/

void ZFireKing::Enthrone(ZQueen* queen)
{
  PARENT_GLASS::Enthrone(queen);
  queen->SetAuthMode(ZQueen::AM_None);
}

/**************************************************************************/
// Mirroring of Queens
/**************************************************************************/

void ZFireKing::RequestQueenMirroring(ZQueen* queen_to_mirror)
{
  // Entry point for initiation of queen mirroring on user request.
  // Should be directed at FireKing, who performs the task
  // on behalf of Saturn.
  // (would be more proper to have MirrorQueen or MirrorPrincess in fire-space)

  static string _eh("ZFireKing::RequestQueenMirroring ");

  // Should assert queen exists etc ...

  ZKing* boss = queen_to_mirror->GetKing();
  if(boss->GetMinID() > mSaturn->GetKing()->GetMinID()) {
    throw(_eh + "queen should be in a moon object-space");
  }

  auto_ptr<ZMIR> mir(boss->S_reflect_queen(queen_to_mirror, mSaturn->GetSaturnInfo()));
  mir->SetRecipient(mSaturn->GetSaturnInfo());
  mSaturn->PostMIR(*mir);

  // assert queen in `above' space and not yet ruling, neither awaiting sceptre.
  // pass control to reflect_queen, which also checks dependencies and
  // generates appropriate beams for the master Saturn.
}

void ZFireKing::RequestQueenShattering(ZQueen* queen_to_leave)
{
  // Ignore gloriously. Deps first.

  static string _eh("ZFireKing::RequestQueenShattering ");

  ISerr(_eh + "not implemented.");
}

/**************************************************************************/
