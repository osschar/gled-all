// $Header$

// simple scene with some GTS models; more available from http://gts.sf.net/

// vars: ZQueen* scenes
// libs: Geom1 GTS

#include <glass_defines.h>

void starwars()
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
    // Declares sun, sun_queen, fire_queen and scenes.
    // Starts Saturn in master server mode.
  }
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GTS");

  Scene* starw  = new Scene("StarWars");
  scenes->CheckIn(starw);
  scenes->Add(starw);

  // Geom elements

  CREATE_ADD_GLASS(cambase, Sphere, starw, "Camera Base",
		   "Type <Home> to jump here.");
  cambase->SetRadius(0.005);
  cambase->Set3Pos(0, -9, 5);
  cambase->SetRotByDegrees(90, -30, 0);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->Set3Pos(0, 0, -5);
  base_plane->SetUnitSquare(15);
  base_plane->SetUV(30, 30);
  scenes->CheckIn(base_plane);
  starw->Add(base_plane);

  Lamp* l = new Lamp("Lamp");
  l->Set3Pos(15, 0, 5);
  l->SetScale(1);
  scenes->CheckIn(l); starw->Add(l);
  starw->GetGlobLamps()->Add(l);

  // GTS models

  GTSurf* surf1 = new GTSurf("Station");
  surf1->Set3Pos(-4.1, -4.3, 0);
  surf1->SetScale(0.4);
  surf1->SetColor(0.6, 1, 0.9);
  scenes->CheckIn(surf1); starw->Add(surf1);
  surf1->SetFile("space_station.gts");
  surf1->Load();
  surf1->Invert(); // this gts file has unstandard orientation of faces

  GTSurf* surf2 = new GTSurf("X Wing");
  surf2->Set3Pos(4.3, -4.3, 0);
  surf2->SetRotByDegrees(110, 180, 170);
  surf2->SetScale(0.12);
  surf2->SetColor(0.6, 1, 0.7);
  scenes->CheckIn(surf2); starw->Add(surf2);
  surf2->SetFile("x_wing.gts");
  surf2->Load();

  GTSurf* surf3 = new GTSurf("Tie");
  surf3->Set3Pos(1, 2.3, 1);
  surf3->SetRotByDegrees(20, 10, -10);
  surf3->SetScale(0.06);
  surf3->SetColor(1, 0.3, 0.5);
  scenes->CheckIn(surf3); starw->Add(surf3);
  surf3->SetFile("tie.gts");
  surf3->Load();

  // Station rotator
  CREATE_ADD_GLASS(dynamo, Eventor, starw, "Dynamo", 0);
  dynamo->SetBeatsToDo(-1);
  dynamo->SetInterBeatMS(40);
  dynamo->SetStampInterval(10);

  CREATE_ADD_GLASS(mv, Mover, dynamo, "S1 Rotator", 0);
  mv->SetNode(surf1); mv->SetRi(1); mv->SetRj(2); mv->SetRa(0.005);


  // Spawn GUI
  // gROOT->ProcessLine(".x eye.C");

  CREATE_ADD_GLASS(shell, ShellInfo, fire_queen, "Shell",
		   "Reflects into a FTW_Shell");
  // shell->ImportKings();  // Get all Kings as top level objects
  shell->Add(starw);

  CREATE_ADD_GLASS(pupil, PupilInfo, shell->GetPupils(), "PupilInfo",
		   "Reflects into a GL viewer");
  pupil->Add(starw);
  pupil->SetCameraBase(cambase);

  Gled::theOne->SpawnEye(shell, "Eye of Ra");


  // Start the thread rotating the station.
  dynamo->Start();
}
