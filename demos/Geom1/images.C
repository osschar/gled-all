// $Id$

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

  //----------------------------------------------------------------------------
  // Fonts

  ZList* fontdir = g_queen->AssertPath("var/fonts", "ZNameMap");

  CREATE_ADD_GLASS(lucida34, ZRlFont, g_queen, "LucidaBright 34", 0);
  lucida34->SetFontFile("lucidabright34.txf");

  //----------------------------------------------------------------------------
  // Images, textures

  ZList* texdir = g_queen->AssertPath("var/textures", "ZNameMap");

  CREATE_ADD_GLASS(image1, ZImage, texdir, "GledLogo", 0);
  image1->SetEnvMode(GL_MODULATE);
  image1->SetFile("gledlogo.png");

  CREATE_ADD_GLASS(image2, ZImage, texdir, "Orchid", 0);
  image2->SetFile("orchid.jpeg");

  CREATE_ADD_GLASS(image3, ZImage, texdir, "HeightField", 0);
  image3->SetFile("terrain_128.png");

  CREATE_ADD_GLASS(image4, ZImage, texdir, "Earth map", 0);
  image4->SetMagFilter(GL_LINEAR);
  image4->SetEnvMode(GL_MODULATE);
  image4->SetFile("earth.png");

  CREATE_ADD_GLASS(image5, ZImage, texdir, "Moon map", 0);
  image5->SetMagFilter(GL_LINEAR);
  image5->SetEnvMode(GL_MODULATE);
  image5->SetFile("moon.jpeg");

  CREATE_ADD_GLASS(image6, ZImage, texdir, "Checker", 0);
  image6->SetMagFilter(GL_LINEAR);
  image6->SetEnvMode(GL_MODULATE);
  image6->SetFile("checker_8.png");

  CREATE_ADD_GLASS(ribbon1, ZRibbon, texdir, "Ribbon1", 0);
  ribbon1->SetPOVFile("ribbon1.pov");
  ribbon1->LoadPOV();

  CREATE_ADD_GLASS(ribbon2, ZRibbon, texdir, "Ribbon2", 0);
  ribbon2->SetPOVFile("booby.pov");
  ribbon2->LoadPOV();

  //----------------------------------------------------------------------------
  // Shaders

  ZList* shdrdir = g_queen->AssertPath("var/shaders", "ZNameMap");

  CREATE_ADD_GLASS(brick_p, ZGlProgram, shdrdir, "Brick Program", 0);
  CREATE_ADD_GLASS(brick_v, ZGlShader,  brick_p, "Brick Vertex",  0);
  brick_v->Load("brick.vert");
  CREATE_ADD_GLASS(brick_f, ZGlShader,  brick_p, "Brick Fragment",  0);
  brick_f->Load("brick.frag");

  CREATE_ADD_GLASS(feigen_p, ZGlProgram, shdrdir, "Feigen Program", 0);
  CREATE_ADD_GLASS(feigen_f, ZGlShader,  feigen_p, "Feigen Fragment",  0);
  feigen_f->Load("feigen.frag");


  //============================================================================
  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  g_queen->CheckIn(base_plane);
  images->Add(base_plane);
  base_plane->SetColor(0.7, 0.7, 0.7);

  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.9, 0.9, 0.7);
  l->SetAmbient(0.11, 0.125, 0.16);
  l->SetLampScale(1);
  l->SetPos(0, -5, 10);
  l->RotateLF(1,2, TMath::Pi());
  g_queen->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);

  Board* board1 = new Board("Board1", "Showing GledLogo");
  g_queen->CheckIn(board1);
  images->Add(board1);
  board1->SetColor(1, 0.7, 0.7);
  board1->SetTexture(image1);
  board1->SetULen(2.5*1.75); board1->SetVLen(2.5*1);
  board1->SetPos(-4.8, 2.5, 0.01);

  Board* board2 = new Board("Board2", "Showing Orchid");
  g_queen->CheckIn(board2);
  images->Add(board2);
  board2->SetTexture(image2);
  board2->SetULen(2.5); board2->SetVLen(2.5);
  board2->SetPos(5.7, 2.5, 0.01);

  RectTerrain* terrain = new RectTerrain("Terrain");
  g_queen->CheckIn(terrain);
  images->Add(terrain);
  terrain->SetFromImage(image3);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->SetPos(-3, -7, 0.01);
  terrain->SetRibbon(ribbon1);
  terrain->SetRnrMode(RectTerrain::RM_FlatTring);

  CREATE_ADD_GLASS(n1, SMorph, images, "Earth", "");
  n1->SetScale(2);
  n1->SetTLevel(20); n1->SetPLevel(20);
  n1->SetTexture(image4);
  n1->SetPos(-6, -6, 0); n1->SetRotByDegrees(0, 90, 0);

  CREATE_ADD_GLASS(n2, SMorph, images, "Moon", "");
  n2->SetScale(2);
  n2->SetTLevel(20); n2->SetPLevel(20);
  n2->SetTexture(image5);
  n2->SetPos( 6, -6, 0); n2->SetRotByDegrees(0, 90, 0);

  CREATE_ADD_GLASS(mod, MD2Object, images, "Rei MD2", 0);
  mod->SetFile("md2/rei.md2");
  mod->SetFrameTime(175);
  mod->SetNormFac(-1); // This model has inverted normals.
  mod->Load();
  mod->SetScale(0.1);
  mod->SetUseScale(true);
  mod->SetPos(3, 2, 2.5);
  mod->SetRotByDegrees(-90, 0, 0);

  CREATE_ATT_GLASS(modtex, ZImage, mod, SetRnrMod, "Rei Texture", 0);
  modtex->SetFile("md2/rei.png");
  modtex->SetMagFilter(GL_LINEAR);
  modtex->SetEnvMode(GL_MODULATE);

  CREATE_ADD_GLASS(mod2, MD2Object, images, "Faerie MD2", 0);
  mod2->SetFile("md2/faerie.md2");
  mod2->SetFrameTime(107);
  mod2->Load();
  mod2->SetScales(0.1, 0.1, 0.08);
  mod2->SetUseScale(true);
  mod2->SetPos(-1, 2, 2);
  mod2->SetRotByDegrees(-90, 0, 0);

  CREATE_ATT_GLASS(modtex2, ZImage, mod2, SetRnrMod, "Faerie Texture", 0);
  modtex2->SetFile("md2/faerie-1.png");
  modtex2->SetMagFilter(GL_LINEAR);
  modtex2->SetEnvMode(GL_MODULATE);

  SMorph* morphs[5];
  for(int i = 0; i < 5; ++i) {
    morphs[i] = new SMorph(Form("Morph %d", i+1));
    g_queen->CheckIn(morphs[i]); images->Add(morphs[i]);
    morphs[i]->SetTLevel(30); morphs[i]->SetPLevel(30);
    morphs[i]->SetScale(2);
    morphs[i]->SetTexture(image6);
    morphs[i]->SetPos(-6+6*i, 7, 0);
  }
  morphs[0]->SetTx(1);  morphs[0]->SetCx(0.5);  morphs[0]->SetRz(-0.25);
  morphs[1]->SetTx(0);  morphs[1]->SetCx(-0.5); morphs[1]->SetRz(0.25);
  morphs[2]->SetTx(-1); morphs[2]->SetCx(0.25); morphs[2]->SetRz(0.25);

  morphs[3]->SetPos(-3, 8, 3);
  morphs[3]->SetRotByDegrees(0, 0, 90);
  morphs[3]->SetRnrMod(brick_p);

  morphs[4]->SetPos(3, 8, 3);
  morphs[4]->SetRotByDegrees(45, -30, 0);
  morphs[4]->SetRnrMod(feigen_p);

  CREATE_ADD_GLASS(txt1, Text, images, "Text1", 0);
  txt1->SetText("GLED");
  txt1->SetPos(1, 1.8, 0);
  txt1->SetScales(2.2, 2.3, 1);
  txt1->SetFillBack(true);
  txt1->SetFGCol(0, 0, 0); txt1->SetBGCol(1, 0.95, 0.6);
  txt1->SetFont(lucida34);

  CREATE_ADD_GLASS(txt2, Text, images, "Text2", 0);
  txt2->SetText("Generic Lightweight Environment for Distributed computing");
  txt2->SetPos(0, 0, 0);
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


  //============================================================================
  // Spawn GUI

  Gled::Macro("eye.C");
  setup_pupil_up_reference();
  g_nest->Add(g_queen->FindLensByPath("var"));
}
