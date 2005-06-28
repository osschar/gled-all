// $Header$

// copy of images.C with added metagui example
//

// vars: ZQueen* g_queen
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

GuiPupilInfo* guipupil = 0;

void metagui_test()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Geom1");

  CREATE_ADD_GLASS(lucida34, ZRlFont, g_queen, "LucidaBright 34", 0);
  lucida34->SetFontFile("lucidabright34.txf");

  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;

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
  terrain->SetOriginMode(RectTerrain::OM_Center);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->Set3Pos(0, -4, 0.01);
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
  CREATE_ADD_GLASS(rs_eventor, Eventor, images, "SMorph-Eventor", 0);
  rs_eventor->SetBeatsToDo(100);
  rs_eventor->SetInterBeatMS(100);
  CREATE_ADD_GLASS(rs_op, RndSMorphCreator, rs_eventor, "SMorph Creator", 0);
  rs_op->SetTarget(rs_node);

  // Rotator
  CREATE_ADD_GLASS(rot_eventor, Eventor, images, "Rotator-Eventor", 0);
  rot_eventor->SetInterBeatMS(30);
  CREATE_ADD_GLASS(rot_op, Mover, rot_eventor, "Rotator", 0);
  rot_op->SetRotateParams(1, 2, 0.01);

  //--------------------------------

  CREATE_ATT_GLASS(clipplane, ZGlClipPlane, morphs[1], SetRnrMod,
		   "ClipPlane", 0);

  //--------------------------------
  // Overlay
  //--------------------------------

  CREATE_ADD_GLASS(overlay, Scene, g_queen, "Overlay", 0);

  CREATE_ADD_GLASS(ovl_lamp, Lamp, overlay, "Ovl Lamp", 0);  
  ovl_lamp->Set3Pos(0, -5, 10);
  ovl_lamp->SetScale(1);
  overlay->GetGlobLamps()->Add(ovl_lamp);

  CREATE_ADD_GLASS(ovl_lm, ZGlLightModel, overlay, "LightOff", 0);
  ovl_lm->SetLightModelOp(0);

  CREATE_ADD_GLASS(nll, ZNodeListLink, overlay, "Element list", "Bar");
  nll->SetContents(images);

  nll->Set3Pos(0, 0, 4);

  nll->StandardFixed();

  nll->SetCbackAlpha(rot_op);
  nll->SetCbackMethodName("Mover::SetNode");

  //--------------------------------
  // Meta GUI
  //--------------------------------

  int W = 32;

  CREATE_ADD_GLASS(mg, MetaViewInfo, g_fire_queen, "Some MetaView", 0);
  mg->Size(W, 10);
  mg->SetExpertP(true);

  CREATE_ADD_GLASS(ms1, MetaSubViewInfo, mg, "ZGlass", 0);
  CREATE_ADD_GLASS(w11, MetaWeedInfo, ms1, "Name", 0);
  w11->Resize(6, 1, 26, 1);
  CREATE_ADD_GLASS(w12, MetaWeedInfo, ms1, "Title", 0);
  w12->Resize(6, 2, 26, 1);
  CREATE_ADD_GLASS(w13, MetaWeedInfo, ms1, "<box>", "ZGlass: basic information, like name and title:");
  w13->Resize(0, 0, W, 1);
  w13->Color(1, .7, .8);
  w13->Align(true, -1, 0);
  w13->Box(MetaWeedInfo::BT_Engraved);

  CREATE_ADD_GLASS(ms2, MetaSubViewInfo, mg, "ZNode", 0);
  ms2->Position(0, 3);
  CREATE_ADD_GLASS(w20, MetaWeedInfo, ms2, "<box>", "ZNode: position and rotation:");
  w20->Resize(0, 0, W, 1);
  w20->Color(1, .7, .8);
  w20->Align(true, -1, 0);
  w20->Box(MetaWeedInfo::BT_Engraved);
  CREATE_ADD_GLASS(w21, MetaWeedInfo, ms2, "Pos", 0);
  w21->Resize(0, 2, W, 2);
  w21->Label("Drag mouse for ultimate enjointment.");
  w21->Align(false, 0, -1);
  CREATE_ADD_GLASS(w22, MetaWeedInfo, ms2, "Rot", 0);
  w22->Resize(0, 4, W, 2);
  w22->Label("middle button x10, right x100");
  w22->Align(false, 0, 1);

  //--------------------------------

  Gled::AssertMacro("gled_view_globals.C");
  Gled::LoadMacro("eye.C");

  ZList* laytop = register_GledCore_layouts();
  laytop->Swallow("MetaGui", new ZGlass ("WeedView",
   "ZGlass(Name[12]) : MetaWeedInfo(*)"));

  Text_t* eye_name   = "Eye";
  Text_t* shell_name = "Shell";
  Text_t* pupil_name = "Pupil";

  ASSIGN_ADD_GLASS(g_shell, ShellInfo, g_fire_queen, shell_name, "");

  ASSIGN_ATT_GLASS(g_nest, NestInfo, g_shell, AddSubShell, "Nest", 0);
  //g_nest->Add(images);
  g_nest->ImportKings();

  ASSIGN_ATT_GLASS(guipupil, GuiPupilInfo, g_shell, AddSubShell, "GuiPupil", "");

  ASSIGN_ATT_GLASS(g_pupil, PupilInfo, guipupil, SetPupil, "Pupil", "");
  g_pupil->Add(images);
  g_pupil->SetUpReference(images);
  g_pupil->SetUpRefAxis(3);

  g_pupil->SetOverlay(overlay);
  g_pupil->SetEventHandler(nll);

  g_shell->SetDefSubShell(g_nest);

  g_eye = Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");

  g_shell->SpawnMetaGui(morphs[1], mg);

  rot_eventor->Start();
}
