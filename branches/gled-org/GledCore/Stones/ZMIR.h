// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMIR_H
#define Gled_ZMIR_H

// Includes
#include <Gled/GledTypes.h>
class Saturn;
class SaturnInfo;
class TMessage;

/*
  TBuffer annotations:
  int Length()		current pos
  SetBufferOffset(int) 	set pos
  Message header 2*UInt_t: length + message type
 */

class ZMIR {
  // **** Custom Streamer ****

private:
  void _init();

public:
  enum Punct_e { P_Comma=1, P_Semicolon, P_Dot };
  enum Direction_e { D_Unknown=0, D_Up, D_Down };

  ID_t		CallerID;
  ID_t		RecipientID;
  ID_t		AlphaID;
  ID_t		BetaID;
  ID_t		GammaID;
  Int_t		UserFlags;
  ZGlass*	Caller;		//!
  SaturnInfo*	Recipient;	//!
  ZGlass*	Alpha;		//!
  ZGlass*	Beta;		//!
  ZGlass*	Gamma;		//!
  TMessage*	Message;	//!
  Direction_e	Direction;	//! 

  ZMIR() : CallerID(0), RecipientID(0), AlphaID(0), BetaID(0), GammaID(0),
	       Message(0), Direction(D_Unknown) { _init(); }
  ZMIR(ID_t a, ID_t b=0, ID_t g=0, ID_t c=0);
  ZMIR(TMessage* m);
  virtual ~ZMIR();

  // operator TMessage& () { return *Message; }
  // operator TMessage* () { return Message; }

  void Demangle(Saturn* s) throw(string);

  void Rewind();
  void RewindToMIR();
  void SetCaller(ZGlass* caller);
  void SetRecipient(SaturnInfo* recipient);
  void ClearRecipient();

  ClassDef(ZMIR, 1)
};

#endif
