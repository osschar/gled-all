// $Header$

// simple scene with some GTS models; more available from http://gts.sf.net/

// vars: ZQueen* scenes
// libs: Geom1 GTS
#include <glass_defines.h>
#include <gl_defines.h>

void iso_surfs()
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GTS");

  Scene* starw  = new Scene("Iso Surfaces");
  scenes->CheckIn(starw);
  scenes->Add(starw);

  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  scenes->CheckIn(base_plane);
  starw->Add(base_plane);

  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->Set3Pos(-10, 0, 0);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  scenes->CheckIn(l); starw->Add(l);
  starw->GetGlobLamps()->Add(l);
  ZNode* n = new Sphere(0.5, "Sph");
  scenes->CheckIn(n); l->Add(n);

  CREATE_ADD_GLASS(maker, GTSIsoMaker, starw, "Iso Maker", 0);

  CREATE_ADD_GLASS(mat1,   ZGlMaterial, starw, "Material1", 0);
  CREATE_ADD_GLASS(blend1, ZGlBlending, starw, "Blending1", 0);
  CREATE_ADD_GLASS(limo1,  ZGlLightModel, starw, "LightModel1", 0);

  // GTS models

  CREATE_ADD_GLASS(surf1, GTSurf, starw, "Surf1", 0);
  maker->SetTarget(surf1);
  maker->SetFormula("x^2 + y^2 + z^2");
  maker->SetValue(0.6);

  maker->MakeSurface();

  { // Station rotator
    Eventor* e = new Eventor("Dynamo");
    e->SetBeatsToDo(-1); e->SetInterBeatMS(40); e->SetStampInterval(10);
    scenes->CheckIn(e); starw->Add(e);
    Mover* mv = new Mover("S1 Rotator");
    mv->SetNode(surf1); mv->SetRi(1); mv->SetRj(2); mv->SetRa(0.005);
    scenes->CheckIn(mv); e->Add(mv);
  }
  // e->Start();
}
