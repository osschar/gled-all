// hello_gled.C
// top-level Saturn startup script
{
  // ROOT usually cacthes segvs ... and then dies horribly anyway.
  // Use this to get your core:
  // gSystem->IgnoreSignal(kSigSegmentationViolation, true);

  Gled::theOne->AssertLibSet("Geom1");

  gROOT->Macro("sun.C");	// init vars needed for further scripts
  
  Scene* hello_gled  = new Scene("Hello Gled");
  scenes->CheckIn(hello_gled);
  scenes->Add(hello_gled);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  scenes->CheckIn(base_plane);
  hello_gled->Add(base_plane);

  SMorph* smorph = new SMorph();
  smorph->SetUseScale(true);
  smorph->SetScales(4, 2, 1);
  smorph->SetCx(0.5);
  smorph->SetColor(0.2, 1, 0.2);
  smorph->SetTLevel(16); smorph->SetPLevel(16);
  scenes->CheckIn(smorph); hello_gled->Add(smorph);

  ZNode* node1 = new ZNode("First Node");
  scenes->CheckIn(node1); hello_gled->Add(node1);
  Sphere* sph1 = new Sphere("First Sphere");
  sph1->MoveLF(1,5);
  sph1->SetColor(1,0.3,0.3); sph1->SetLOD(16);
  scenes->CheckIn(sph1);
  node1->Add(sph1);

  ZNode* node2 = new ZNode("Second Node");
  scenes->CheckIn(node2); hello_gled->Add(node2);
  Sphere* sph2 = new Sphere("Second Sphere");
  sph2->MoveLF(1,-5);
  sph2->SetColor(0.3,0.3,1); sph2->SetLOD(16);
  scenes->CheckIn(sph2);
  node2->Add(sph2);

  Eventor* dynamo = new Eventor("Dynamo");
  dynamo->SetBeatsToDo(-1);
  dynamo->SetStampInterval(10);
  dynamo->SetInterBeatMS(50);
  scenes->CheckIn(dynamo); hello_gled->Add(dynamo);
 
  Mover* mover1 = new Mover("Rotates first node");
  mover1->SetNode(node1);
  mover1->SetRi(1); mover1->SetRj(2); mover1->SetRa(0.01745);
  scenes->CheckIn(mover1); dynamo->Add(mover1);

  Mover* mover2 = new Mover("Rotates second node");
  mover2->SetNode(node2);
  mover2->SetRi(3); mover2->SetRj(1); mover2->SetRa(0.01745/2);
  scenes->CheckIn(mover2); dynamo->Add(mover2);

  dynamo->Start();

  gROOT->Macro("eye.C");	// spawn an eye with a pupil on first scene
}
