// $Header$

// simple scene with images and RectTerrain
//
// crystal.png bacteria.png are 512x512 AFM scans, eg:
//   image3->SetFile("bacteria.png");
//   image3->Load();
//   terrain->SetFromImage(image3);

// vars: ZQueen* scenes
// libs: Geom1
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

#include <gl_defines.h>

  Scene* images  = new Scene("Images");
  scenes->CheckIn(images);
  scenes->Add(images);

  // Images

  ZImage* image1 = new ZImage("GledLogo");
  scenes->CheckIn(image1);
  images->Add(image1);
  image1->SetFile("gledlogo.png");
  image1->Load();
  image1->SetEnvMode(GL_MODULATE);
  image1->SetLoadAdEnlight(true);

  ZImage* image2 = new ZImage("Orchid");
  scenes->CheckIn(image2);
  images->Add(image2);
  image2->SetFile("orchid.jpeg");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  ZImage* image3 = new ZImage("HeightField");
  scenes->CheckIn(image3);
  images->Add(image3);
  image3->SetFile("terrain_128.png");
  image3->Load();
  image3->SetLoadAdEnlight(true);

  ZImage* image4 = new ZImage("Earth map");
  image4->SetMagFilter(GL_LINEAR);
  image4->SetEnvMode(GL_MODULATE);
  scenes->CheckIn(image4);
  images->Add(image4);
  image4->SetFile("earth.png");
  image4->Load();
  image4->SetLoadAdEnlight(true);

  ZImage* image5 = new ZImage("Moon map");
  image5->SetMagFilter(GL_LINEAR);
  image5->SetEnvMode(GL_MODULATE);
  scenes->CheckIn(image5);
  images->Add(image5);
  image5->SetFile("moon.jpeg");
  image5->Load();
  image5->SetLoadAdEnlight(true);

  ZImage* image6 = new ZImage("Checker");
  image6->SetMagFilter(GL_LINEAR);
  image6->SetEnvMode(GL_MODULATE);
  scenes->CheckIn(image6);
  images->Add(image6);
  image6->SetFile("checker_8.png");
  image6->Load();
  image6->SetLoadAdEnlight(true);


  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  scenes->CheckIn(base_plane);
  images->Add(base_plane);


  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetScale(1);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  scenes->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);
  ZNode* n = new Sphere(0.5, "Sph");
  scenes->CheckIn(n); l->Add(n);


  Board* board1 = new Board("Board1", "Showing GledLogo");
  scenes->CheckIn(board1);
  images->Add(board1);
  board1->SetColor(1, 0.7, 0.7);
  board1->SetTexture(image1);
  board1->SetULen(2.5*1.75); board1->SetVLen(2.5*1);
  board1->Set3Pos(-3, 2.5, 0.01);

  Board* board2 = new Board("Board2", "Showing Orchid");
  scenes->CheckIn(board2);
  images->Add(board2);
  board2->SetTexture(image2);
  board2->SetULen(2.5); board2->SetVLen(2.5);
  board2->Set3Pos(3, 2.5, 0.01);

  RectTerrain* terrain = new RectTerrain("Terrain");
  scenes->CheckIn(terrain);
  images->Add(terrain);
  terrain->SetFromImage(image3);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetZScale(0.5);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->ToCenter();
  terrain->Set3Pos(0, -3, 0.01);


  SMorph* n1 = new SMorph("Earth"); n1->SetS(2); n1->SetNormalize(true);
  n1->SetTLevel(20); n1->SetPLevel(20);
  n1->SetTexture(image4);
  n1->Set3Pos(-6, -6, 0); n1->SetRotByAngles(0, TMath::Pi()/2, 0);
  scenes->CheckIn(n1); images->Add(n1);

  SMorph* n2 = new SMorph("Moon"); n2->SetS(2); n2->SetNormalize(true);
  n2->SetTLevel(20); n2->SetPLevel(20);
  n2->SetTexture(image5);
  n2->Set3Pos( 6, -6, 0); n2->SetRotByAngles(0, TMath::Pi()/2, 0);
  scenes->CheckIn(n2); images->Add(n2);

  SMorph* morphs[3];
  for(int i=0; i<3; ++i) {
    morphs[i] = new SMorph(Form("Morph %d", i+1));
    morphs[i]->SetTLevel(20); morphs[i]->SetPLevel(20);
    morphs[i]->SetS(2);
    morphs[i]->SetNormalize(true);
    morphs[i]->SetTexture(image6);
    morphs[i]->Set3Pos(-6+6*i, 7, 0);
    scenes->CheckIn(morphs[i]); images->Add(morphs[i]);
  }
  morphs[0]->SetTx(1); morphs[0]->SetCx(0.5); morphs[0]->SetRz(-0.25);
  morphs[1]->SetTx(0); morphs[1]->SetCx(-0.5); morphs[1]->SetRz(0.25);
  morphs[2]->SetTx(-1); morphs[2]->SetCx(0.25); morphs[2]->SetRz(0.25);
}
