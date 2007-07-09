// $Header$

// simple scene with some GTS models; more available from http://gts.sf.net/

// vars: ZQueen* g_queen
// libs: Geom1 GTS

#include <glass_defines.h>

void starwars()
{
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GTS");

  Gled::AssertMacro("sun_demos.C");

  Scene* starw  = new Scene("StarWars");
  g_queen->CheckIn(starw);
  g_queen->Add(starw);
  g_scene = starw;

  // Geom elements

  CREATE_ADD_GLASS(cambase, Sphere, starw, "Camera Base",
		   "Type <Home> to jump here.");
  cambase->SetRadius(0.005);
  cambase->SetPos(0, -9, 5);
  cambase->SetRotByDegrees(90, -30, 0);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetPos(0, 0, -5);
  base_plane->SetUnitSquare(15);
  base_plane->SetUV(30, 30);
  g_queen->CheckIn(base_plane);
  starw->Add(base_plane);

  Lamp* l = new Lamp("Lamp");
  l->SetPos(15, 0, 5);
  l->SetLampScale(1);
  g_queen->CheckIn(l); starw->Add(l);
  starw->GetGlobLamps()->Add(l);

  // GTS models

  GTSurf* surf1 = new GTSurf("Station");
  surf1->SetPos(-4.1, -4.3, 0);
  surf1->SetColor(0.6, 1, 0.9);
  g_queen->CheckIn(surf1); starw->Add(surf1);
  surf1->SetFile("space_station.gts");
  surf1->Load();
  surf1->Invert(); // this gts file has unstandard orientation of faces
  surf1->Rescale(0.4);

  GTSurf* surf2 = new GTSurf("X Wing");
  surf2->SetPos(4.3, -4.3, 0);
  surf2->SetRotByDegrees(110, 180, 170);
  surf2->SetColor(0.6, 1, 0.7);
  g_queen->CheckIn(surf2); starw->Add(surf2);
  surf2->SetFile("x_wing.gts");
  surf2->Load();
  surf2->Rescale(0.12);

  GTSurf* surf3 = new GTSurf("Tie");
  surf3->SetPos(1, 2.3, 1);
  surf3->SetRotByDegrees(20, 10, -10);
  surf3->SetColor(1, 0.3, 0.5);
  g_queen->CheckIn(surf3); starw->Add(surf3);
  surf3->SetFile("tie.gts");
  surf3->Load();
  surf3->Rescale(0.06);

  // Station rotator
  CREATE_ADD_GLASS(dynamo, Eventor, starw, "Dynamo", 0);
  dynamo->SetBeatsToDo(-1);
  dynamo->SetInterBeatMS(40);
  dynamo->SetStampInterval(10);

  CREATE_ADD_GLASS(mv, Mover, dynamo, "S1 Rotator", 0);
  mv->SetNode(surf1); mv->SetRi(1); mv->SetRj(2); mv->SetRa(0.005);


  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  Gled::theOne->LockFltk();

  if(g_pupil) {
    g_pupil->SetCameraBase(cambase);
    g_pupil->EmitCameraHomeRay();
  }

  Gled::theOne->UnlockFltk();

  // Start the thread rotating the station.
  dynamo->Start();
}
