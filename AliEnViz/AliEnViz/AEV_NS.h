// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEV_NS_H
#define AliEnViz_AEV_NS_H

#include <Gled/GledTypes.h>
class ZList;
class ZImage;

namespace AEV_NS {

  struct BiDiPipe {
    string fDir;
    string fAtoBName;
    string fBtoAName;
    FILE*  fAtoB;
    FILE*  fBtoA;

    char*  fRB;
    static const int sRBSize;

    bool   bError;
    string fError;

    bool   bDebug;

    BiDiPipe() : fAtoB(0), fBtoA(0), bDebug(false)
    { fRB = new char[sRBSize]; reset_error(); }

    ~BiDiPipe()
    { delete [] fRB; }

    void init(const string& base);
    void open_BtoA();
    void open_AtoB();
    void close_fifos();

    void reset_error() { bError = false; fError = "<no-error>"; }
    int  send_command(const char* cmd);

    const char* read_line();
    string      read_string();

    int    read_int();
    double read_double();
  };

}

#endif
