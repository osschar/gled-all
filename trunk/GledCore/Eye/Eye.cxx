// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Eye.h"
//#include <Eye/Pupil.h>
//#include <Eye/GLoodge.h>
#include "FTW_Leaf.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"

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
    UInt_t ss;  *m >> ss; mSaturn = (Saturn*)ss;
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
  OS::hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(glass);
  if(i != mGlass2ImgHash.end()) return i->second;
  OS::ZGlassImg* gi = new OptoStructs::ZGlassImg(this, glass);
  mGlass2ImgHash[glass] = gi;
  return gi;
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
  mGlass2ImgHash.erase(i);
  delete img;
}

/**************************************************************************/

void Eye::InvalidateRnrs(OS::ZGlassImg* img, bool invalidate_links_p)
{
  for(OS::lpA_View_i i=img->fFullViews.begin(); i!=img->fFullViews.end(); ++i)
    (*i)->InvalidateRnrScheme();
  if(invalidate_links_p) {
    for(OS::lpA_View_i i=img->fLinkViews.begin(); i!=img->fLinkViews.end(); ++i)
      (*i)->InvalidateRnrScheme();
  }
}

/**************************************************************************/
// SatSocket bussines
/**************************************************************************/

Int_t Eye::Manage(int fd)
{
  static const string _eh("Eye::Manage ");

  TMessage *m;
  UInt_t length;
  Int_t  count = 0, len;

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

    if(m->What() == kMESS_STRING) {
      TString str;
      *m >> str;
      ISmess(_eh + "got message string: " + str.Data());
      delete m;
      continue;
    }

    if(!m || m->What() != GledNS::MT_Ray) {
      ISerr(GForm("%s Recv failed or non MT_Ray' len=%d,what=%d. "
		  "Dis-synchronization possible.", _eh.c_str(), len, m->What()));
      delete m;
      return -3;
    }

    ++count;
    mRay.Streamer(*m);
    // cout << mRay << endl;

    if(mRay.fEvent == Ray::RQN_message) {
      mShell->Message(GForm("[%s] %s", mRay.fCaller->GetName(), mRay.fMessage.Data()),
		      FTW_Shell::MT_std);
      delete m;
      continue;
    }
    if(mRay.fEvent == Ray::RQN_error) {
      mShell->Message(GForm("[%s] %s", mRay.fCaller->GetName(), mRay.fMessage.Data()),
		      FTW_Shell::MT_err);
      delete m;
      continue;
    }

    OS::hpZGlass2pZGlassImg_i alpha_it = mGlass2ImgHash.find(mRay.fAlpha);
    if(alpha_it == mGlass2ImgHash.end()) {
      delete m;
      continue; // no image of alpha
    }

    OS::ZGlassImg* a = alpha_it->second;
    OS::ZGlassImg* b = mRay.fBeta  ? DemanglePtr(mRay.fBeta)  : 0;
    OS::ZGlassImg* g = mRay.fGamma ? DemanglePtr(mRay.fGamma) : 0;

    a->fGlass->ReadLock();
    switch(mRay.fEvent) {
    case Ray::RQN_change: {
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_Change(mRay.fLibID, mRay.fClassID);
      }
      if(mRay.IsBasic()) {
	for(OS::lpA_View_i i=a->fLinkViews.begin(); i!=a->fLinkViews.end(); ++i) {
	  (*i)->Absorb_Change(mRay.fLibID, mRay.fClassID);
	}
      }
      break;
    }
    case Ray::RQN_link_change: {
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_LinkChange(mRay.fLibID, mRay.fClassID);
      }
      InvalidateRnrs(a);
      break;
    }
    case Ray::RQN_list_add: {
      // alpha zlist, beta newcomer, gamma before
      // call treereps, pupils
      assert(b);
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListAdd(b, g);
      }
      InvalidateRnrs(a, true);
      break;
    }
    case Ray::RQN_list_remove: {
      // alpha zlist, beta departee
      // full-views
      assert(b);
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListRemove(b);
      }
      InvalidateRnrs(a, true);
      break;
    }
    case Ray::RQN_list_rebuild: {
      // alpha zlist
      // full-views
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListRebuild();
      }
      InvalidateRnrs(a, true);
      break;
    }
    case Ray::RQN_list_clear: {
      // alpha zlist
      // full-views
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListClear();
      }
      InvalidateRnrs(a, true);
      break;
    }
    case Ray::RQN_birth: {
      // This point never reached !!!!
      // Should check for this RQN before (if birth_callback enabled).
      break;
    }
    case Ray::RQN_death: {
      // alpha the dustbiter
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_Delete();
      }
      for(OS::lpA_View_i i=a->fLinkViews.begin(); i!=a->fLinkViews.end(); ++i) {
	(*i)->Absorb_Delete();
      }
      InvalidateRnrs(a, true);
      RemoveImage(a);
      break;
    }

    case Ray::RQN_apocalypse: {
      // server shutting down ... Saturn actually never sends this.
      break;
    }
    default: {
      assert(0);
    }
    } // end switch
    a->fGlass->ReadUnlock();

    delete m;
  }

  for(lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
    (*w)->redraw();

  ISdebug(6, GForm("%s got %d Ray(s) in this gulp", _eh.c_str(), count));
  return count;
}

void Eye::Send(TMessage* m) { mSatSocket->Send(*m); }

void Eye::Send(ZMIR& mir) {
  mir.WriteHeader();
  mSatSocket->Send(mir);
}

/**************************************************************************/
/**************************************************************************/

void Eye::CloseEye()
{

}
