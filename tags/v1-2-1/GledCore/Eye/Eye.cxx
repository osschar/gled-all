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

namespace OS = OptoStructs;

/**************************************************************************/

namespace {
  void EyeFdMonitor(int fd, void *arg) { ((Eye*)arg)->Manage(fd); }

  // void CloseEye_cb(Fl_Button* b, Eye* e)  { e->CloseEye(); }
}

/**************************************************************************/

Eye::Eye(UInt_t port, ID_t shell_id, const char* name, const char* title,
	 const Fl_SWM_Manager* swm_copy) : mMsg(0)
{
  static string _eh("Eye::Connect ");

  mSatSocket = new TSocket("localhost", port);
  if(!mSatSocket->IsValid()) {
    delete mSatSocket;
    throw(_eh + GForm("opening socket to %localhost:%d failed.", port));
  }

  { // Receive greeting
    int ml; char buf[256];
    ml = mSatSocket->RecvRaw(buf, 255, kDontBlock);
    if(ml <= 0) {
      delete mSatSocket;
      throw(_eh + GForm("handshake failed; len=%d", ml));
    }
    buf[ml] = 0;
    ISmess(_eh + buf);
  }
  // No protocol exchange ... 
  { // Send the desired EyeInfo
    TMessage m(GledNS::MT_EyeConnect);
    EyeInfo ei(name, title);
    GledNS::StreamGlass(m, &ei);
    mSatSocket->Send(m);
  }
  {
    TMessage *m;
    Int_t ml = mSatSocket->Recv(m);
    if(ml == -1 || m->What() != kMESS_ANY) {
      ISerr(GForm("Eye::Connect Recv of Saturn* failed; len=%d", ml));
      goto fail;
    }
    UInt_t ss;  *m >> ss; mSaturn = (Saturn*)ss;
    ID_t ei_id; *m >> ei_id;
    mEyeInfo = dynamic_cast<EyeInfo*>(mSaturn->DemangleID(ei_id));
    if(mEyeInfo == 0) {
      throw(string("Eye::Eye bad eye_info"));
    }
    delete m;
    ISdebug(0, GForm("Eye::Connect got (Saturn*)%p %s", mSaturn, mSaturn->GetName()));
    // Install fd handler
    mSatSocket->SetOption(kNoBlock, 1);
    Fl::add_fd(mSatSocket->GetDescriptor(), EyeFdMonitor, this);
  }

  {
    ShellInfo* si = dynamic_cast<ShellInfo*>(mSaturn->DemangleID(shell_id));
    if(si == 0) {
      throw(string("Eye::Eye bad shell_info"));
    }
    mShell = new FTW_Shell(DemanglePtr(si), swm_copy);
  }

  return;
 fail:
  delete mSatSocket;
}

Eye::~Eye() {
  // !!!! Send sth impressive to Saturn  
  Fl::remove_fd(mSatSocket->GetDescriptor());
  // close, delete suncket !!!!
  // Tell gled you passing away
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

Int_t Eye::Connect(UInt_t port)
{

  return 0;
}

Int_t Eye::Manage(int fd)
{
  TMessage *m;
  int count=0, len;
  while((len = mSatSocket->Recv(m)) != -4) { // -4 means "would block"

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

    mRay.Streamer(*m);
    // cout << mRay << endl;

    if(mRay.fEvent == Ray::RQN_message) {
      mShell->Message(GForm("[%s] %s", mRay.fCaller->GetName(), mRay.fMessage.Data()),
		      FTW_Shell::MT_std);
      continue;
    }
    if(mRay.fEvent == Ray::RQN_error) {
      mShell->Message(GForm("[%s] %s", mRay.fCaller->GetName(), mRay.fMessage.Data()),
		      FTW_Shell::MT_err);
      continue;
    }

    OS::hpZGlass2pZGlassImg_i alpha_it = mGlass2ImgHash.find(mRay.fAlpha);
    if(alpha_it == mGlass2ImgHash.end()) {
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
    count++;
  }

  for(lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
    (*w)->redraw();

  ISdebug(9, GForm("Eye::Manage got %d Ray(s) in this gulp", count));
  return count;
}

void Eye::Send(TMessage* m) { mSatSocket->Send(*m); }

void Eye::Send(ZMIR& mir) {
  // !!!! Could set Caller ID before sending !!!!
  // but then ... Saturn does that for MIRs from Eyes ... security or sth
  mSatSocket->Send(*mir.Message);
}

/**************************************************************************/
/**************************************************************************/

void Eye::CloseEye()
{

}
