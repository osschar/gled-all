// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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

OptoStructs::ZGlassImg* Eye::DemanglePtr(ZGlass* glass)
{
  OS::hpZGlass2pZGlassImg_i i = mGlass2ImgHash.find(glass);
  if(i != mGlass2ImgHash.end()) return i->second;
  OS::ZGlassImg* gi = new OptoStructs::ZGlassImg(this, glass);
  mGlass2ImgHash[glass] = gi;
  return gi;
}

/**************************************************************************/
// SatSocket bussines
/**************************************************************************/

Int_t Eye::Manage(int fd)
{
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
      perror("Eye::Manage Prefetch got error that is not EWOULDBLOCK");
      break;
    }

    m = 0;
    len = mSatSocket->Recv(m);

    if(len == -1) {
      ISerr("Eye::Manage: Recv error");
      delete m; return -1;
    }

    if(len == 0) {
      ISerr("Eye::Manage Saturn closed connection ... unregistring");
      Fl::remove_fd(mSatSocket->GetDescriptor());
      delete m; return -2;
    }

    if(m->What() == kMESS_STRING) {
      TString str;
      *m >> str;
      ISmess(GForm("Eye::Manage got message string: %s", str.Data()));
      delete m;
      continue;
    }

    if(!m || m->What() != GledNS::MT_Ray) {
      ISerr(GForm("Eye::Manage: Recv failed or non MT_Ray' len=%d,what=%d. "
		  "Dis-synchronization possible.", len, m->What()));
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

    bool invalidate_rnrs = false, invalidate_link_parent_rnrs = false;

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
      // alpha; call treereps
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_LinkChange(mRay.fLibID, mRay.fClassID);
      }
      invalidate_rnrs = true;
      break;
    }
    case Ray::RQN_list_add: {
      // alpha zlist, beta newcomer, gamma before
      // call treereps, pupils
      assert(b);
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListAdd(b, g);
      }
      invalidate_rnrs = true;
      invalidate_link_parent_rnrs = true;
      /*
	for(lpPupil_i i=a->fPupils.begin(); i!=a->fPupils.end(); ++i) {
	(*i)->redraw();
	}
      */
      break;
    }
    case Ray::RQN_list_remove: {
      // alpha zlist, beta departee
      // call treereps, pupils
      assert(b);
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListRemove(b);
      }
      invalidate_rnrs = true;
      invalidate_link_parent_rnrs = true;
      break;
    }
    case Ray::RQN_list_rebuild: {
      // alpha zlist
      // call treereps, pupils
      // !! should check if some views drop to null (or one) !!!
      // !!!! wait a sec ... no destruction mechanism for Img, DefView
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i) {
	(*i)->Absorb_ListRebuild();
      }
      invalidate_rnrs = true;
      invalidate_link_parent_rnrs = true;
      break;
    }
    case Ray::RQN_birth: {
      // alpha ... demangle from Saturn
      // !!! call birth_hook if defed
      break;
    }
    case Ray::RQN_death: {
      // alpha the dustbiter; wipe/repair treereps, mixers and pupils
      
      // Assert all views closed
      // !!!!
      break;
    }

    case Ray::RQN_apocalypse: {
      // server shutting down
      break;
    }
    default: {
      assert(0);
    }
    } // end switch

    // Invalidate alpha's renderer
    if(invalidate_rnrs) {
      for(OS::lpA_View_i i=a->fFullViews.begin(); i!=a->fFullViews.end(); ++i)
	(*i)->InvalidateRnrScheme();
    }
    if(invalidate_link_parent_rnrs) {
      for(OS::lpA_View_i i=a->fLinkViews.begin(); i!=a->fLinkViews.end(); ++i)
	(*i)->InvalidateRnrScheme();
    }

    delete m;
  }

  for(lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
    (*w)->redraw();

  ISdebug(6, GForm("Eye::Manage got %d Ray(s) in this gulp", count));
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
