/// $Header$
//
//
// vars: ZQueen* g_queen
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

#include "alice_simple.C"

class ZGeoNode;
class GeoUserData;
class ZAliLoad;

ZGeoNode*     volt      = 0;
Scene*        gscene    = 0;
ZAliLoad*     aliload   = 0;

PupilInfo*    movie_pupil = 0;

// default event display layout
const Text_t* particle_layout =
  "ZNode(RnrSelf[4],RnrElements[4],RnrMod[8]):"
  "TrackBase(V[10],P[14]):"
  "MCTrack(NDaughters[4],ImportDaughtersRec[5],VDecay[10],ImportHits[4],Dump[4])";
  //"MCTrackRnrStyle(VertexColor[4],RnrP[4])";
  // "HitContainer(NHits[4],Color[4],Dump[4])";

/*
  PP kine selection example:
  fStatusCode<=1 && Pt()>0.4 && abs(Eta())<1 && GetPDG()->Charge()!=0

*/

/**************************************************************************/

class MultiBoard;
class ZImage;

class WSSeed;
void setup_camera_path(WSSeed* s, Int_t N,
		       Double_t x0, Double_t y0, Double_t z0,
		       Double_t dphi, Double_t dr0,
		       Double_t t_start_dr, Double_t max_r);

void rnr_off(ZGlass* top, const Text_t* path);

/**************************************************************************/

void AliAnimator(const Text_t* dirname = 0,
		 Bool_t use_aliroot    = false)
{
  // gSystem->IgnoreSignal(kSigSegmentationViolation, true);
  Gled::AssertMacro("sun.C");

  g_queen = new ZQueen(512*1024, "AliceQueen", "Goddess of Ver");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);

  g_queen->SetAuthMode(ZQueen::AM_None);
  g_queen->SetMapNoneTo(ZMirFilter::R_Allow);

  Gled::AssertLibSet("Geom1");
  Gled::AssertLibSet("RootGeo");

  //gROOT->Macro("loadlibs.C");
  Gled::AssertMacro("loadlibs.C");
  Gled::AssertLibSet("Alice");

  // gROOT->LoadMacro("alice_simple.C");
  // Gled::LoadMacro("alice_simple.C");

  // Scene returned in g_scene.
  alice_simple_init("alice_minigeo.root", "def_geoview.root");
  // alice_simple_init("alice_fullgeo.root", "def_geoview.root");

  ZNode* geo_top = (ZNode*) g_scene->FindLensByPath("Geometry");
  CREATE_ATT_GLASS(pick_off, ZRlNameStack, geo_top, SetRnrMod, "Picking Off", 0);
  pick_off->SetClearStack(true); pick_off->SetRestoreStack(true);
  pick_off->SetNameStackOp(0);

  ZNode* var = (ZNode*) g_scene->FindLensByPath("Var");

  CREATE_ADD_GLASS(image, ZImage, var, "Checker", 0);
  image->SetMagFilter(GL_LINEAR);
  image->SetEnvMode(GL_MODULATE);
  image->SetFile(file_grep("images/checker_8.png"));
  image->Load();
  image->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, var, "LineTex", 0);
  image2->SetMagFilter(GL_LINEAR);
  image2->SetEnvMode(GL_MODULATE);
  image2->SetFile(file_grep("images/lin_fssccw.png"));
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(mcrst, MCTrackRnrStyle, var, "MC Track RnrStyle", 0);
  mcrst->SetTrackWidth(2);
  mcrst->SetTexture(image2);
  CREATE_ADD_GLASS(recrst, RecTrackRS, var, "Rec Track RnrStyle", 0);

  CREATE_ADD_GLASS(ribbon1, ZRibbon, var, "Ribbon1", 0);
  ribbon1->SetPOVFile(file_grep("images/romania.pov"));
  ribbon1->LoadPOV();

  CREATE_ADD_GLASS(giis, GIImportStyle, var, "ImportMode", 0);


  ASSIGN_ADD_GLASS(aliload, ZAliLoad, g_scene, "ZAliLoad", 0);
  aliload->SetOM(-2);
  aliload->SetUseOM(true);

  /**************************************************************************/
  // Anim
  /**************************************************************************/

  CREATE_ADD_GLASS(tclist, ZList, g_queen, "TimeConsumerList", 0);
  tclist->Add(mcrst);

  CREATE_ADD_GLASS(stxt, TimeScreenText, var, "TimeScreenText", 0);
  stxt->SetX(-1);
  stxt->SetFormat("time: %4.1f ns");
  tclist->Add(stxt);

  /**************************************************************************/

  CREATE_ADD_GLASS(etor, Eventor, g_queen, "Animator", 0);
  etor->SetEpochType(Eventor::ET_Manual);
  etor->SetTimeSource(Eventor::TS_IntStep);
  etor->SetBeatsToDo(1750);
  etor->SetTimeStep(0.02);

  CREATE_ADD_GLASS(tmaker, TimeMaker, etor, "TimeMaker", 0);
  tmaker->SetMinT(0);
  tmaker->SetMaxT(1000);
  tmaker->SetClients(tclist);

  etor->Add(g_scene->FindLensByPath("Lamp Origin/Dynamo/Lamp Origin Rotator"));

  CREATE_ADD_GLASS(sdumper, ScreenDumper, etor, "ScreenDumper", 0);
  sdumper->SetDumpImage(true);
  sdumper->SetFileNameFmt("/foo/kabl/mXXX/img-%04d");
  sdumper->SetWaitSignal(true);

  /**************************************************************************/

  Double_t fade_time = 30;
  Double_t max_time  = 35;


  CREATE_ADD_GLASS(mpupil, PupilInfo, g_queen, "MoviePupil", 0);
  movie_pupil = mpupil; // assign to global
  mpupil->SetAutoRedraw(false);
  mpupil->SetupZFov(65);
  mpupil->SetBlend(true);
  mpupil->SetShowView(false);
  mpupil->Add(g_scene);
  mpupil->SetUpReference(g_scene);
  mpupil->SetUpRefAxis(2);

  sdumper->SetPupil(mpupil);

  mcrst->SetCheckT(true);
  mcrst->SetMaxT(0.1);
  mcrst->SetFadeT(fade_time);
  mcrst->SetKillT(max_time);
  mcrst->SetAnimDeltaT(10);
  mcrst->SetTScale(-9);
  mcrst->SetTrackWidth(1.6);

  // Camera movement
  //--------------------------------

  CREATE_ATT_GLASS(wker, WSWalker, tmaker, AddClient, "WSWalker", 0);

  CREATE_ADD_GLASS(cam_path, WSSeed, g_scene, "Camera Path", 0);
  Int_t travel_time = fade_time;
  setup_camera_path(cam_path, travel_time,
		    -2, 0, 0,
		    -TMath::TwoPi()/travel_time, 0.4,
		    2, 10);
  cam_path->RotateLF(2, 3, TMath::PiOver2());
  cam_path->MovePF(2, 1);
  cam_path->SetFat(false);

  CREATE_ADD_GLASS(traveler, SMorph, cam_path, "Travel SMorph", 0);
  traveler->SetScale(0.25);

  wker->SetSeed(cam_path);
  wker->SetNode(traveler);
  wker->SetTime(0);

  mpupil->SetCameraBase(traveler);
  mpupil->SetLookAt(g_scene);

  // Fix things
  //--------------------------------

  // Geometry line width.
  ZGlBlending* zglb = dynamic_cast<ZGlBlending*>(var->FindLensByPath("Blending"));
  zglb->SetLineWidth(2);

  /**************************************************************************/
  // End Anim
  /**************************************************************************/


  ZList* aliconf = g_scene->GetQueen()->AssertPath("Etc/Alice", "ZNameMap");
  CREATE_ATT_GLASS(tpclist, ZRnrModList, aliconf, Swallow, "TPC_RM_list", 0);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, tpclist, "TPC Light Model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(0);

  CREATE_ADD_GLASS(tpcrnrmod, TPCSegRnrMod, tpclist, "TPC RnrMod", 0);
  tpcrnrmod->SetRnrFrame(1);
  tpcrnrmod->SetRibbon(ribbon1);

  CREATE_ADD_GLASS(itsrnrmod, ITSDigRnrMod, var, "ITS RnrMod", 0);
  itsrnrmod->SetRnrFrame(1);
  itsrnrmod->SetRibbon(ribbon1);

  CREATE_ADD_GLASS(tofrnrmod, TOFDigRnrMod, var, "TOF RnrMod", 0);
  tofrnrmod->SetRnrModFrame(1);
  tofrnrmod->SetRibbon(ribbon1);

  //--------------------------------------------------------------

  // leave geometry as default
  default_nest_layout = particle_layout;
  setup_default_gui();
  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("Alice", "ZList");
    l->Swallow(new ZGlass("Particle", particle_layout));
  }

  spawn_default_gui();

  Gled::LoadMacro("zaliload_metagui.C");
  MetaViewInfo* mvi = make_zaliload_metagui();
  mvi->SetExpertP(true);
  g_shell->SpawnMetaGui(aliload, mvi);

  // Movie!!
  g_shell->AddSubShell(mpupil);
  mpupil->SetWidth(800);
  mpupil->SetHeight(600);
  g_nest->ImportKings();

  if(dirname != 0) {
    aliload->SetDataDir(dirname);
    // Shoot a MIR to have functional GUI during processing:
    ZMIR* setup_mir = aliload->S_LoadVSD();
    g_saturn->ShootMIR(setup_mir);
  }
}

void prepare_for_capture()
{
  rnr_off(g_scene, "Camera Path");
  rnr_off(g_scene, "Lamp Origin");
  rnr_off(g_scene, "Markers");

  printf("Importing particles ...\n");
  aliload->SelectParticles(0, true);
  printf("Turning off display lists ...\n");
  aliload->SetUseDLRec(false);
  printf("Done.\n");
}

/**************************************************************************/
/**************************************************************************/

void setup_camera_path(WSSeed* s, Int_t N,
		       Double_t x0, Double_t y0, Double_t z0,
		       Double_t dphi, Double_t dr0,
		       Double_t t_start_dr, Double_t max_r)
{
  // !!! So far ignoring t_start_dr and max_r !!!

  Double_t t   = 0;
  Double_t r   = TMath::Sqrt(x0*x0 + y0*y0);
  Double_t phi = TMath::ATan2(y0, x0);

  for(Int_t i=0; i<=N; ++i) {
    WSPoint* p = new WSPoint(GForm("Point %d", i));
    double ss = TMath::Sin(phi);
    double cc = TMath::Cos(phi);
    p->SetPos(r*cc, r*ss, z0);

    Double_t dr = 0;
    if(t >= t_start_dr) dr = dr0;
    if(r + dr > max_r)  dr = max_r - r;

    Double_t der_phi  = TMath::ATan2(dr*ss + r*dphi*cc, dr*cc - r*dphi*ss);
    p->RotateLF(1, 2, der_phi);

    g_queen->CheckIn(p);
    s->Add(p);

    phi += dphi;
    r   += dr;
    t   += 1;
  }
}

void rnr_off(ZGlass* top, const Text_t* path)
{
  ZNode* n = dynamic_cast<ZNode*>(top->FindLensByPath(path));
  if(n) {
    n->SetRnrSelf(false);
    n->SetRnrElements(false);
  } else {
    printf("WRN: rnr_off '%s' not found under %s.\n", path, top->Identify().Data());
  }
}

