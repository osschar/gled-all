// $Id$

// A simple scene demonstrating GTSIsoMaker.

// vars: ZQueen* g_queen
// libs: Geom1 GTS
#include <glass_defines.h>
#include <gl_defines.h>

void iso_surfs()
{
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GledGTS");

  Gled::AssertMacro("sun_demos.C");

  Scene* starw  = new Scene("Iso Surfaces");
  g_queen->CheckIn(starw);
  g_queen->Add(starw);
  g_scene = starw;

  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  g_queen->CheckIn(base_plane);
  starw->Add(base_plane);

  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetPos(-2, -6, 6);
  l->RotateLF(1,2, TMath::Pi());
  g_queen->CheckIn(l); starw->Add(l);

  // GTS models

  CREATE_ADD_GLASS(maker1, GTSIsoMaker, starw, "Iso Maker 1", 0);
  CREATE_ADD_GLASS(surf1, GTSurf, starw, "Clipped Sphere", 0);
  surf1->SetPos(3, 0, 0);
  surf1->SetColor(1,0.8,0.2);
  maker1->SetTarget(surf1);
  maker1->SetFormula("x^2 + y^2 + z^2");
  maker1->SetValue(1.2);
  maker1->MakeSurface();

  CREATE_ADD_GLASS(maker2, GTSIsoMaker, starw, "Iso Maker 2", 0);
  CREATE_ADD_GLASS(surf2, GTSurf, starw, "Torus", 0);
  surf2->SetRotByDegrees(0,30,-30);
  surf2->SetColor(0.2,1,0.8);
  maker2->SetTarget(surf2);
  maker2->SetFormula("(0.8 - sqrt(x^2 + y^2))^2 + z^2");
  maker2->SetValue(0.04);
  maker2->MakeSurface();

  CREATE_ADD_GLASS(mat,   ZGlMaterial, starw, "Material1", 0);
  CREATE_ADD_GLASS(blend, ZGlBlending, starw, "Blending1", 0);
  CREATE_ADD_GLASS(limo,  ZGlLightModel, starw, "LightModel1", 0);
  limo->SetLightModelOp(ZRnrModBase::O_On);
  limo->SetLiMoTwoSide(true);
  limo->SetShadeModelOp(ZRnrModBase::O_On);

  CREATE_ADD_GLASS(maker3, GTSIsoMaker, starw, "Iso Maker 3", 0);
  CREATE_ADD_GLASS(surf3, GTSurf, starw, "Hyperboloid", 0);
  surf3->SetPos(-3, 0, 0);
  surf3->SetRotByDegrees(0,-30,0);
  surf3->SetScale(1.5);
  surf3->SetUseScale(true);
  surf3->SetColor(0.8,0.2,1);
  maker3->SetTarget(surf3);
  maker3->SetFormula("(x^2+y^2) - z^2/0.75");
  maker3->SetValue(-0.05);
  maker3->MakeSurface();

  CREATE_ADD_GLASS(retring, GTSRetriangulator, starw, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  retring->SetTarget(surf2);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  { // Torus rotator
    Eventor* e = new Eventor("Dynamo");
    e->SetBeatsToDo(-1); e->SetInterBeatMS(40); e->SetStampInterval(10);
    g_queen->CheckIn(e); starw->Add(e);
    Mover* mv = new Mover("S1 Rotator");
    mv->SetNode(surf2); mv->SetRi(2); mv->SetRj(3); mv->SetRa(0.01);
    g_queen->CheckIn(mv); e->Add(mv);
    e->Start();
  }
}
