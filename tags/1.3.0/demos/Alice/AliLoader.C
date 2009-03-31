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

/*
  Racoon import:

aliload->SelectParticles("fStatusCode <= 1 && Pt() > 1 && abs(Eta()) < 2 && GetPDG()->Charge() != 0")
aliload->SelectParticles("fStatusCode <= 1 && P() > 1 && Eta() > 2")
aliload->SelectParticles("fStatusCode <= 1 && Pt() > 0.5 && abs(Eta()) < 2 && GetPDG()->Charge() != 0")
aliload->SelectParticles("fStatusCode <= 1 && Pt() > 0.5 && abs(Eta()) < 3 && GetPDG()->Charge() != 0")

*/

/**************************************************************************/

class MultiBoard;
class ZImage;

/**************************************************************************/

void AliLoader(const Text_t* dirname = 0)
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

  CREATE_ADD_GLASS(image2, ZImage, var, "LineDefault", 0);
  image2->SetMagFilter(GL_LINEAR);
  image2->SetEnvMode(GL_MODULATE);
  image2->SetFile(file_grep("images/lin_fssccw.png"));
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image3, ZImage, var, "LineTrackExplosion", 0);
  image3->SetMagFilter(GL_LINEAR);
  image3->SetFile(file_grep("images/lin_track_explosion-3.png"));
  image3->Load();
  image3->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image4, ZImage, var, "LineColdsteel", 0);
  image4->SetMagFilter(GL_LINEAR);
  image4->SetEnvMode(GL_MODULATE);
  image4->SetFile(file_grep("images/lin_coldsteel.png"));
  image4->Load();
  image4->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(mcrst, MCTrackRnrStyle, var, "MC Track RnrStyle", 0);
  mcrst->SetTexture(image3);
  CREATE_ADD_GLASS(mcrst2, MCTrackRnrStyle, var, "MC Track RnrStyle 2", 0);
  mcrst2->SetTexture(image4);
  CREATE_ADD_GLASS(recrst, RecTrackRS, var, "Rec Track RnrStyle", 0);

  CREATE_ADD_GLASS(ribbon1, ZRibbon, var, "Ribbon1", 0);
  ribbon1->SetPOVFile(file_grep("images/romania.pov"));
  ribbon1->LoadPOV();

  CREATE_ADD_GLASS(giis, GIImportStyle, var, "ImportMode", 0);


  ZAliLoad* al = new ZAliLoad();
  aliload = al;
  al->SetOM(-2);
  al->SetUseOM(true);
  g_queen->CheckIn(al);
  g_scene->Add(al);


  ZList* aliconf = g_scene->GetQueen()->AssertPath("Etc/Alice", "ZNameMap");

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

  // NestInfo* geo_nest = new_nest("Geometry's nest", geometry_mini_layout);
  // geo_nest->Add(g_simple_geometry);
  // geo_nest->SetDefW(66);

  spawn_default_gui();

  Gled::LoadMacro("zaliload_metagui.C");
  MetaViewInfo* mvi = make_zaliload_metagui();
  // mvi->SetExpertP(true);
  g_shell->SpawnMetaGui(al, mvi);

  if(dirname != 0) {
    al->SetDataDir(dirname);
    // Shoot a MIR to have functional GUI during processing:
    ZMIR* setup_mir = al->S_LoadVSD();
    g_saturn->ShootMIR(setup_mir);
  }
}
