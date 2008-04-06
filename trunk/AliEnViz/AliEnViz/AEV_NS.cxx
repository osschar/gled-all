// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEV_NS.h"

#include <stdlib.h>
#include <fcntl.h>

const int AEV_NS::BiDiPipe::sRBSize = 256;

/**************************************************************************/

void AEV_NS::BiDiPipe::init(const string& base) {
  static const Exc_t _eh("AEV_NS::BiDiPipe::init ");

  fDir = GForm("/tmp/%s_XXXXXX", base.c_str());
  if(mkdtemp((char*)fDir.c_str()) == 0) {
    throw(_eh + "mkdtemp failed.");
  }

  fAtoBName = GForm("%s/AtoB", fDir.c_str());
  mknod(fAtoBName.c_str(), 0666 | S_IFIFO, 0);

  fBtoAName = GForm("%s/BtoA", fDir.c_str());
  mknod(fBtoAName.c_str(), 0666 | S_IFIFO, 0);
}

void AEV_NS::BiDiPipe::open_BtoA() {
  fBtoA = fopen(fBtoAName.c_str(), "r");
}

void AEV_NS::BiDiPipe::open_AtoB() {
  fAtoB = fopen(fAtoBName.c_str(), "w");
  setlinebuf(fAtoB);
}

void AEV_NS::BiDiPipe::close_fifos() {
  if(fAtoB) { fclose(fAtoB); fAtoB = 0; }
  if(fBtoA) { fclose(fBtoA); fBtoA = 0; }
}

/**************************************************************************/

int AEV_NS::BiDiPipe::send_command(const char* cmd)
{
  // Returns 0 on error, positive int on success (num of records).

  if(bDebug) {
    printf("BiDiPipe send_command: %s\n", cmd);
  }

  reset_error();
  fputs(cmd, fAtoB); fputc(10, fAtoB);
  read_line();
  if(strncmp(fRB, "ERR", 3) == 0) {
    bError = true;
    fError = read_string();
    return -1;
  } else {
    return read_int();
  }
}

/**************************************************************************/

const char* AEV_NS::BiDiPipe::read_line()
{
  fgets(fRB, sRBSize, fBtoA);
  int s = strlen(fRB);
  // chomp
  if(s && fRB[s-1] == 10) fRB[s-1] = 0;
  if(bDebug) {
    printf("BiDiPipe read_line: %s\n", fRB);
  }
  return fRB;
}

string AEV_NS::BiDiPipe::read_string()
{
  read_line();
  return string(fRB);
}

/**************************************************************************/

int AEV_NS::BiDiPipe::read_int()
{
  read_line();
  return (int)strtol(fRB, 0, 0);
}

double AEV_NS::BiDiPipe::read_double()
{
  read_line();
  return strtod(fRB, 0);
}

/**************************************************************************/
