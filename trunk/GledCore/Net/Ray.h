// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Ray_H
#define Gled_Ray_H

// Includes
#include <Gled/GledTypes.h>
#include <TString.h>

// for birth: creation request ID ??

struct Ray {
  enum RayQN_e { RQN_change=0,
		 RQN_link_change,
		 RQN_list_add, RQN_list_remove, RQN_list_rebuild,
		 RQN_birth, RQN_death,
		 RQN_message, RQN_error,
		 RQN_apocalypse
  };

  RayQN_e	fEvent;		// for Streaming casted into unsigned char
  TimeStamp_t	fStamp;
  ZGlass*	fCaller;
  ZGlass*	fAlpha;
  ZGlass*	fBeta;
  ZGlass*	fGamma;
  LID_t		fLibID;
  CID_t		fClassID;
  TString	fMessage;

  Ray() {}

  Ray(RayQN_e e, TimeStamp_t t, ZGlass* a, ZGlass* b=0, ZGlass* g=0) :
    fEvent(e), fStamp(t), fAlpha(a), fBeta(b), fGamma(g), fLibID(0), fClassID(0) {}

  Ray(RayQN_e e, TimeStamp_t t, ZGlass* a, LID_t lid, CID_t cid, ZGlass* b=0, ZGlass* g=0) :
    fEvent(e), fStamp(t), fAlpha(a), fBeta(b), fGamma(g), fLibID(lid), fClassID(cid) {}

  Ray(RayQN_e e, ZGlass* c, const Text_t* msg) :
    fEvent(e), fStamp(0), fCaller(c), fMessage(msg) {}

  Ray(RayQN_e e, ZGlass* c, TString& msg) :
    fEvent(e), fStamp(0), fCaller(c), fMessage(msg) {}

  void ListAdd(ZGlass* a, ZGlass* b, ZGlass* g)
  { fEvent=RQN_list_add; fAlpha=a; fBeta=b; fGamma=g; }
  void ListRemove(ZGlass* a, ZGlass* b)
  { fEvent=RQN_list_remove; fAlpha=a; fBeta=b; }
  void ListRebuild(ZGlass* a)
  { fEvent=RQN_list_rebuild; fAlpha=a; }

  bool IsBasic() { return ( (fLibID==0 && fClassID==0) || (fLibID==1 && fClassID==1) ); }

  void Streamer(TBuffer &buf);
  void PutSLC(TBuffer &buf);
  void GetSLC(TBuffer &buf);

  const char* EventName() const;
  void Dump(ostream& s) const;
};

ostream& operator<<(ostream& s, const Ray& n);

#endif
