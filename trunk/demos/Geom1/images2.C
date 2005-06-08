// $Header$

// Simple scene with images and RectTerrain.
// Like images.C but with severeal queens (test of queen dependencies).
// Does NOT create the default GUI.
// Requires manual dragging of SunKing/GeomQueen/Images into the PupilInfo.

// vars: ZQueen* g_queen
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

void images2()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  // Images

  ZQueen* imgqueen = new ZQueen(128, "ImageQueen", "");
  Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(imgqueen);
  imgqueen->SetMandatory(true);

  CREATE_ADD_GLASS(image1, ZImage, imgqueen, "GledLogo", 0);
  image1->SetFile("gledlogo.png");
  image1->Load();
  image1->SetEnvMode(GL_MODULATE);
  image1->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, imgqueen, "Orchid", 0);
  image2->SetFile("orchid.jpeg");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image3, ZImage, imgqueen, "HeightField", 0);
  image3->SetFile("terrain_128.png");
  image3->Load();
  image3->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image4, ZImage, imgqueen, "Earth map", 0);
  image4->SetMagFilter(GL_LINEAR);
  image4->SetEnvMode(GL_MODULATE);
  image4->SetFile("earth.png");
  image4->Load();
  image4->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image5, ZImage, imgqueen, "Moon map", 0);
  image5->SetMagFilter(GL_LINEAR);
  image5->SetEnvMode(GL_MODULATE);
  image5->SetFile("moon.jpeg");
  image5->Load();
  image5->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image6, ZImage, imgqueen, "Checker", 0);
  image6->SetMagFilter(GL_LINEAR);
  image6->SetEnvMode(GL_MODULATE);
  image6->SetFile("checker_8.png");
  image6->Load();
  image6->SetLoadAdEnlight(true);


  // Geom elements

  ZQueen* geomqueen = new ZQueen(10000, "GeomQueen", "");
  Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(geomqueen);
  geomqueen->SetMandatory(true);
  geomqueen->GetDeps()->Add(imgqueen);

  Scene* images  = new Scene("Images");
  geomqueen->CheckIn(images);
  geomqueen->Add(images);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  geomqueen->CheckIn(base_plane);
  images->Add(base_plane);


  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetScale(1);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  geomqueen->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);
  ZNode* n = new Sphere(0.5, "Sph");
  geomqueen->CheckIn(n); l->Add(n);

  CREATE_ADD_GLASS(goo, ZNode, images, "SmorphContainer", 0);

  Eventor* e = new Eventor;
  e->SetBeatsToDo(10); e->SetInterBeatMS(0); e->SetStampInterval(1);
  geomqueen->CheckIn(e); images->Add(e);
  RndSMorphCreator* rs = new RndSMorphCreator;
  geomqueen->CheckIn(rs); e->Add(rs);
  rs->SetTarget(goo);
  rs->SetReportID(false);
  rs->SetGetResult(false);

  Board* board1 = new Board("Board1", "Showing GledLogo");
  geomqueen->CheckIn(board1);
  images->Add(board1);
  board1->SetColor(1, 0.7, 0.7);
  board1->SetTexture(image1);
  board1->SetULen(2.5*1.75); board1->SetVLen(2.5*1);
  board1->Set3Pos(-3, 2.5, 0.01);

  Board* board2 = new Board("Board2", "Showing Orchid");
  geomqueen->CheckIn(board2);
  images->Add(board2);
  board2->SetTexture(image2);
  board2->SetULen(2.5); board2->SetVLen(2.5);
  board2->Set3Pos(3, 2.5, 0.01);

  RectTerrain* terrain = new RectTerrain("Terrain");
  geomqueen->CheckIn(terrain);
  images->Add(terrain);
  terrain->SetFromImage(image3);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetUseScale(true);
  terrain->SetScales(1,1,0.5);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->SetOriginMode(RectTerrain::OM_Center);
  terrain->Set3Pos(0, -3, 0.01);


  CREATE_ADD_GLASS(n1, SMorph, images, "Earth", 0);
  n1->SetS(2);
  n1->SetTLevel(20); n1->SetPLevel(20);
  n1->SetTexture(image4);
  n1->Set3Pos(-6, -6, 0); n1->SetRotByAngles(0, TMath::Pi()/2, 0);

  CREATE_ADD_GLASS(n2, SMorph, images, "Moon", 0);
  n2->SetS(2);
  n2->SetTLevel(20); n2->SetPLevel(20);
  n2->SetTexture(image5);
  n2->Set3Pos( 6, -6, 0); n2->SetRotByAngles(0, TMath::Pi()/2, 0);

  SMorph* morphs[3];
  for(int i=0; i<3; ++i) {
    morphs[i] = new SMorph(Form("Morph %d", i+1));
    geomqueen->CheckIn(morphs[i]); images->Add(morphs[i]);
    morphs[i]->SetTLevel(20); morphs[i]->SetPLevel(20);
    morphs[i]->SetS(2);
    morphs[i]->SetTexture(image6);
    morphs[i]->Set3Pos(-6+6*i, 7, 0);
  }
  morphs[0]->SetTx(1); morphs[0]->SetCx(0.5); morphs[0]->SetRz(-0.25);
  morphs[1]->SetTx(0); morphs[1]->SetCx(-0.5); morphs[1]->SetRz(0.25);
  morphs[2]->SetTx(-1); morphs[2]->SetCx(0.25); morphs[2]->SetRz(0.25);

}
