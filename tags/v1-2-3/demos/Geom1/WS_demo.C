// $Header$

// simple scene with a Weaver Symbol
// Use "ZNode(Pos,Rot):WSPoint(W,S,T)" as custom view for best performance.
//
// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

void WS_demo()
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

  Lamp  *l;

  Scene* wsdemo  = new Scene("WeaverSymbol Demo Scene");
  scenes->CheckIn(wsdemo);
  scenes->Add(wsdemo);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  scenes->CheckIn(base_plane);
  wsdemo->Add(base_plane);

  ZNode* lamp_base = new ZNode("LampBase");
  lamp_base->MoveLF(3, 3.5);
  scenes->CheckIn(lamp_base);
  wsdemo->Add(lamp_base);
  
  l = new Lamp("Lamp1");
  l->SetDiffuse(1, 0.3, 0.3);
  l->MoveLF(1, 5); l->RotateLF(1,2, TMath::Pi());
  l->SetScale(1);
  scenes->CheckIn(l); lamp_base->Add(l);
  wsdemo->GetGlobLamps()->Add(l);
  n = new Sphere(0.1, "Sph1");
  scenes->CheckIn(n);
  l->Add(n);

  l = new Lamp("Lamp2");
  l->SetDiffuse(0.3, 0.3, 1);
  l->MoveLF(1,-5);
  l->SetScale(1);
  scenes->CheckIn(l); lamp_base->Add(l);
  wsdemo->GetGlobLamps()->Add(l);
  n = new Sphere(0.1, "Sph2");
  scenes->CheckIn(n);
  l->Add(n);

  CREATE_ADD_GLASS(image1, ZImage, wsdemo, "Photon", 0);
  image1->SetFile("photon.png");
  image1->Load();
  image1->SetEnvMode(GL_MODULATE);
  image1->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, wsdemo, "Gluon", 0);
  image2->SetFile("gluon.png");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  WSSeed* seed1 = new WSSeed();
  seed1->SetFat(true);
  seed1->SetTexture(image1);
  scenes->CheckIn(seed1); wsdemo->Add(seed1);

  Int_t NP = 10;
  WSPoint **points = new WSPoint[NP];
  for(int i=0; i<NP; ++i) {
    points[i] = new WSPoint(GForm("Point %d",i+1));
    double phi = i*TMath::Pi()/4;
    double ss = TMath::Sin(phi);
    double cc = TMath::Cos(phi);
    points[i]->Set3Pos(i*cc, i*ss, 0);
    double der = TMath::ATan2(ss+phi*cc, cc-phi*ss);
    points[i]->RotateLF(1,2, der);
    points[i]->SetW(i*0.1);
    scenes->CheckIn(points[i]); seed1->Add(points[i]);
  }

}
