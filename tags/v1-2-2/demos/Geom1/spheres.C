// $Header$

// Spheres: a bunch of spheres with a rotator
//
// vars: ZQueen* scenes
// libs: Geom1
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

  Scene* spheres = new Scene("Spheres");
  scenes->CheckIn(spheres);
  scenes->Add(spheres);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(40);
  scenes->CheckIn(base_plane);
  spheres->Add(base_plane);

  Lamp* lamp = new Lamp("Lamp");
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->MoveLF(3, 20); lamp->RotateLF(3,1, TMath::Pi()/2);
  scenes->CheckIn(lamp); spheres->Add(lamp);
  spheres->GetGlobLamps()->Add(lamp);
  ZNode* n = new Sphere(1.5, "Sph");
  scenes->CheckIn(n); lamp->Add(n);

  {
    float x = 0; float phi = 3.14 / 6;
    Sphere* sss[6];
    for(int i=1; i<=6; ++i) {
      Sphere* s = new Sphere(Form("Sphere_%d",i));
      sss[i-1] = s;
      s->MoveLF(1,x - 20); s->RotateLF(2,3,(i-1)*phi);
      s->SetRadius(i);
      s->SetColor(1,1,1);
      scenes->CheckIn(s); spheres->Add(s);
      float y = i + 1;
      for(int j=1; j<=3; ++j) {
	Sphere* ss = new Sphere(Form("Sphere_%d_%d",i,j));
	ss->MoveLF(2,y);
	ss->SetRadius(j);
	float c[3] = {0,0,0}; c[j-1] = 1;
	ss->SetColor(c[0], c[1], c[2]);
	scenes->CheckIn(ss); s->Add(ss);
	y += 2*j + 1;
      }
      x += 2*i + 1;
    }
  }

  {
    Eventor* e = new Eventor("Dynamo");
    e->SetBeatsToDo(-1); e->SetInterBeatMS(10); e->SetStampInterval(10);
    scenes->CheckIn(e); spheres->Add(e);
    Mover* mv = new Mover("S1 Rotator");
    mv->SetNode(sss[0]); mv->SetRi(1); mv->SetRj(3); mv->SetRa(0.005);
    scenes->CheckIn(mv); e->Add(mv);
  }

}
