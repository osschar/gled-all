// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Operator_H
#define Gled_Operator_H

#include <Glasses/ZList.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
class Eventor;

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

    // could have execution stack ??

    Arg() {}
    virtual ~Arg() {}
  };

  enum Exc_e { OE_Done, OE_Continue, OE_Wait, OE_Stop, OE_Break };

  struct Exception {
    Operator*	fSource;
    Exc_e	fExc;
    string	fMessage;

    Exception(Operator* o, Exc_e e, const string& m) :
      fSource(o), fExc(e), fMessage(m) {}
    Exception(Operator* o, Exc_e e, const char* m) :
      fSource(o), fExc(e), fMessage(m) {}
  };

private:
  void _init();

protected:
  bool		bOpActive;	//  X{GS} 7 Bool()
  bool		bOpRecurse;	//  X{GS} 7 Bool()

public:
  Operator(const Text_t* n="Operator", const Text_t* t=0) :
    ZList(n,t) { _init(); }

  // virtuals
  virtual void PreOperate(Arg* op_arg)	throw(Exception) {}
  virtual void Operate(Arg* op_arg)	throw(Exception);
  virtual void PostOperate(Arg* op_arg)	throw(Exception);

#include "Operator.h7"
  ClassDef(Operator, 1)
}; // endclass Operator

GlassIODef(Operator);


#define OP_EXE_OR_SP_MIR(_lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR(_lens_, _method_, __VA_ARGS__) }}

#define OP_EXE_OR_SP_MIR_SATURN(_sat_, _lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR_SATURN(_sat_, _lens_, _method_, __VA_ARGS__) }}


#endif
