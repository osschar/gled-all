// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
#include <Ephra/Forest.h>
#include <Ephra/Mountain.h>

#include <TUnixSystem.h>

#include <FL/Fl.H>

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>

namespace OS = OptoStructs;

/**************************************************************************/

void Eye::EyeFdMonitor(int fd, void* arg) { ((Eye*)arg)->Manage(fd); }

/**************************************************************************/

Eye::Eye(TSocket* sock, EyeInfo* ei) : mSatSocket(sock)
{
  static const string _eh("Eye::Eye ");

  try {
    Saturn::HandleClientSideSaturnHandshake(mSatSocket);
  }
  catch(string exc) {
    throw(_eh + exc);
  }
  // No protocol exchange ...
  {
    TMessage* m;
    try {
      m = Saturn::HandleClientSideMeeConnection(mSatSocket, ei);
    }
    catch(string exc) {
      throw(_eh + exc);
    }
    UInt_t ss;  *m >> ss; mSaturn = (Saturn*)ss; mSaturnInfo = mSaturn->GetSaturnInfo();
    ID_t ei_id; *m >> ei_id;
    delete m;
    mEyeInfo = dynamic_cast<EyeInfo*>(mSaturn->DemangleID(ei_id));
    if(mEyeInfo == 0) {
      throw(_eh + "bad eye_info.");
    }
  }

  ISdebug(0, GForm("%screation of Eye('%s') complete", _eh.c_str(), mEyeInfo->GetName()));

}

Eye::~Eye() {
  // !!!! Send sth impressive to Saturn  
  if(mSatSocket) {
    mSatSocket->Close();
    delete mSatSocket;
  }
  // Cleanup own shit ... like all Views
}

/**************************************************************************/

OS::ZGlassImg* Eye::DemanglePtr(ZGlass* glass)
{
  if(glass == 0) return 0;
  OS::hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(glass);
  if(i != mGlass2ImgHash.end()) return i->second;
  OS::ZGlassImg* gi;
  {
    GLensReadHolder rlck(glass);
    glass->IncEyeRefCount();
    gi = new OS::ZGlassImg(this, glass);
  }
  mGlass2ImgHash[glass] = gi;
  return gi;
}

OS::ZGlassImg* Eye::DemangleID(ID_t id)
{
  return DemanglePtr(mSaturn->DemangleID(id));
}

void Eye::RemoveImage(OS::ZGlassImg* img)
{
  static const string _eh("Eye::RemoveImage ");

  OS::hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(img->fGlass);
  if(i == mGlass2ImgHash.end()) {
    cout << _eh + "lens of passed image not found in hash.\n";
    return;
  }
  if(i->second != img) {
    cout << _eh + " non-matching images.\n";
    return;
  }

  for(OS::lpImgConsumer_i c=mImgConsumers.begin(); c!=mImgConsumers.end(); ++c)
    (*c)->ImageDeath(img);

  {
    GLensReadHolder rlck(i->first);
    i->first->DecEyeRefCount();
  }
  mGlass2ImgHash.erase(i);
  delete img;
}

/**************************************************************************/
// SatSocket bussines
/**************************************************************************/

Int_t Eye::Manage(int fd)
{
  static const string _eh("Eye::Manage ");

  TMessage *m;
  UInt_t    length;
  Int_t     ray_count = 0, all_count = 0, len;

  while(1) {

    // Prefetch ...
    len = recv(mSatSocket->GetDescriptor(), &length, sizeof(UInt_t),
	       MSG_PEEK|MSG_DONTWAIT);
    if(len < 0) {
      if(errno == EWOULDBLOCK) {
	break;
      }
      ISerr(_eh + "prefetch got error that is not EWOULDBLOCK.");
      break;
    }

    m = 0;
    len = mSatSocket->Recv(m);

    if(len == -1) {
      ISerr(_eh + "Recv error.");
      delete m; return -1;
    }

    if(len == 0) {
      ISerr(_eh + "Saturn closed connection ... unregistring fd handler.");
      Fl::remove_fd(mSatSocket->GetDescriptor());
      delete m; return -2;
    }

    ++all_count;

    switch(m->What()) {

    case kMESS_STRING: {
      TString str;
      *m >> str;
      Message(GForm("Raw message: %s", str.Data()), MT_std);
      break;
    }

    case GledNS::MT_TextMessage: {
      TextMessage tm;
      tm.Streamer(*m);
      // printf("Got message from <%p,%s> %s '%s'\n", tm.fCaller, tm.fCaller ? tm.fCaller->GetName() : "<none>",
      //        tm.fType ? "error" : "message", tm.fMessage.Data());
      switch(tm.fType) {
      case TextMessage::TM_Message: {
	Message(GForm("[%s] %s", tm.fCaller->GetName(), tm.fMessage.Data()),
		MT_std);
	break;
      }
      case TextMessage::TM_Error: {
	Message(GForm("[%s] %s", tm.fCaller->GetName(), tm.fMessage.Data()),
		       MT_err);
	break;
      }
      default: {
	ISerr(_eh + "unknown TextMessage type");
	break;
      }
      } // end switch TextMessage type
      break;
    }

    case GledNS::MT_Ray: {
      Ray ray(*m);
      OS::hpZGlass2pZGlassImg_i alpha_it = mGlass2ImgHash.find(ray.fAlpha);
      if(alpha_it == mGlass2ImgHash.end()) {
	break;
      }
      ray.Read(*m);
      // cout << _eh << ray << endl;

      ++ray_count;
      OS::ZGlassImg* a = ray.fAlphaImg = alpha_it->second;
      ray.fBetaImg  = ray.HasBeta()  ? DemanglePtr(ray.fBeta)  : 0;
      ray.fGammaImg = ray.HasGamma() ? DemanglePtr(ray.fGamma) : 0;

      // Read-lock alpha
      GLensReadHolder(a->fGlass);
      
      a->PreAbsorption(ray);

      for(OS::lpA_View_i i=a->fViews.begin(); i!=a->fViews.end(); ) {
	// Be careful! Views can come and go in response to Rays.
	OS::A_View* v = *i; --i;
	v->AbsorbRay(ray);
	++i; if(*i == v) ++i;
      }

      a->PostAbsorption(ray);

      if(ray.fRQN==RayNS::RQN_death) {
	RemoveImage(a);
      }

    } // end case MT_Ray

    } // end switch message->What()
    delete m;

    if(bBreakManageLoop) {
      bBreakManageLoop = false;
      // printf("Eye::Manage breaking loop on request ...\n");
      break;
    }
  } // end while(1)

  ISdebug(6, GForm("%s got %d message(s), %d ray(s) in this gulp",
		   _eh.c_str(), all_count, ray_count));

  PostManage(ray_count);

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
