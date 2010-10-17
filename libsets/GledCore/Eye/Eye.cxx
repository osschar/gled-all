// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Eye.h"

#include <Gled/GledNS.h>
#include <Glasses/ZGod.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/EyeInfo.h>
#include <Gled/Gled.h>
#include <Ephra/Saturn.h>
#include "Eye/Ray.h"

#include <TSocket.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>

namespace OS = OptoStructs;

/**************************************************************************/

void Eye::EyeFdMonitor(int fd, void* arg) { ((Eye*)arg)->Manage(fd); }

/**************************************************************************/

Eye::Eye(TSocket* sock, EyeInfo* ei) :
  mSatSocket   (sock),
  mSatSocketFd (sock->GetDescriptor()),
  mMaxManageLoops  (9999),
  bBreakManageLoop (false),
  fCurrentRay (0),
  fAlphaImg   (0),
  fBetaImg    (0),
  fGammaImg   (0)
{
  static const Exc_t _eh("Eye::Eye ");

  if (ei->hEye != 0) throw _eh + "Eye already instantiated!\n";

  try
  {
    Saturn::HandleClientSideSaturnHandshake(mSatSocket);
  }
  catch(Exc_t& exc)
  {
    throw _eh + exc;
  }
  // No protocol exchange ...
  {
    TMessage* m;
    try
    {
      m = Saturn::HandleClientSideMeeConnection(mSatSocket, ei);
    }
    catch(Exc_t& exc)
    {
      throw _eh + exc;
    }
    size_t ss;  *m >> ss;
    mSaturn     = (Saturn*) ss;
    mSaturnInfo = mSaturn->GetSaturnInfo();
    ID_t ei_id; *m >> ei_id;
    delete m;
    mEyeInfo = dynamic_cast<EyeInfo*>(mSaturn->DemangleID(ei_id));
    if (mEyeInfo == 0)
    {
      throw _eh + "bad eye_info.";
    }
  }

  mEyeInfo->hEye = this;

  ISdebug(0, GForm("%screation of Eye('%s') complete", _eh.Data(), mEyeInfo->GetName()));
}

Eye::~Eye()
{
  // unregister from all queens
  // cleanup ray queue
  // reset eye-infe hEye
  // !!!! Send sth impressive to Saturn
  if (mSatSocket) {
    mSatSocket->Close();
    delete mSatSocket;
  }
  // Cleanup own shit ... like all Views (they are all A_View!)
}

/**************************************************************************/

OS::ZGlassImg* Eye::DemanglePtr(ZGlass* lens)
{
  if (lens == 0) return 0;

  hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(lens);
  if (i != mGlass2ImgHash.end()) return i->second;

  mpQueen2Int_i qci = mQueenLensCount.find(lens->GetQueen());
  if (qci == mQueenLensCount.end())
  {
    mQueenLensCount.insert(make_pair(lens->GetQueen(), 1));
    lens->GetQueen()->AddObserver(mEyeInfo);
  }
  else
  {
    ++qci->second;
  }

  lens->IncEyeRefCount();
  OS::ZGlassImg *gi = new OS::ZGlassImg(this, lens);
  mGlass2ImgHash .insert(make_pair(lens, gi));
  mZeroRefCntImgs.insert(gi);

  return gi;
}

ZGlass* Eye::DemangleID2Lens(ID_t id)
{
  OS::ZGlassImg* img = DemangleID(id);
  return img ? img->fLens : 0;
}

OS::ZGlassImg* Eye::DemangleID(ID_t id)
{
  return DemanglePtr(mSaturn->DemangleID(id));
}

//------------------------------------------------------------------------------

void Eye::ZeroRefCountImage(OptoStructs::ZGlassImg* img)
{
  mZeroRefCntImgs.insert(img);
}

//------------------------------------------------------------------------------

void Eye::RemoveImage(OS::ZGlassImg* img, bool wipe_zrc_set)
{
  static const Exc_t _eh("Eye::RemoveImage ");

  hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(img->fLens);

  assert (i != mGlass2ImgHash.end());
  assert (i->second == img);

  if (wipe_zrc_set)
    mZeroRefCntImgs.erase(img);
  // Prevent further additions into zero-ref-count set.
  img->IncRefCount();

  for (lpImgConsumer_i c = mImgConsumers.begin(); c != mImgConsumers.end(); ++c)
  {
    (*c)->ImageDeath(img);
  }

  i->first->DecEyeRefCount();

  mpQueen2Int_i qci = mQueenLensCount.find(i->first->GetQueen());
  if (--qci->second <= 0)
  {
    i->first->GetQueen()->RemoveObserver(mEyeInfo);
    mQueenLensCount.erase(qci);
  }

  mGlass2ImgHash.erase(i);
  delete img;
}

void Eye::ProcessZeroRefCntImgs()
{
  spZGlassImg_i i = mZeroRefCntImgs.begin();
  while (i != mZeroRefCntImgs.end())
  {
    spZGlassImg_i j = i++;
    if ((*j)->HasZeroRefCount())
    {
      RemoveImage(*j, false);
    }
    mZeroRefCntImgs.erase(j);
  }
}

//==============================================================================

Int_t Eye::GetImageCount(ZQueen* q)
{
  mpQueen2Int_i qci = mQueenLensCount.find(q);
  if (qci != mQueenLensCount.end())
    return qci->second;
  return 0;
}

Int_t Eye::PrintObservedLenses(ZQueen* q, bool dump_views)
{
  Int_t cnt = 0;
  for (hpZGlass2pZGlassImg_i i = mGlass2ImgHash.begin(); i != mGlass2ImgHash.end(); ++i)
  {
    if (i->first->GetQueen() == q)
    {
      printf("%3d. %s, N_views=%d, N_links=%u\n", ++cnt,
	     i->first->Identify().Data(), (Int_t) i->second->fViews.size(), i->second->fRefCount);
      if (dump_views)
      {
	Int_t vcnt = 0;
	for (OS::lpA_View_i j = i->second->fViews.begin(); j != i->second->fViews.end(); ++j)
	{
	  printf("    %3d. %s\n", ++vcnt, typeid(**j).name());
	}
      }
    }
  }
  return cnt;
}


//==============================================================================
// Current ray
//==============================================================================

OptoStructs::ZGlassImg* Eye::GetCurrentBetaImg()
{
  if (fBetaImg == 0 && fCurrentRay->HasBeta())
  {
    fBetaImg = DemanglePtr(fCurrentRay->fBeta);
  }
  return fBetaImg;
}

OptoStructs::ZGlassImg* Eye::GetCurrentGammaImg()
{
  if (fGammaImg == 0 && fCurrentRay->HasGamma())
  {
    fGammaImg = DemanglePtr(fCurrentRay->fGamma);
  }
  return fGammaImg;
}

/**************************************************************************/
// SatSocket bussines
/**************************************************************************/

Int_t Eye::Manage(int fd)
{
  static const Exc_t _eh("Eye::Manage ");

  Int_t ray_count = 0, all_count = 0;

  RayNS::SaturnToEyeEnvelope see;

  while (true)
  {
    ssize_t len = recv(mSatSocketFd, &see, sizeof(RayNS::SaturnToEyeEnvelope),
		       MSG_DONTWAIT);
    if (len < 0)
    {
      if (errno == EWOULDBLOCK)
	break;

      ISerr(_eh + "recv error: " + strerror(errno));
      break;
    }
    else if (len == 0)
    {
      ISerr(_eh + "Saturn closed connection ... unregistring fd handler.");
      UninstallFdHandler();
      // !!! destroy eye
      return -2;
    }

    ++all_count;

    switch (see.fType)
    {
      case RayNS::MT_TextMessage:
      {
	TextMessage& tm = * see.fTextMessage;
	// printf("Got message from <%p,%s> %s '%s'\n", tm.fCaller, tm.fCaller ? tm.fCaller->GetName() : "<none>",
	//        ISnames[tm.fType], tm.fMessage.Data());

	Message(GForm("[%s] %s", tm.fCaller->GetName(), tm.fMessage.Data()), tm.fType);
	delete see.fTextMessage;
	break;
      }

      case RayNS::MT_Ray:
      {
	fCurrentRay = see.fRay;

	hpZGlass2pZGlassImg_i alpha_it = mGlass2ImgHash.find(fCurrentRay->fAlpha);
	if (alpha_it != mGlass2ImgHash.end())
	{
	  Ray &ray = *fCurrentRay;
	  ++ray_count;

	  OS::ZGlassImg* a = fAlphaImg = alpha_it->second;

	  {
	    GLensReadHolder(a->fLens);

	    a->PreAbsorption(ray);

	    for (OS::lpA_View_i i=a->fViews.begin(); i!=a->fViews.end(); )
	    {
	      // Be careful! Views can come and go in response to Rays.
	      OS::A_View* v = *i; --i;
	      v->AbsorbRay(ray);
	      ++i; if(*i == v) ++i;
	    }

	    a->PostAbsorption(ray);
	  }

	  if (ray.fRQN == RayNS::RQN_death)
	  {
	    RemoveImage(a, true);
	  }
	}

	fCurrentRay->DecRefCnt();
	fCurrentRay = 0;
	fAlphaImg = fBetaImg = fGammaImg = 0;
	break;
      } // end case MT_Ray

      case RayNS::MT_EyeCommand:
      {
	// None sent ... none handled
	ISerr(_eh + "Got EyeCommand message, handling not implemented!");
	delete see.fEyeCommand;
	break;
      }
    } // end switch message->What()

    if (bBreakManageLoop)
    {
      bBreakManageLoop = false;
      // printf("Eye::Manage breaking loop on request ...\n");
      break;
    }

    if (all_count > mMaxManageLoops)
    {
      break;
    }
  } // end while (true)

  ISdebug(6, GForm("%s got %d message(s), %d ray(s) in this gulp",
		   _eh.Data(), all_count, ray_count));

  PostManage(ray_count);

  ProcessZeroRefCntImgs();

  return all_count;
}

/**************************************************************************/

void Eye::Send(TMessage* m)
{
  mSatSocket->Send(*m);
}

void Eye::Send(ZMIR& mir)
{
  mir.WriteHeader();
  mSatSocket->Send(mir);
}

/**************************************************************************/
/**************************************************************************/

void Eye::CloseEye()
{}
