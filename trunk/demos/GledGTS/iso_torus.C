// $Id$

// A simple scene demonstrating GTSTorus.

// vars: ZQueen* g_queen
// libs: Geom1 GTS
#include <glass_defines.h>
#include <gl_defines.h>

void iso_torus()
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

  starw->GetGlobLamps()->Add(l);

  // GTS models

  CREATE_ADD_GLASS(surf, GTSTorus, starw, "GTSTorus", 0);
  surf->SetColor(1,0.8,0.2);
  surf->SetRM(4);
  surf->SetRm(1.5);
  surf->SetStep(0.05);

  CREATE_ADD_GLASS(maker, GTSIsoMaker, starw, "Iso Maker", 0);
  maker->SetTarget(surf);
  maker->SetFunctor(surf);

  CREATE_ADD_GLASS(retring, GTSRetriangulator, starw, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  retring->SetTarget(surf);

  maker->MakeSurface();
  surf->CalcStats();

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  g_shell->SpawnClassView(surf);
  g_shell->SpawnClassView(maker);
  g_shell->SpawnClassView(retring);
}
