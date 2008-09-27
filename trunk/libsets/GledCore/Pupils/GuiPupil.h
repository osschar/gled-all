// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GuiPupil_H
#define GledCore_GuiPupil_H

#include <Gled/GledTypes.h>
#include <Eye/OptoStructs.h>
#include <GledView/FTW_SubShell.h>
class Pupil;
class GuiPupilInfo;

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_SWM.H>

class GuiPupil : public FTW_SubShell,
                 public OptoStructs::A_View,
                 public Fl_Window, public Fl_SWM_Client
{
private:

protected:
  GuiPupilInfo*  mInfo;   // X{g}

  Fl_Pack*       wMainPack;
  Fl_Pack*       wMenuPack;

  Pupil*         wPupil; // X{g}

public:

  static GuiPupil* Create_GuiPupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img);

  GuiPupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int w=64, int h=30);
  GuiPupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int x, int y, int w, int h);
  ~GuiPupil();

  virtual void AbsorbRay(Ray& ray);

  virtual void label_window(const char* l=0);
  virtual int  handle(int ev);

#include "GuiPupil.h7"
}; // endclass GuiPupil

#endif
