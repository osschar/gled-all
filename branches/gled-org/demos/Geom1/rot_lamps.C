// $Header$

// simple scene with rotating lamps
// type dynamo->Start() (or use GUI) to start the thread
//
// vars: ZQueen* scenes
// libs: Geom1
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

  Lamp  *l;

  Scene* rot_lamps  = new Scene("Rotating Lamps Scene");
  scenes->CheckIn(rot_lamps);
  scenes->Add(rot_lamps);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  scenes->CheckIn(base_plane);
  rot_lamps->Add(base_plane);

  ZNode* lamp_base = new ZNode("LampBase");
  lamp_base->MoveLF(3, 3.5);
  scenes->CheckIn(lamp_base);
  rot_lamps->Add(lamp_base);
  
  l = new Lamp("Lamp1");
  l->SetDiffuse(0.8, 0.3, 0.5);
  l->MoveLF(1, 5); l->RotateLF(1,2, TMath::Pi());
  scenes->CheckIn(l); lamp_base->Add(l);
  rot_lamps->GetGlobLamps()->Add(l);
  n = new Sphere(0.1, "Sph1");
  scenes->CheckIn(n);
  l->Add(n);

  l = new Lamp("Lamp2");
  l->SetDiffuse(0.5, 0.3, 0.8);
  l->MoveLF(1,-5);
  scenes->CheckIn(l); lamp_base->Add(l);
  rot_lamps->GetGlobLamps()->Add(l);
  n = new Sphere(0.1, "Sph2");
  scenes->CheckIn(n);
  l->Add(n);

  SMorph* n1 = new SMorph(); n1->SetSs(5, 2, 1); n1->SetNormalize(true);
  n1->SetColor(0.48, 0.78, 0.58);
  n1->SetTLevel(16); n1->SetPLevel(16);
  scenes->CheckIn(n1); rot_lamps->Add(n1);
  Sphere* n2 = new Sphere(); n2->MoveLF(1,5); n2->SetRadius(0.5);
  n2->SetColor(1,0.3,0.3);
  scenes->CheckIn(n2); n1->Add(n2);
  Sphere* n3 = new Sphere(); n3->MoveLF(2,2); n3->SetRadius(0.5);
  n3->SetColor(0,1,1);
  scenes->CheckIn(n3); n1->Add(n3);

  // Eventor and a Mover

  Eventor *dynamo = new Eventor("Dynamo");
  dynamo->SetBeatsToDo(-1);
  dynamo->SetStampInterval(10);
  dynamo->SetInterBeatMS(50);
  scenes->CheckIn(dynamo); rot_lamps->Add(dynamo);

  Mover* mm = new Mover("Rotates_LampBase");
  mm->SetNode(lamp_base);
  mm->SetRi(1); mm->SetRj(2); mm->SetRa(0.01745);
  scenes->CheckIn(mm); dynamo->Add(mm);
}
