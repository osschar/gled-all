// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEV_NS_H
#define AliEnViz_AEV_NS_H

#include <Gled/GledTypes.h>

namespace AEV_NS
{

  struct BiDiPipe
  {
    TString fDir;
    TString fAtoBName;
    TString fBtoAName;
    FILE*   fAtoB;
    FILE*   fBtoA;

    char*   fRB;
    static const int sRBSize;

    bool    bError;
    TString fError;

    bool    bDebug;

    BiDiPipe() : fAtoB(0), fBtoA(0), bDebug(false)
    { fRB = new char[sRBSize]; reset_error(); }

    ~BiDiPipe()
    { delete [] fRB; }

    void init(const TString& base);
    void open_BtoA();
    void open_AtoB();
    void close_fifos();

    bool eof_BtoA();

    void reset_error() { bError = false; fError = "<no-error>"; }
    int  send_command(const TString& cmd);

    TString  read_line();
    TString  read_string();

    int    read_int();
    double read_double();
  };

}

#endif
