// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// GuiPupil
//

#include "GuiPupil.h"
#include "Pupil.h"

#include <Glasses/GuiPupilInfo.h>
#include <Glasses/PupilInfo.h>

#include <GledView/GledViewNS.h>
#include <GledView/FTW_Shell.h>
#include <GledView/FltkGledStuff.h>

namespace OS   = OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace FGS  = FltkGledStuff;

/**************************************************************************/
// creator foo
/**************************************************************************/

GuiPupil* GuiPupil::Create_GuiPupil(FTW_Shell* sh, OS::ZGlassImg* img)
{
  static const Exc_t _eh("GuiPupil::Create_GuiPupil ");

  GuiPupilInfo* ni = dynamic_cast<GuiPupilInfo*>(img->fLens);
  if(ni == 0) throw(_eh + "user-data is not GuiPupilInfo.");
  if(ni->GetPupil() == 0) throw(_eh + "link 'Pupil' must be set.");

  GuiPupil* guipupil = new GuiPupil(sh, img);

  sh->adopt_window(guipupil);

  return guipupil;
}

void *SubShellCreator_GledCore_GuiPupil = (void*)GuiPupil::Create_GuiPupil;

/**************************************************************************/
/**************************************************************************/

namespace
{
  /**************************************************************************/
  // Configs
  /**************************************************************************/

  // int def_W = 80;
  // int def_H = 16;

  int min_W = 30;
  int max_W = 240;
  int min_H = 16;
  int max_H = 120;
}

/**************************************************************************/
/**************************************************************************/

GuiPupil::GuiPupil(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int w, int h) :
  FTW_SubShell(sh, this),
  OS::A_View(img),
  Fl_Window(w, h)
{
  mInfo = dynamic_cast<GuiPupilInfo*>(fImg->fLens);
  assert(mInfo);

  OS::ZGlassImg* pupil_img = mShell->DemanglePtr(mInfo->GetPupil());

  wMainPack = new Fl_Pack(0, 0, w, h);
  wMainPack->type(FL_VERTICAL);

  // Menu group
  {
    wMenuPack = new Fl_Pack(0, 0, w, 2);
    wMenuPack->type(FL_HORIZONTAL);

    FGS::LensChoiceMenuBox* view_sel = new FGS::LensChoiceMenuBox
      (pupil_img, 0, 0, 4, 2, "View");
    view_sel->SetSrcImg(fImg);
    view_sel->SetSrcLinkName("Cameras");
    view_sel->SetSrcFid(CameraInfo::FID());
    view_sel->SetMethodInfo(pupil_img->GetCI()->
			    FindMethodInfo("ImportCameraInfo", true));
    view_sel->box((Fl_Boxtype)GVNS::menubar_box);

    FGS::LensRepNameBox* self = new FGS::LensRepNameBox(fImg, 0, 0, 220, 2);
    self->box((Fl_Boxtype)GVNS::menubar_box);
    self->color(fl_rgb_color(220,200,200));

    wMenuPack->end();
    wMenuPack->resizable(0);
  }

  wMainPack->end();
  end();

  wPupil = new Pupil(mShell, pupil_img, 0, 0, w, h-2);

  wPupil->show();
  wPupil->hide();
  wMainPack->add(wPupil);

  wMainPack->resizable(wPupil);
  resizable(wMainPack);
  swm_size_range = new SWM_Size_Range(min_W, min_H, max_W, max_H);
  label_window();
}

GuiPupil::~GuiPupil()
{}

/**************************************************************************/
/**************************************************************************/

void GuiPupil::AbsorbRay(Ray& ray)
{
  using namespace RayNS;

  if(ray.IsBasic()) {
    label_window();
    return;
  }

  if(ray.fFID != GuiPupilInfo::FID())
    return;

  // Handle GuiPupil's rays.
}

/**************************************************************************/
/**************************************************************************/

void GuiPupil::label_window(const char* l)
{
  if(l == 0) l = GForm("guipupil: %s '%s'", mInfo->GetName(), mInfo->GetTitle());
  FTW_SubShell::label_window(l);
}

int GuiPupil::handle(int ev)
{
  switch (ev) {

  case FL_SHOW: wPupil->show(); break;
  case FL_HIDE: wPupil->hide(); break;

  case FL_KEYBOARD: {
    if(wPupil->handle(ev)) return 1;
    break;
  }

  }

  return Fl_Window::handle(ev);
}

/**************************************************************************/
