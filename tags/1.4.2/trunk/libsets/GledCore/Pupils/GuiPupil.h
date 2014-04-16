// $Id$

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

#include <FL/Fl_Group.H>
class Fl_Pack;

class GuiPupil : public FTW_SubShell,
		 public OptoStructs::A_View,
                 public Fl_Group
{
private:

protected:
  GuiPupilInfo*  mInfo;   // X{g}

  Fl_Pack*       wMainPack;
  Fl_Pack*       wMenuPack;

  Pupil*         wPupil; // X{g}

public:

  static GuiPupil* Create_GuiPupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img);

  GuiPupil(FTW_Shell* sh, Fl_Window* win, OptoStructs::ZGlassImg* img, int w, int h);
  ~GuiPupil();

  virtual void AbsorbRay(Ray& ray);

  virtual void label_window(const char* l=0);
  virtual int  handle(int ev);

#include "GuiPupil.h7"
}; // endclass GuiPupil

#endif
