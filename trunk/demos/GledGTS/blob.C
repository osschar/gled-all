// $Id$

// A simple scene demonstrating GTSIsoMaker.

// vars: ZQueen* g_queen
// libs: Geom1 GTS
#include <glass_defines.h>
#include <gl_defines.h>

void blob()
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

  CREATE_ADD_GLASS(maker1, GTSIsoMaker, starw, "Iso Maker 1", 0);
  maker1->SetXAxis(-2, 2, 60);
  maker1->SetYAxis(-2, 2, 60);
  maker1->SetZAxis(-2, 2, 60);
  maker1->SetAlgo(GTSIsoMaker::A_Tetra);
  CREATE_ADD_GLASS(surf1, GTSurf, starw, "Clipped Sphere", 0);
  surf1->SetPos(0, 0, 0);
  surf1->SetColor(1,0.8,0.2);
  maker1->SetTarget(surf1);
  maker1->SetFormula("pow((x-1)^2 + y^2 + z^2, -1) + pow((x+1)^2 + y^2 + z^2, -1)");
  maker1->SetValue(1.);
  maker1->MakeSurface();

  CREATE_ADD_GLASS(retring, GTSRetriangulator, starw, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  retring->SetTarget(surf1);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  { // Torus rotator
    Eventor* e = new Eventor("Dynamo");
    e->SetBeatsToDo(-1); e->SetInterBeatMS(40); e->SetStampInterval(10);
    g_queen->CheckIn(e); starw->Add(e);
    Mover* mv = new Mover("S1 Rotator");
    g_queen->CheckIn(mv);
    mv->SetNode(surf1); mv->SetRi(2); mv->SetRj(3); mv->SetRa(0.01);
    e->Add(mv);
    //e->Start();
  }
}
