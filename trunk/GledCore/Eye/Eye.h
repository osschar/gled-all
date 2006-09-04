// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

class FTW_Shell;
class Fl_SWM_Manager;
class Fl_Window;

class TSocket;
class TMessage;


class Eye {

protected:
  OptoStructs::hpZGlass2pZGlassImg_t mGlass2ImgHash;
  OptoStructs::lpImgConsumer_t       mImgConsumers;

  Saturn*	mSaturn;	// X{g}
  SaturnInfo*	mSaturnInfo;	// X{g}
  EyeInfo*	mEyeInfo;	// X{g}

  TSocket*	mSatSocket;

  bool		bBreakManageLoop;

public:
  Eye(TSocket* sock, EyeInfo* ei);
  virtual ~Eye();

  virtual void InstallFdHandler()   = 0;
  virtual void UninstallFdHandler() = 0;

  // Basic ZGlassImg functionality
  virtual OptoStructs::ZGlassImg* DemanglePtr(ZGlass* glass);
  virtual OptoStructs::ZGlassImg* DemangleID(ID_t id);
  virtual ZGlass*                 DemangleID2Lens(ID_t id);
  virtual void RemoveImage(OptoStructs::ZGlassImg* img);

  void RegisterImageConsumer(OptoStructs::ImageConsumer* imgc)
  { mImgConsumers.push_back(imgc); }
  void UnregisterImageConsumer(OptoStructs::ImageConsumer* imgc)
  { mImgConsumers.remove(imgc); }

  enum MType_e { MT_std=0, MT_err, MT_wrn, MT_msg };
  virtual void Message(const char* msg, MType_e t=MT_std) {}
  virtual void Message(const TString& msg, MType_e t=MT_std) {}

  // Socketing
  Int_t	Manage(int fd);
  virtual void PostManage(int ray_count) {}

  void Send(TMessage* m);
  void Send(ZMIR& c);

  void CloseEye();

  void BreakManageLoop() { bBreakManageLoop = true; }

  static void EyeFdMonitor(int fd, void* arg);

#include "Eye.h7"
}; // endclass Eye

#endif
