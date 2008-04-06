// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Operator_H
#define Gled_Operator_H

#include <Glasses/ZList.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
#include <Gled/GTime.h>
class Eventor;
#include <Gled/GledMirDefs.h>

class Operator : public ZList {

public:

  struct Arg {
    Eventor*	fEventor;

    bool	fMultix;
    bool	fSignalSafe;
    bool	fContinuous;
    bool	fUseDynCast;

    GCondition	fSuspendidor;	// Suspender for *not* signal-safe evtors
    GMutex	fSignalodor;	// Lock for signal-safe evtors

    GTime	fStart,     fStop;
    GTime	fBeatStart, fBeatStop;
    GTime	fBeatSum;

    Int_t	fBeatID;

    // could have execution stack ??

    Arg() {}
    virtual ~Arg() {}
  };

  enum Exc_e { OE_Done, OE_Continue, OE_Wait, OE_Stop, OE_Break };

  struct Exception : public Exc_t {
    Operator*	fSource;
    Exc_e	fExc;

    Exception(Operator* o, Exc_e e, const TString& m) :
      Exc_t(m), fSource(o), fExc(e) {}
    Exception(Operator* o, Exc_e e, const char* m) :
      Exc_t(m), fSource(o), fExc(e) {}
    virtual ~Exception() throw() {}
  };

private:
  void _init();

protected:
  Bool_t	bOpActive;	//  X{GS} 7 Bool(-join=>1)
  Bool_t	bOpRecurse;	//  X{GS} 7 Bool()

public:
  Operator(const Text_t* n="Operator", const Text_t* t=0) :
    ZList(n,t) { _init(); }

  // virtuals
  virtual void ResetRecursively();

  virtual void PreOperate(Operator::Arg* op_arg)  throw(Operator::Exception);
  virtual void Operate(Operator::Arg* op_arg)     throw(Operator::Exception);
  virtual void PostOperate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "Operator.h7"
  ClassDef(Operator, 1)
}; // endclass Operator



#define OP_EXE_OR_SP_MIR(_lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR(_lens_, _method_, __VA_ARGS__) }}

#define OP_EXE_OR_SP_MIR_SATURN(_sat_, _lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR_SATURN(_sat_, _lens_, _method_, __VA_ARGS__) }}


#endif
