// $Id$

// A simple scene demonstrating GTSTorus.

// vars: ZQueen* g_queen
// libs: Geom1 GledGTS

#include <glass_defines.h>
#include <gl_defines.h>

class GTSTorus;
class GTSIsoMaker;
class GTSRetriangulator;

GTSTorus          *g_torus = 0;
GTSIsoMaker       *g_isomaker = 0;
GTSRetriangulator *g_retriangulator = 0;

Double_t def_step = 0.05;

void make_std();
void coarsen_std(Double_t fac=1.0, Bool_t print_stats=kTRUE);

void iso_torus()
{
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GledGTS");

  Gled::AssertMacro("sun_demos.C");

  CREATE_ADD_GLASS(scene, Scene, g_queen, "Iso Surfaces", "");
  g_scene = scene;

  // Geom elements

  CREATE_ADD_GLASS(base_plane, Rect, scene, "BasePlane", "");
  base_plane->SetUnitSquare(16);

  CREATE_ADD_GLASS(lamp, Lamp, scene, "Lamp", "");
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->SetPos(-2, -6, 6);
  lamp->RotateLF(1,2, TMath::Pi());

  scene->GetGlobLamps()->Add(lamp);

  // GTS models

  CREATE_ADD_GLASS(surf, GTSTorus, scene, "GTSTorus", 0);
  surf->SetColor(1,0.8,0.2);
  surf->SetRM(4);
  surf->SetRm(1.5);
  g_torus = surf;

  CREATE_ADD_GLASS(maker, GTSIsoMaker, scene, "Iso Maker", 0);
  maker->SetTarget(surf);
  maker->SetFunctor(surf);
  g_isomaker = maker;

  CREATE_ADD_GLASS(retring, GTSRetriangulator, scene, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  retring->SetTarget(surf);
  g_retriangulator = retring;

  make_std();

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  g_shell->SpawnClassView(surf);
  g_shell->SpawnClassView(maker);
  g_shell->SpawnClassView(retring);
}

void make_std()
{
  g_torus->SetStep(def_step);
  g_isomaker->MakeSurface();
  g_isomaker->MovePointsOntoIsoSurface();
  g_torus->CalcStats();
}

void coarsen_std(Double_t fac, Bool_t print_stats)
{
  g_retriangulator->SetStopOpts(GTSRetriangulator::SO_Cost);

  g_retriangulator->SetStopCost(def_step * fac);
  g_retriangulator->SetCostOpts(GTSRetriangulator::CO_Length);
  g_retriangulator->Coarsen();
  g_isomaker->MovePointsOntoIsoSurface();

  if (print_stats) { g_torus->CalcStats(); g_torus->PrintStats(); }

  g_retriangulator->SetStopCost(0.5);
  g_retriangulator->SetCostOpts(GTSRetriangulator::CO_Angle);
  g_retriangulator->Coarsen();
  g_isomaker->MovePointsOntoIsoSurface();

  if (print_stats) { g_torus->CalcStats(); g_torus->PrintStats(); }
}
