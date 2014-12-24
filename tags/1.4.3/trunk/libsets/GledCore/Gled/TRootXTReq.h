// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_TRootXTReq_H
#define GledCore_TRootXTReq_H

class GMutex;
class GCondition;
class GThread;

#include <TString.h>

class TTimer;
class TSignalHandler;

#include <list>

class TRootXTReq
{
private:
  GCondition               *m_return_condition;

  static list<TRootXTReq*>  sQueue;
  static GThread           *sRootThread;
  static GMutex            *sQueueMutex;
  static bool               sSheduled;

  virtual void Act() = 0;

protected:
  TString                   mName;

  void post_request();

public:
  TRootXTReq(const char* n="TRootXTReq");
  virtual ~TRootXTReq();

  void ShootRequest();
  void ShootRequestAndWait();

  // --- Static interface ---

  static void Bootstrap(GThread* root_thread);
  static void Shutdown();

  static void ProcessQueue();

#include "TRootXTReq.h7"
  ClassDef(TRootXTReq, 1);
}; // endclass TRootXTReq

#endif
