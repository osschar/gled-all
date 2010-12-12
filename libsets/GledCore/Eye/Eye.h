// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Eye_H
#define GledCore_Eye_H

#include <Glasses/ZGlass.h>
#include "OptoStructs.h"

class ZQueen;
class SaturnInfo;
class EyeInfo;
class ShellInfo;
class ZMIR;
class Ray;
class TextMessage;

class FTW_Shell;
class Fl_SWM_Manager;
class Fl_Window;

class TSocket;
class TMessage;


class Eye
{
protected:
#ifndef __CINT__
  typedef hash_map<ZGlass*, OptoStructs::ZGlassImg*> hpZGlass2pZGlassImg_t;
  typedef hpZGlass2pZGlassImg_t::iterator            hpZGlass2pZGlassImg_i;

  typedef hash_set<OptoStructs::ZGlassImg*>          spZGlassImg_t;
  typedef spZGlassImg_t::iterator                    spZGlassImg_i;
#endif

  typedef list<OptoStructs::ImageConsumer*>          lpImgConsumer_t;
  typedef lpImgConsumer_t::iterator                  lpImgConsumer_i;

  typedef map<ZQueen*, Int_t>                        mpQueen2Int_t;
  typedef mpQueen2Int_t::iterator                    mpQueen2Int_i;

  hpZGlass2pZGlassImg_t    mGlass2ImgHash;
  spZGlassImg_t            mZeroRefCntImgs;
  lpImgConsumer_t          mImgConsumers;
  mpQueen2Int_t            mQueenLensCount;

  Saturn*	mSaturn;	// X{g}
  SaturnInfo*	mSaturnInfo;	// X{g}
  EyeInfo*	mEyeInfo;	// X{g}

  TSocket*	mSatSocket;     // X{g}
  Int_t         mSatSocketFd;   // X{g}

  int           mMaxManageLoops;
  bool          bBreakManageLoop;

  virtual void RemoveImage(OptoStructs::ZGlassImg* img, bool wipe_zrc_set);
  virtual void ProcessZeroRefCntImgs();

  // Current ray being processed. Eyes are expected to be single threaded!
  Ray                    *fCurrentRay;
  OptoStructs::ZGlassImg *fAlphaImg;
  OptoStructs::ZGlassImg *fBetaImg;
  OptoStructs::ZGlassImg *fGammaImg;

public:
  Eye(TSocket* sock, EyeInfo* ei);
  virtual ~Eye();

  virtual void OpenEye();
  virtual void CloseEye();

  void DestroyViews();
  void DrainRayQueue();

  virtual void InstallFdHandler()   = 0;
  virtual void UninstallFdHandler() = 0;
  virtual void PostManage(int ray_count) = 0;
  virtual void Message(const TString& msg, InfoStream_e t=ISmessage) = 0;

  virtual void AbsorbEyeInfoRay(Ray& ray);

  // Basic ZGlassImg functionality
  virtual OptoStructs::ZGlassImg* DemanglePtr(ZGlass* lens);
  virtual OptoStructs::ZGlassImg* DemangleID(ID_t id);
  virtual ZGlass*                 DemangleID2Lens(ID_t id);

  virtual void ZeroRefCountImage(OptoStructs::ZGlassImg* img);

  Int_t GetImageCount(ZQueen* q);
  Int_t PrintObservedLenses(ZQueen* q, Bool_t dump_views=false);

  void RegisterImageConsumer(OptoStructs::ImageConsumer* imgc)   { mImgConsumers.push_back(imgc); }
  void UnregisterImageConsumer(OptoStructs::ImageConsumer* imgc) { mImgConsumers.remove(imgc); }

  // Current ray
  Ray*                    GetCurrentRay()      { return fCurrentRay; }
  OptoStructs::ZGlassImg* GetCurrentAlphaImg() { return fAlphaImg; }
  OptoStructs::ZGlassImg* GetCurrentBetaImg();
  OptoStructs::ZGlassImg* GetCurrentGammaImg();

  // Socketing
  Int_t	Manage(int fd);

  void Send(TMessage* m);
  void Send(ZMIR& c);

  void BreakManageLoop() { bBreakManageLoop = true; }

  static void EyeFdMonitor(int fd, void* arg);

#include "Eye.h7"
}; // endclass Eye

#endif
