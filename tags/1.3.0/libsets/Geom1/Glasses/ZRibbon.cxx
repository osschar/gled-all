// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRibbon
//
// Interpolated color gradient.
// Internal storage could be optimized.
//
// Should merge with RGBAPalette.

#include "ZRibbon.h"
#include "ZRibbon.c7"
#include <TSystem.h>

ClassImp(ZRibbon);

/**************************************************************************/

void ZRibbon::_init()
{
  bLoadAdEnlight = false;
}

/**************************************************************************/

ZColor ZRibbon::MarkToCol(Float_t mark)
{
  if(mColMarks.empty()) return ZColor();
  list<ZColorMark>::iterator i = mColMarks.begin();
  if(mark < i->m()) return ZColor(mColMarks.front());
  ++i;
  while(i != mColMarks.end() && mark > i->m()) ++i;
  if(i == mColMarks.end()) return ZColor(mColMarks.back());
  list<ZColorMark>::iterator j = i;
  --j;
  if(j == mColMarks.end()) return ZColor(mColMarks.back());
  Float_t fi = (mark - j->m())/(i->m() - j->m());
  return fi * ZColor(*i) + (1 - fi) * ZColor(*j);
}

/**************************************************************************/

namespace {
  const char loadpov_filter[] =
    "perl -e 'while(<>) { next if /^#/; next if /^\\s*$/; s/^\\s*//; print; }'";
}

void ZRibbon::LoadPOV()
{
  FILE* pov = gSystem->OpenPipe(
    GForm("cpp %s | %s", mPOVFile.Data(), loadpov_filter), "r");
  if(!pov) {
    ISerr(GForm("ZRibbon::LoadPOV [%s] OpenPipe(%s) failed",
		GetName(), mPOVFile.Data()));
    return;
  }
  mColMarks.clear();
  char linebuff[128];
  while(fgets(linebuff, 128, pov)) {
    float x, r, g, b, t;
    int n = sscanf(linebuff, "[%f color rgbt <%f, %f, %f, %f>]",
		   &x, &r, &g, &b, &t);
    if(n != 5) continue;
    mColMarks.push_back(ZColorMark(x,r,g,b,1-t));

  }
  gSystem->ClosePipe(pov);
  printf("ZRibbon::LoadPOV loaded %zu marks from '%s'.\n",
	 mColMarks.size(), mPOVFile.Data());

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/
