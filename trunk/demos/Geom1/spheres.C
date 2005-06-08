// $Header$

// Spheres: a bunch of spheres with a rotator
//
// vars: ZQueen* g_queen
// libs: Geom1

#include <gl_defines.h>

void spheres()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  Scene* spheres = new Scene("Spheres");
  g_queen->CheckIn(spheres);
  g_queen->Add(spheres);
  g_scene = spheres;

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(40);
  g_queen->CheckIn(base_plane);
  spheres->Add(base_plane);

  Lamp* lamp = new Lamp("Lamp");
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->MoveLF(3, 20); lamp->RotateLF(3,1, TMath::Pi()/2);
  g_queen->CheckIn(lamp); spheres->Add(lamp);
  spheres->GetGlobLamps()->Add(lamp);

  Sphere* top_sphere = new Sphere("Mother Sphere");
  top_sphere->SetRadius(25);
  top_sphere->SetLOD(50);
  top_sphere->SetColor(0.7, 0.1, 0.3);
  g_queen->CheckIn(top_sphere); spheres->Add(top_sphere);

  ZGlLightModel* lm = new ZGlLightModel("ZGlLightModel 1");
  g_queen->CheckIn(lm);
  lm->SetShadeModelOp(1);
  lm->SetFrontMode(GL_LINE);
  lm->SetBackMode(GL_LINE);
  lm->SetFaceCullOp(1);
  top_sphere->SetRnrMod(lm);
  top_sphere->SetModElements(false);

  Sphere* sss[6];
  {
    float x = 0; float phi = 3.14 / 6;
    for(int i=1; i<=6; ++i) {
      Sphere* s = new Sphere(Form("Sphere_%d",i));
      sss[i-1] = s;
      s->MoveLF(1,x - 20); s->RotateLF(2,3,(i-1)*phi);
      s->SetRadius(i);
      s->SetColor(1,1,1);
      g_queen->CheckIn(s); top_sphere->Add(s);
      float y = i + 1;
      for(int j=1; j<=3; ++j) {
	Sphere* ss = new Sphere(Form("Sphere_%d_%d",i,j));
	ss->MoveLF(2,y);
	ss->SetRadius(j);
	float c[3] = {0,0,0}; c[j-1] = 1;
	ss->SetColor(c[0], c[1], c[2]);
	g_queen->CheckIn(ss); s->Add(ss);
	y += 2*j + 1;
      }
      x += 2*i + 1;
    }
  }

  ZGlLightModel* lm2 = new ZGlLightModel("ZGlLightModel 2");
  g_queen->CheckIn(lm2);
  lm2->SetShadeModelOp(1);
  lm2->SetFrontMode(GL_LINE);
  lm2->SetBackMode(GL_LINE);
  sss[4]->SetRnrMod(lm2);
  sss[4]->SetModSelf(false);

  Eventor* etor = new Eventor("Dynamo");
  etor->SetBeatsToDo(-1);
  etor->SetInterBeatMS(25);
  etor->SetStampInterval(50);
  g_queen->CheckIn(etor); spheres->Add(etor);

  Mover* mv = new Mover("S1 Rotator");
  mv->SetNode(top_sphere); mv->SetRi(1); mv->SetRj(2); mv->SetRa(0.001);
  g_queen->CheckIn(mv);
  etor->Add(mv);
  
  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  etor->Start();
}
