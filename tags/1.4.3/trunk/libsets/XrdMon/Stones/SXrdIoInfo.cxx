// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdIoInfo.h"

//==============================================================================
// SXrdIoInfo
//==============================================================================

//______________________________________________________________________________
//
//

void SXrdIoInfo::Dump(Int_t level)
{
  printf("SXrdIoInfo::Dump() N_reqs = %zu, N_vec_read_subreqs = %zu, N_errors = %d\n",
	 mReqs.size(), mOffsetVec.size(), mNErrors);

  if (level <= 0) return;

  int cnt = 0;
  for (vSXrdReq_i i = mReqs.begin(); i != mReqs.end(); ++i, ++cnt)
  {
    printf("%4d  %-7s ", cnt, i->TypeName());
    switch (i->Type())
    {
      case SXrdReq::R_Write:
      {
	printf("t=%-5d l=%-9d o=%lld\n", i->Time(), -i->Length(), i->Offset());
	break;
      }
      case SXrdReq::R_Read:
      {
	printf("t=%-5d l=%-9d o=%lld\n", i->Time(),  i->Length(), i->Offset());
	break;
      }
      case SXrdReq::R_VecRead:
      {
	Int_t sr_idx = i->SubReqIndex();

	printf("t=%-5d l=%-9d n_req_cnt=%-5hu n_reqs_lost=%-5hu idx=%d\n",
	       i->Time(), i->Length(), i->SubReqCount(), i->SubReqsLost(),
	       sr_idx);

	if (level > 1 && sr_idx >= 0)
	{
	  Int_t max = sr_idx + i->SubReqsStored();
	  Int_t j   = 0;
	  for (Int_t si = i->SubReqIndex(); si < max; ++si, ++j)
	  {
	    printf("             i=%-5d l=%-9d o=%lld\n", j, mLengthVec[si], mOffsetVec[si]);
	  }
	}
	break;
      }
    }
  }
}

//==============================================================================
// SXrdIoReq
//==============================================================================

//______________________________________________________________________________
//
//
