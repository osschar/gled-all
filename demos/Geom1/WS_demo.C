// $Id$

// simple scene with a Weaver Symbol
// Use "ZNode(Pos,Rot):WSPoint(W,S,T)" as custom view for best performance.
//
// vars: ZQueen* g_queen
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

void WS_demo()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");
  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("WS_Demo", "ZList");
    l->Swallow(new ZGlass("ZNode:WS_Point",
			  "ZNode(Pos[18],Rot[18]):WSPoint(*)"));
  }

  Scene* wsdemo  = new Scene("WeaverSymbol Demo Scene");
  g_queen->CheckIn(wsdemo);
  g_queen->Add(wsdemo);
  g_scene = wsdemo;

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  g_queen->CheckIn(base_plane);
  wsdemo->Add(base_plane);

  ZNode* lamp_base = new ZNode("LampBase");
  lamp_base->MoveLF(3, 3.5);
  g_queen->CheckIn(lamp_base);
  wsdemo->Add(lamp_base);

  CREATE_ADD_GLASS(lamp1, Lamp, lamp_base, "Lamp1", 0);
  lamp1->MoveLF(1, 5);
  lamp1->RotateLF(1,2, TMath::Pi());
  lamp1->SetDiffuse(1, 0.3, 0.3);
  lamp1->SetLampScale(1);
  wsdemo->GetGlobLamps()->Add(lamp1);
  CREATE_ADD_GLASS(sph1, Sphere, lamp1, "Sph1", 0);
  sph1->SetRadius(0.38); sph1->SetLOD(20);

  CREATE_ADD_GLASS(lamp2, Lamp, lamp_base, "Lamp2", 0);
  lamp2->MoveLF(1,-5);
  lamp2->SetDiffuse(0.3, 0.3, 1);
  lamp2->SetLampScale(1);
  wsdemo->GetGlobLamps()->Add(lamp2);
  CREATE_ADD_GLASS(sph2, Sphere, lamp2, "Sph2", 0);
  sph2->SetRadius(0.38); sph2->SetLOD(20);

  CREATE_ADD_GLASS(image1, ZImage, wsdemo, "Photon", 0);
  image1->SetFile("photon-blurred.png");
  image1->Load();
  image1->SetEnvMode(GL_MODULATE);
  image1->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, wsdemo, "Checker", 0);
  image2->SetFile("checker_4.png");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(seed1, WSSeed, wsdemo, "WSSeed", 0);
  seed1->SetFat(true);
  seed1->SetTexture(image1);

  Int_t NP = 10;
  WSPoint **points = new WSPoint[NP];
  for(int i=0; i<NP; ++i) {
    points[i] = new WSPoint(GForm("Point %d",i+1));
    double phi = i*TMath::Pi()/4;
    double ss = TMath::Sin(phi);
    double cc = TMath::Cos(phi);
    points[i]->SetPos(i*cc, i*ss, 0);
    double der = TMath::ATan2(ss+phi*cc, cc-phi*ss);
    points[i]->RotateLF(1,2, der);
    points[i]->SetW(i*0.1);
    g_queen->CheckIn(points[i]); seed1->Add(points[i]);
  }

  CREATE_ADD_GLASS(t1, SMorph, wsdemo, "TubeStart", 0);
  t1->SetTLevel(20); t1->SetPLevel(3);
  t1->SetPos(-5, 8, 0);
  t1->SetColor(.6, 0.2, 0.2);

  CREATE_ADD_GLASS(t2, SMorph, wsdemo, "TubeEnd", 0);
  t2->SetTLevel(20); t2->SetPLevel(5);
  t2->SetPos(3, -2, 6);
  t2->SetColor(.2, 0.2, 0.6);

  CREATE_ADD_GLASS(tube1, WSTube, wsdemo, "Tube1", 0);
  tube1->SetTLevel(30);
  tube1->SetFat(false);
  tube1->SetTexture(image2);
  tube1->SetNodeA(t1);
  tube1->SetNodeB(t2);
  tube1->Connect();

  CREATE_ADD_GLASS(tube2, WSTube, wsdemo, "Tube2", 0);
  tube2->SetTLevel(30);
  tube2->SetFat(false);
  tube2->SetTexture(image2);
  tube2->SetNodeA(sph1);
  tube2->SetNodeB(sph2);
  tube2->SetVecA(TLorentzVector(0,     0,  1, 1));
  tube2->SetVecB(TLorentzVector(0,     0, -1, 1));
  tube2->SetSgmA(TLorentzVector(0.2, 0.2,  0, 0));
  tube2->SetSgmB(TLorentzVector(0.2, 0.2,  0, 0));
  tube2->Connect();

  CREATE_ADD_GLASS(etor, Eventor, wsdemo, "Eventor", 0);
  etor->SetInterBeatMS(50);
  CREATE_ADD_GLASS(tmaker, TimeMaker, etor, "TimeMaker", 0);
  CREATE_ATT_GLASS(wker, WSWalker, tmaker, AddClient, "WSWalker", 0);
  wker->SetLoop(true);

  CREATE_ADD_GLASS(traveler, SMorph, seed1, "Travel SMorph", 0);
  wker->SetSeed(seed1);
  wker->SetNode(traveler);

  Gled::Macro("eye.C");
  setup_pupil_up_reference();
}
