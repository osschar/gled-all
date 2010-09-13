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

  CREATE_ADD_GLASS(blending, ZGlBlending, starw, "ZGlBlending", 0);
  blending->SetAntiAliasOp(1);
  blending->SetPointSize(16);

  // GTS models

  CREATE_ADD_GLASS(maker1, GTSIsoMaker, starw, "Iso Maker 1", 0);
  maker1->SetXAxis(-2, 2, 30);
  maker1->SetYAxis(-2, 2, 30);
  maker1->SetZAxis(-2, 2, 30);
  maker1->SetAlgo(GTSIsoMaker::A_Tetra);

  CREATE_ADD_GLASS(surf1, GTSurf, starw, "Clipped Sphere", 0);
  surf1->SetPos(-3, 0, 0);
  surf1->SetRotByDegrees(55, -30, 0);
  surf1->SetColor(1,0.8,0.2);

  maker1->SetTarget(surf1);
  maker1->SetFormula("pow((x-1)^2 + y^2 + z^2, -1) + pow((x+1)^2 + y^2 + z^2, -1)");
  maker1->SetValue(1.);
  maker1->MakeSurface();

  CREATE_ADD_GLASS(retring, GTSRetriangulator, starw, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  retring->SetTarget(surf1);

  // Bool stuff

  CREATE_ADD_GLASS(surfa, GTSurf, starw, "Sphere A", 0);
  surfa->GenerateSphere(1);
  surfa->SetPos(-0.5, 0, 3);
  surfa->SetRnrSelf(false);
  // surfa->RotateLF(2, 3, 0.2);

  CREATE_ADD_GLASS(surfb, GTSurf, starw, "Sphere B", 0);
  surfb->GenerateSphere(1);
  surfb->SetPos( 0.5, 0, 3);
  surfb->SetRnrSelf(false);

  CREATE_ADD_GLASS(surfu, GTSurf, starw, "Union", 0);
  surfu->SetPos(0, 0, 3);
  surfu->SetRnrPoints(true);

  surfu->Union(surfa, surfb);
  // surfu->Intersection(surfa, surfb);
  // surfu->Difference(surfa, surfb);

  // Clipped cone

  CREATE_ADD_GLASS(cone, GTSurf, starw, "Cone", 0);
  cone->SetColor(0.7, 0.5, 1);
  cone->Load("cone.gts");
  cone->Invert();
  cone->SetRotByDegrees(-90, 0, 0);
  cone->MoveLF(2, -0.5);
  cone->RescaleXYZ(1, 2, 1);
  cone->TransformAndResetTrans();

  CREATE_ADD_GLASS(boxu, GTSurf, starw, "Box Z Up", 0);
  boxu->SetRnrSelf(false);
  boxu->SetColor(0.5, 0.5, 0.5, 0.3);
  boxu->Load("cube.gts");
  boxu->Move3LF(0.5, 0, 0.5);
  boxu->RescaleXYZ(3, 3, 1);
  boxu->TransformAndResetTrans();
  boxu->MoveLF(3, 0.35);

  CREATE_ADD_GLASS(boxd, GTSurf, starw, "Box Z Down", 0);
  boxd->SetRnrSelf(false);
  boxd->SetColor(0.5, 0.5, 0.5, 0.3);
  boxd->Load("cube.gts");
  boxd->Move3LF(0.5, 0, -0.5);
  boxd->RescaleXYZ(3, 3, 1);
  boxd->TransformAndResetTrans();
  boxd->MoveLF(3, -0.1);

  cone->Difference(cone, boxu);
  cone->Difference(cone, boxd);
  cone->MoveLF(1, 2);
  cone->SetRotByDegrees(30, 30, 30);
  cone->SetScale(2);
  cone->SetUseScale(true);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();
  g_pupil->SetBlend(true);

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
