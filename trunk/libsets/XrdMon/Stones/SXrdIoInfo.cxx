// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdIoInfo.h"

// SXrdIoInfo

//______________________________________________________________________________
//
//

ClassImp(SXrdIoInfo);

//==============================================================================

void SXrdIoInfo::RegisterRead(Int_t time, Long64_t offset, Int_t length)
{

}

void SXrdIoInfo::RegisterWrite(Int_t time, Long64_t offset, Int_t length)
{

}

void SXrdIoInfo::RegisterReadV(Int_t time, Int_t n_segments, Int_t total_length)
{

}

void SXrdIoInfo::RegisterReadVSeg(Long64_t offset, Int_t length)
{

}

//==============================================================================
