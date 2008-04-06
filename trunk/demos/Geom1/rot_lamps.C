// $Header$

// simple scene with rotating lamps
// type dynamo->Start() (or use GUI) to start the thread
//
// vars: ZQueen* g_queen
// libs: Geom1
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  Lamp  *l;

  Scene* rot_lamps  = new Scene("Rotating Lamps Scene");
  g_queen->CheckIn(rot_lamps);
  g_queen->Add(rot_lamps);
  g_scene = rot_lamps;

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  g_queen->CheckIn(base_plane);
  rot_lamps->Add(base_plane);

  ZNode* lamp_base = new ZNode("LampBase");
  lamp_base->MoveLF(3, 3.5);
  g_queen->CheckIn(lamp_base);
  rot_lamps->Add(lamp_base);

  l = new Lamp("Lamp1");
  l->SetDiffuse(1, 0.3, 0.4);
  l->MoveLF(1, 5);
  l->SetRotByDegrees(180, -30, 0);
  l->SetLampScale(1);
  l->SetSpotExp(3);
  l->SetSpotCutOff(60);
  l->SetLinAtt(0.05);
  g_queen->CheckIn(l); lamp_base->Add(l);
  rot_lamps->GetGlobLamps()->Add(l);

  l = new Lamp("Lamp2");
  l->SetDiffuse(0.2, 0.5, 1);
  l->MoveLF(1,-5);
  l->SetRotByDegrees(0, -30, 0);
  l->SetLampScale(1);
  l->SetSpotExp(3);
  l->SetSpotCutOff(60);
  l->SetLinAtt(0.05);
  g_queen->CheckIn(l); lamp_base->Add(l);
  rot_lamps->GetGlobLamps()->Add(l);

  SMorph* n1 = new SMorph(); n1->SetScales(5, 2, 1);
  n1->SetColor(0.75, 0.75, 0.75);
  n1->SetTLevel(16); n1->SetPLevel(16);
  g_queen->CheckIn(n1); rot_lamps->Add(n1);

  Sphere* n2 = new Sphere(); n2->MoveLF(1,1); n2->SetRadius(0.5);
  n2->SetColor(1,0.3,0.3);
  n2->SetLOD(20);
  g_queen->CheckIn(n2); n1->Add(n2);
  Sphere* n3 = new Sphere(); n3->MoveLF(2,1); n3->SetRadius(0.5);
  n3->SetColor(0,1,1);
  n3->SetLOD(20);
  g_queen->CheckIn(n3); n1->Add(n3);

  // Eventor and a Mover

  Eventor *dynamo = new Eventor("Dynamo");
  dynamo->SetBeatsToDo(-1);
  dynamo->SetStampInterval(10);
  dynamo->SetInterBeatMS(50);
  g_queen->CheckIn(dynamo); rot_lamps->Add(dynamo);

  Mover* mm = new Mover("Rotates_LampBase");
  mm->SetNode(lamp_base);
  mm->SetRi(1); mm->SetRj(2); mm->SetRa(0.01745);
  g_queen->CheckIn(mm); dynamo->Add(mm);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  dynamo->Start();
}
