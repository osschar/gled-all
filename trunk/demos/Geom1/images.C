// $Header$

// simple scene with images and RectTerrain
//
// crystal.png bacteria.png are 512x512 AFM scans, eg:
//   image3->SetFile("bacteria.png");
//   image3->Load();
//   terrain->SetFromImage(image3);

// vars: ZQueen* g_queen
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

void images()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;

  CREATE_ADD_GLASS(lucida34, ZRlFont, g_queen, "LucidaBright 34", 0);
  lucida34->SetFontFile("lucidabright34.txf");

  // Images

  CREATE_ADD_GLASS(image1, ZImage, images, "GledLogo", 0);
  image1->SetFile("gledlogo.png");
  image1->Load();
  image1->SetEnvMode(GL_MODULATE);
  image1->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, images, "Orchid", 0);
  image2->SetFile("orchid.jpeg");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image3, ZImage, images, "HeightField", 0);
  image3->SetFile("terrain_128.png");
  image3->Load();
  image3->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image4, ZImage, images, "Earth map", 0);
  image4->SetMagFilter(GL_LINEAR);
  image4->SetEnvMode(GL_MODULATE);
  image4->SetFile("earth.png");
  image4->Load();
  image4->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image5, ZImage, images, "Moon map", 0);
  image5->SetMagFilter(GL_LINEAR);
  image5->SetEnvMode(GL_MODULATE);
  image5->SetFile("moon.jpeg");
  image5->Load();
  image5->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image6, ZImage, images, "Checker", 0);
  image6->SetMagFilter(GL_LINEAR);
  image6->SetEnvMode(GL_MODULATE);
  image6->SetFile("checker_8.png");
  image6->Load();
  image6->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(ribbon1, ZRibbon, images, "Ribbon1", 0);
  ribbon1->SetPOVFile("ribbon1.pov");
  ribbon1->LoadPOV();

  CREATE_ADD_GLASS(ribbon2, ZRibbon, images, "Ribbon2", 0);
  ribbon2->SetPOVFile("booby.pov");
  ribbon2->LoadPOV();

  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  g_queen->CheckIn(base_plane);
  images->Add(base_plane);
  base_plane->SetColor(0.7, 0.7, 0.7);

  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetScale(1);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  g_queen->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);

  Board* board1 = new Board("Board1", "Showing GledLogo");
  g_queen->CheckIn(board1);
  images->Add(board1);
  board1->SetColor(1, 0.7, 0.7);
  board1->SetTexture(image1);
  board1->SetULen(2.5*1.75); board1->SetVLen(2.5*1);
  board1->Set3Pos(-4.8, 2.5, 0.01);

  Board* board2 = new Board("Board2", "Showing Orchid");
  g_queen->CheckIn(board2);
  images->Add(board2);
  board2->SetTexture(image2);
  board2->SetULen(2.5); board2->SetVLen(2.5);
  board2->Set3Pos(5.7, 2.5, 0.01);

  RectTerrain* terrain = new RectTerrain("Terrain");
  g_queen->CheckIn(terrain);
  images->Add(terrain);
  terrain->SetFromImage(image3);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->Set3Pos(-3, -7, 0.01);
  terrain->SetRibbon(ribbon1);

  CREATE_ADD_GLASS(n1, SMorph, images, "Earth", "");
  n1->SetS(2);
  n1->SetTLevel(20); n1->SetPLevel(20);
  n1->SetTexture(image4);
  n1->Set3Pos(-6, -6, 0); n1->SetRotByAngles(0, TMath::Pi()/2, 0);

  CREATE_ADD_GLASS(n2, SMorph, images, "Moon", "");
  n2->SetS(2);
  n2->SetTLevel(20); n2->SetPLevel(20);
  n2->SetTexture(image5);
  n2->Set3Pos( 6, -6, 0); n2->SetRotByAngles(0, TMath::Pi()/2, 0);

  SMorph* morphs[3];
  for(int i=0; i<3; ++i) {
    morphs[i] = new SMorph(Form("Morph %d", i+1));
    g_queen->CheckIn(morphs[i]); images->Add(morphs[i]);
    morphs[i]->SetTLevel(30); morphs[i]->SetPLevel(30);
    morphs[i]->SetS(2);
    morphs[i]->SetTexture(image6);
    morphs[i]->Set3Pos(-6+6*i, 7, 0);
  }
  morphs[0]->SetTx(1);  morphs[0]->SetCx(0.5);  morphs[0]->SetRz(-0.25);
  morphs[1]->SetTx(0);  morphs[1]->SetCx(-0.5); morphs[1]->SetRz(0.25);
  morphs[2]->SetTx(-1); morphs[2]->SetCx(0.25); morphs[2]->SetRz(0.25);

  CREATE_ADD_GLASS(txt1, Text, images, "Text1", 0);
  txt1->SetText("GLED");
  txt1->Set3Pos(1, 1.8, 0);
  txt1->SetScales(2.2, 2.3, 1);
  txt1->SetFillBack(true);
  txt1->SetFGCol(0, 0, 0); txt1->SetBGCol(1, 0.95, 0.6);
  txt1->SetFont(lucida34);

  CREATE_ADD_GLASS(txt2, Text, images, "Text2", 0);
  txt2->SetText("Generic Lightweight Environment for Distributed computing");
  txt2->Set3Pos(0, 0, 0);
  txt2->SetScales(0.67, 1.1, 1);
  txt2->SetFillBack(true);
  txt2->SetFGCol(0,0,0); txt2->SetBGCol(0.65, 0.8, 1);

  // RndSmorphCreator
  CREATE_ADD_GLASS(rs_node, ZNode, images, "RndSmorph Node", 0);
  CREATE_ADD_GLASS(rs_eventor, Eventor, images, "Eventor", 0);
  rs_eventor->SetBeatsToDo(100);
  rs_eventor->SetInterBeatMS(100);
  CREATE_ADD_GLASS(rs_op, RndSMorphCreator, rs_eventor, "SMorph Creator", 0);
  rs_op->SetTarget(rs_node);


  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

}
