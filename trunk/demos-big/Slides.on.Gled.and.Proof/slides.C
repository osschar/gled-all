// $Header$

// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

#include <memory>

class Amphitheatre;
class MultiBoard;
class ZImage;

/**************************************************************************/
// main
/**************************************************************************/

void slides(Bool_t start_hunt_p=true)
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

  CREATE_ADD_GLASS(ss, Scene, scenes, "Slide Scene", "Gled and PROOF talk");


  // Basic scene elements
  //----------------------

  CREATE_ADD_GLASS(cam_base, Sphere, ss, "CameraBase", 0);
  cam_base->Set3Pos(5, -5, 4);
  cam_base->SetRotByDegrees(140, -26, 0);
  cam_base->SetRadius(0.01);

  CREATE_ADD_GLASS(l, Lamp, ss, "Lamp", 0);
  l->SetScale(1);
  l->Set3Pos(-4, -3, 10);
  l->SetAmbient(0.4, 0.4, 0.4);
  ss->GetGlobLamps()->Add(l);

  //--------------------------------

  CREATE_ADD_GLASS(blend, ZGlBlending, ss, "Line Smoothing", 0);
  blend->SetBlendOp(ZGlStateBase::GSO_On);
  blend->SetAntiAliasOp(ZGlStateBase::GSO_On);
  blend->SetLineWidth(1);
  blend->SetPointSize(16);

  CREATE_ADD_GLASS(plane_xy, Rect, ss, "Plane XY", 0);
  plane_xy->SetUnitSquare(20);
  plane_xy->SetColor(0,0,1,0.5);
  plane_xy->Set3Pos(0,0,-0.05);
  plane_xy->SetRotByDegrees(0,0,0);
  plane_xy->SetWidth(1);

  //--------------------------------

  // gled_promo(ss);

  CREATE_ADD_GLASS(amph, Amphitheatre, ss, "Amphitheatre", 0);
  demo_amphitheatre(amph);

  MultiBoard* mtalk = matevz_talk(ss);
  mtalk->Set3Pos(-2, -3.5, 2.2);
  mtalk->SetUV(6, 4.5);
  mtalk->SetRotByDegrees(90, 0, 90);
  mtalk->First();

  MultiBoard* ftalk = fons_talk(ss);
  ftalk->Set3Pos(-2, 3.5, 2.2);
  ftalk->SetUV(6, 4.5);
  ftalk->SetRotByDegrees(90, 0, 90);
  ftalk->First();

  /**************************************************************************/
  // Spawn the eye
  /**************************************************************************/

  printf("Instantiating GUI ...\n");

  CREATE_ADD_GLASS(shell, ShellInfo, fire_queen, "Slide Shell", 0);
  // shell->ImportKings();   // Get all Kings as top level objects
  shell->Add(ss);

  CREATE_ADD_GLASS(pupil, PupilInfo, shell->GetPupils(), "Pupil", 0);
  pupil->Add(ss);
  pupil->SetFOV(90);
  pupil->SetWidth(1024);
  pupil->SetHeight(600);
  pupil->SetClearColor(0.06, 0.07, 0.06);
  pupil->SetTileCol(0.3, 0, 0.3, 0.5);

  pupil->SetCameraBase(cam_base);
  pupil->SetUpReference(ss);

  CREATE_ADD_GLASS(etor, Eventor, ss, "Movie Maker", 0);
  etor->SetBeatsToDo(-1);
  etor->SetInterBeatMS(400); etor->SetStampInterval(10);
  CREATE_ADD_GLASS(sdumper, ScreenDumper, etor, "DumpRayEmitter", 0);
  sdumper->SetPupil(pupil);
  // sdumper->SetFileNameFmt("someplace/img%05d.tga");

  Gled::theOne->SpawnEye(shell, "Eye of magick");

  if(start_hunt_p) {
    gSystem->Sleep(1000);
    amph->StartHunt();
  }
}

/**************************************************************************/
// Lens-init foos
/**************************************************************************/

void init_texture(ZImage* tex, const char* fname, bool loadp=true)
{
  tex->SetMagFilter(GL_LINEAR);
  tex->SetMinFilter(GL_LINEAR);
  tex->SetEnvMode(GL_MODULATE);
  tex->SetFile(fname);
  if(loadp) {
    tex->Load();
    tex->SetLoadAdEnlight(true);
  }
}

/**************************************************************************/
// Scene elements
/**************************************************************************/

void gled_promo(ZList* list)
{
  CREATE_ADD_GLASS(txt1, Text, list, "Text1", 0);
  txt1->SetText("GLED");
  txt1->Set3Pos(0, 5, 1.55);
  txt1->SetRotByDegrees(0, 0, 90);
  txt1->SetScales(1.8, 1.2, 1);
  txt1->SetCenter(false);
  txt1->SetFillBack(true);
  txt1->SetFGCol(0, 0, 0); txt1->SetBGCol(1, 0.95, 0.6);

  CREATE_ADD_GLASS(txt2, Text, list, "Text2", 0);
  txt2->SetText("Generic Lightweight Environment for Distributed computing");
  txt2->Set3Pos(0, 5, 0.3);
  txt2->SetRotByDegrees(0, 0, 90);
  txt2->SetScales(0.5, 1, 1);
  txt2->SetCenter(false);
  txt2->SetFillBack(true);
  txt2->SetFGCol(0,0,0); txt2->SetBGCol(0.65, 0.8, 1);
}

/**************************************************************************/

MultiBoard* fons_talk(ZList* list)
{
  printf("Loading GridKA Fons' talk ...\n");
  CREATE_ADD_GLASS(multiboard, MultiBoard, list, "Fons-GridKA", 0);
  for(int i=0; i<40; i++) {
    ZImage* img = new ZImage(GForm("Page %d", i+1));
    img->SetFile(GForm("fons-gridka/page.%d.png", i));
    img->SetMagFilter(GL_LINEAR);
    img->Load();
    scenes->CheckIn(img);
    multiboard->GetSlides()->Add(img);
  }

  CREATE_ADD_GLASS(talkview, Sphere, multiboard, "Talk Viewpoint", 0);
  talkview->SetRotByDegrees(90, -90, 0);
  talkview->MoveLF(1, -4.5/2);
  talkview->SetRadius(0.01);

  return multiboard;
}

/**************************************************************************/

MultiBoard* matevz_talk(ZList* list)
{
  printf("Loading ALICE Offline GLED talk ...\n");
  CREATE_ADD_GLASS(multiboard, MultiBoard, list, "Gled Talk", 0);
  for(int i=0; i<10; i++) {
    ZImage* img = new ZImage(GForm("Page %d", i));
    img->SetFile(GForm("matevz-gled/page.%d.png", i));
    img->SetMagFilter(GL_LINEAR);
    img->SetMinFilter(GL_LINEAR);
    img->Load();
    img->SetLoadAdEnlight(true);
    scenes->CheckIn(img);
    multiboard->GetSlides()->Add(img);
  }

  CREATE_ADD_GLASS(talkview, Sphere, multiboard, "Talk Viewpoint", 0);
  talkview->SetRotByDegrees(90, -90, 0);
  talkview->MoveLF(1, -4.5/2);
  talkview->SetRadius(0.01);

  return multiboard;
}

/**************************************************************************/

void demo_amphitheatre(Amphitheatre* amph)
{
  amph->SetGuestStep(0.05);
  amph->SetStepSleepMS(50);

  amph->CreateChairs(6, 0, 2,    90, 26);
  amph->CreateChairs(5, 0, 1,    90, 20);
  amph->CreateChairs(4, 0, 0.5,  90, 16);
  amph->CreateChairs(3, 0, 0.25, 90, 12);
  amph->CreateChairs(2, 0, 0,    90, 8);

  CREATE_ADD_GLASS(occ1, Sphere, amph, "A Big Guest", 0);
  occ1->SetRadius(0.2);
  occ1->Set3Pos(0, 0.25, 0);
  occ1->SetColor(0, 0, 1);
  amph->AddGuest(occ1);
  CREATE_ADD_GLASS(occ2, Sphere, amph, "Big Guest's Wife", 0);
  occ2->SetRadius(0.25);
  occ2->Set3Pos(0, -0.25, 0);
  occ2->SetColor(1, 0, 0);
  amph->AddGuest(occ2);

  amph->MakeRandomGuests(80, 5);
}

/**************************************************************************/
// end
/**************************************************************************/
