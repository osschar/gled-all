// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Eye.h"

#include <Gled/GledNS.h>
#include <Glasses/ZGod.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/EyeInfo.h>
#include <Glasses/NestInfo.h>
#include <Glasses/ShellInfo.h>
#include <Gled/Gled.h>
#include <Ephra/Saturn.h>
#include <Ephra/Forest.h>
#include <Ephra/Mountain.h>

#include "FTW_Shell.h"

#include <FL/Fl_SWM.H>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <TUnixSystem.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <errno.h>

namespace OS = OptoStructs;

/**************************************************************************/

namespace {
  void EyeFdMonitor(int fd, void *arg) { ((Eye*)arg)->Manage(fd); }

  // void CloseEye_cb(Fl_Button* b, Eye* e)  { e->CloseEye(); }
}

/**************************************************************************/

Eye::Eye(UInt_t port, TString identity, ID_t shell_id,
	 const char* name, const char* title,
	 const Fl_SWM_Manager* swm_copy)
{
  static const string _eh("Eye::Eye ");

  mSatSocket = new TSocket("localhost", port);
  try {
    Saturn::HandleClientSideSaturnHandshake(mSatSocket);
  }
  catch(string exc) {
    ISerr(_eh + exc);
    goto fail;
  }
  // No protocol exchange ...
  {
    TMessage* m;
    try {
      EyeInfo ei(name, title);
      ei.SetLogin(identity);
      m = Saturn::HandleClientSideMeeConnection(mSatSocket, &ei);
    }
    catch(string exc) {
      ISerr(_eh + exc);
      goto fail;
    }
    UInt_t ss;  *m >> ss; mSaturn = (Saturn*)ss; mSaturnInfo = mSaturn->GetSaturnInfo();
    ID_t ei_id; *m >> ei_id;
    delete m;
    mEyeInfo = dynamic_cast<EyeInfo*>(mSaturn->DemangleID(ei_id));
    if(mEyeInfo == 0) {
      ISerr(_eh + "bad eye_info");
      goto fail;
    }
  }
  {
    ShellInfo* si = dynamic_cast<ShellInfo*>(mSaturn->DemangleID(shell_id));
    if(si == 0) {
      ISerr(_eh + "bad shell_info");
      goto fail;
    }
    mShell = new FTW_Shell(DemanglePtr(si), swm_copy);
  }

  // Install fd handler
  Fl::add_fd(mSatSocket->GetDescriptor(), EyeFdMonitor, this);
  //mSatSocket->SetOption(kNoBlock, 1);

  ISdebug(0, GForm("%screation of Eye('%s') complete", _eh.c_str(), name));

  return;

 fail:
  delete mSatSocket;
  throw(_eh + "creation failed");
}

Eye::~Eye() {
  if(mSatSocket) {
    // !!!! Send sth impressive to Saturn  
    Fl::remove_fd(mSatSocket->GetDescriptor());
    // close, delete suncket !!!!
  }
  // Cleanup own shit ... like all Views
}

void Eye::show() { mShell->show(); }

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
  OS::hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(img->fGlass);
  if(i == mGlass2ImgHash.end()) {
    cout <<"Eye::RemoveImage not in hash\n";
    return;
  }
  if(i->second != img) {
    cout <<"Eye::RemoveImage non-matching images\n";
    return;
  }
  {
    GLensReadHolder rlck(i->first);
    i->first->DecEyeRefCount();
  }
  mGlass2ImgHash.erase(i);
  delete img;
}

/**************************************************************************/

void Eye::InvalidateRnrs(OS::ZGlassImg* img)
{
  for(OS::lpA_View_i i=img->fFullViews.begin(); i!=img->fFullViews.end(); ++i)
    (*i)->InvalidateRnrScheme();
  for(OS::lpA_View_i i=img->fLinkViews.begin(); i!=img->fLinkViews.end(); ++i)
    (*i)->InvalidateRnrScheme();
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
      mShell->Message(GForm("Raw message: %s", str.Data()), FTW_Shell::MT_std);
      break;
    }

    case GledNS::MT_TextMessage: {
      TextMessage tm;
      tm.Streamer(*m);
      // printf("Got message from <%p,%s> %s '%s'\n", tm.fCaller, tm.fCaller ? tm.fCaller->GetName() : "<none>",
      //        tm.fType ? "error" : "message", tm.fMessage.Data());
      switch(tm.fType) {
      case TextMessage::TM_Message: {
	mShell->Message(GForm("[%s] %s", tm.fCaller->GetName(), tm.fMessage.Data()),
			FTW_Shell::MT_std);
	break;
      }
      case TextMessage::TM_Error: {
	mShell->Message(GForm("[%s] %s", tm.fCaller->GetName(), tm.fMessage.Data()),
			FTW_Shell::MT_err);
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
      // cout << mRay << endl;

      ++ray_count;
      OS::ZGlassImg* a = ray.fAlphaImg = alpha_it->second;
      ray.fBetaImg  = ray.HasBeta()  ? DemanglePtr(ray.fBeta)  : 0;
      ray.fGammaImg = ray.HasGamma() ? DemanglePtr(ray.fGamma) : 0;

      // Read-lock alpha
      GLensReadHolder(a->fGlass);
      
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->AbsorbRay(ray);
      }

      if(ray.IsBasicChange() || ray.fRQN==RayNS::RQN_death) {
	// The death part of 'if' should not be relevant until the
	// local links are implemented.
	for(OS::lpA_View_i i=a->fLinkViews.begin(); i!=a->fLinkViews.end(); ++i) {
	  (*i)->AbsorbRay(ray);
	}
      }

      if(ray.fEyeBits & Ray::EB_StructuralChange) {
	InvalidateRnrs(a);
      }

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

  if(ray_count) {
    for(lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
      (*w)->redraw();
  }

  ISdebug(6, GForm("%s got %d message(s), %d ray(s) in this gulp",
		   _eh.c_str(), all_count, ray_count));
  return all_count;
}

void Eye::Send(TMessage* m) { mSatSocket->Send(*m); }

void Eye::Send(ZMIR& mir) {
  mir.WriteHeader();
  mSatSocket->Send(mir);
}

/**************************************************************************/
/**************************************************************************/

void Eye::CloseEye()
{}
