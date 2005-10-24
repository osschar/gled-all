// $Header$

// Simple setup for GL speed tests.

// vars: ZQueen* g_queen
// libs: Geom1

// 3.Feb.2005 Measurements on PIV-3.2, ATI radeon 9600 (w/ fglrx driver):
// 1k points ~ 23mus
// 1k trings ~ 70mus [2.0ms w/ translate, 2.6ms w/ push/pop matrix and translate]
//
// 1k GetFloatv  MODELVIEW_MATRIX ~ 158mus
// 1k GetDoublev MODELVIEW_MATRIX ~ 182mus

#include <glass_defines.h>
#include <gl_defines.h>

void gl_tests()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);

  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(25);
  g_queen->CheckIn(base_plane);
  images->Add(base_plane);
  base_plane->SetColor(0.1, 0.4, 0.1);

  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetLampScale(1);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  g_queen->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);

  CREATE_ADD_GLASS(pointmod, ZGlBlending, images, "Blending", 0);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, images, "Light model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(1);

  CREATE_ADD_GLASS(g1, GLTesterOne, images, "GlOne", 0);
  g1->SetPos(0, 0, 2);

  // Spawn GUI
  g_scene = images;
  Gled::Macro("eye.C");

  // Set pupil so that GLTesterOne can send redraw requests.
  g1->SetPupil(g_pupil);

  // Open a canvas.
  // Creation of canvases from a non-cint thread is dangerous.
  TCanvas* canvas = new TCanvas();
}
