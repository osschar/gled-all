// $Header$

// simple scene with some GTS models; more available from http://gts.sf.net/

// vars: ZQueen* scenes
// libs: Geom1 GTS
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("GTS");

  Scene* starw  = new Scene("StarWars");
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

  // GTS models

  GTSurf* surf1 = new GTSurf("Station");
  surf1->Set3Pos(45,0,0);
  surf1->SetRotByDegrees(0, 28, -83);
  surf1->SetScale(3.5);
  scenes->CheckIn(surf1); starw->Add(surf1);
  surf1->SetFile("space_station.gts");
  surf1->Load();
  surf1->Invert(); // this gts file has unstandard orientation of faces

  GTSurf* surf2 = new GTSurf("X Wing");
  surf2->Set3Pos(45,-17,25);
  surf2->SetRotByDegrees(67, 41, 194);
  surf2->SetScale(0.7);
  scenes->CheckIn(surf2); starw->Add(surf2);
  surf2->SetFile("x_wing.gts");
  surf2->Load();

  GTSurf* surf3 = new GTSurf("Tie");
  surf3->Set3Pos(46,0,-42);
  surf3->SetRotByDegrees(0, 12, 16);
  surf3->SetScale(0.2);
  scenes->CheckIn(surf3); starw->Add(surf3);
  surf3->SetFile("tie.gts");
  surf3->Load();

  { // Station rotator
    Eventor* e = new Eventor("Dynamo");
    e->SetBeatsToDo(-1); e->SetInterBeatMS(40); e->SetStampInterval(10);
    scenes->CheckIn(e); starw->Add(e);
    Mover* mv = new Mover("S1 Rotator");
    mv->SetNode(surf1); mv->SetRi(1); mv->SetRj(2); mv->SetRa(0.005);
    scenes->CheckIn(mv); e->Add(mv);
  }
  e->Start();
}
