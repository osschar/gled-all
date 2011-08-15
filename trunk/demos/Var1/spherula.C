// $Id$

// A simple scene demonstrating GTSTorus.

// vars: ZQueen* g_queen
// libs: Geom1 GledGTS

#include <glass_defines.h>
#include <gl_defines.h>

class GTSurf;
class LegendreCoefs;

GTSurf            *gtsurf      = 0;
LegendreCoefs     *legendcoefs = 0;

void make_std();

void spherula()
{
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GledGTS");

  Gled::AssertMacro("sun_demos.C");

  CREATE_ADD_GLASS(scene, Scene, g_queen, "Spherula Scene", "");
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

  ASSIGN_ADD_GLASS(gtsurf, GTSurf, scene, "GTSurf", 0);
  gtsurf->SetColor(1,0.8,0.2);

  ASSIGN_ADD_GLASS(legendcoefs, LegendreCoefs, scene, "LegendreCoefs", 0);
  legendcoefs->ReadEgmFile("EGM-2008.data", 1000);

  make_std();

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  g_shell->SpawnClassView(gtsurf);
  g_shell->SpawnClassView(legendcoefs);
}

void make_std()
{
  gtsurf->GenerateSphere(7);
  // gtsurf->Rescale(10);
}

void make_rnd(Int_t max_l=30, Double_t abs_scale=0.1, Double_t pow_scale=1.5)
{
  gtsurf->GenerateSphereThetaConst(7);
  gtsurf->LegendrofyScaleRandomMulti(max_l, abs_scale, pow_scale);
}

void make_leg1()
{
  gtsurf->GenerateSphere(7);
  gtsurf->LegendrofyAdd(legendcoefs, 1e-5, 200);
  // gtsurf->Rescale(10);
}

void make_leg2()
{
  gtsurf->GenerateSphere(7);
  gtsurf->LegendrofyMulti(legendcoefs, 1e-5, 400);
  // gtsurf->Rescale(10);
}
