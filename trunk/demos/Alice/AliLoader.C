/// $Header$

// 
//
// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

class ZGeoNode;
class GeoUserData;
class ZAliLoad;

ZGeoNode* volt    = 0;
ZAliLoad* aliload = 0;
Scene*    gscene  = 0;

// default event display layout
const Text_t* particle_layout =
  "ZNode(RnrSelf[4],RnrElements[4],RnrMod[8]):"
  "MCTrack(NDaughters[4],ImportDaughtersRec[5],V[10],P[14],VDecay[10],ImportHitsFromPrimary[4],Dump[4])";
  //"MCTrackRnrStyle(VertexColor[4],RnrP[4])";
  // "HitContainer(NHits[4],Color[4],Dump[4])";

/*
  PP kine selection example:
  fStatusCode<=1 && Pt()>0.4 && abs(Eta())<1 && GetPDG()->Charge()!=0

*/

/**************************************************************************/

class MultiBoard;
class ZImage;

/**************************************************************************/

void AliLoader(const Text_t* dirname = 0,
	       Bool_t use_aliroot    = 0,
	       Bool_t geninfo        = 0)
{
  gSystem->IgnoreSignal(kSigSegmentationViolation, true);
  if(Gled::theOne->GetSaturn() == 0) gROOT->Macro("sun.C");

  scenes->SetAuthMode(ZQueen::AM_None);
  scenes->SetMapNoneTo(ZMirFilter::R_Allow);

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  gROOT->Macro("loadlibs.C");
  Gled::theOne->AssertLibSet("Alice");

  gROOT->LoadMacro("alice_simple.C");

  Scene* rscene = alice_simple_init("alice_minigeo.root",
				    "def_geoview.root");
  
  ZNode* var = (ZNode*) rscene->FindLensByPath("Var");

  CREATE_ADD_GLASS(image, ZImage, var, "Checker", 0);
  image->SetMagFilter(GL_LINEAR);
  image->SetEnvMode(GL_MODULATE);
  image->SetFile("images/checker_8.png");
  image->Load();
  image->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(image2, ZImage, var, "LineTex", 0);
  image2->SetMagFilter(GL_LINEAR);
  image2->SetEnvMode(GL_MODULATE);
  image2->SetFile("images/lin_fssccw.png");
  image2->Load();
  image2->SetLoadAdEnlight(true);

  CREATE_ADD_GLASS(rst, MCTrackRnrStyle, var, "MCParticle RnrStyle", 0);
  rst->SetTexture(image2);

  CREATE_ADD_GLASS(ribbon1, ZRibbon, var, "Ribbon1", 0);
  ribbon1->SetPOVFile("images/romania.pov");
  ribbon1->LoadPOV();

  //CREATE_ADD_GLASS(giis, GIImportStyle, var, "GenInfo Import", 0);

  ZAliLoad* al = new ZAliLoad(); 
  aliload = al;
  al->SetOM(-2);
  al->SetUseOM(true);
  al->SetRnrMod(rst);
  //al->SetGIIStyle(giis);
  scenes->CheckIn(al);
  rscene->Add(al);

  if(dirname != 0) {
    al->SetDataDir(dirname);
    al->SetupDataSource(use_aliroot);
  }

  ZList* aliconf = rscene->GetQueen()->AssertPath("Etc/Alice", "ZNameMap");
  CREATE_ATT_GLASS(tpclist, ZRnrModList, aliconf, Swallow, "TPC_RM_list", 0);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, tpclist, "TPC Light Model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(0);

  
  CREATE_ADD_GLASS(tpcrnrmod, TPCSegRnrMod, tpclist, "TPC RnrMod", 0);
  tpcrnrmod->SetRnrFrame(1);
  tpcrnrmod->SetRibbon(ribbon1);

  //--------------------------------------------------------------

  // leave geometry as default
  default_nest_layout = particle_layout;
  setup_default_gui(rscene);
  {
    ZList* l = fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("Alice", "ZList");
    l->Swallow(new ZGlass("Particle", particle_layout));
  }

  // NestInfo* geo_nest = new_nest("Geometry's nest", geometry_mini_layout);
  // geo_nest->Add(g_simple_geometry);
  // geo_nest->SetDefW(66);

  MetaViewInfo* mvi = make_zaliload_metagui();
  // mvi->SetExpertP(true);

  spawn_default_gui();

  shell->SpawnMetaGui(al, mvi);
}

/**************************************************************************/
// Stripped down ZAliLoad view.
/**************************************************************************/

MetaViewInfo* make_zaliload_metagui()
{
  int Y = 0, W = 32, H = 19;

  CREATE_ADD_GLASS(mv, MetaViewInfo, fire_queen, "MetaGui for ZAliLoad", 0);
  mv->Size(W, H);

  int y = 0;

  // DataDir/Event handling
  CREATE_ADD_GLASS(ms1, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms1->Position(0, 0);
  // Intro
  CREATE_ADD_GLASS(w10, MetaWeedInfo, ms1, "<box>", "Data Source control:");
  w10->Resize(0, y, W, 1);
  w10->Color(0.85, 0.7, 0.7);
  w10->Align(true, -1, 0);
  w10->Box(MetaWeedInfo::BT_Engraved);
  y++;
  // members
  CREATE_ADD_GLASS(w11, MetaWeedInfo, ms1, "DataDir", 0);
  w11->Resize(6, y, W-12-6, 1);
  CREATE_ADD_GLASS(w12, MetaWeedInfo, ms1, "Event", 0);
  w12->Resize(W-6, y, 6, 1);
  y++;
  CREATE_ADD_GLASS(w16, MetaWeedInfo, ms1, "KineType", 0);
  w16->Resize(8, y, W-8-12, 1);
  CREATE_ADD_GLASS(w13, MetaWeedInfo, ms1, "SetupDataSource", 0);
  w13->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w14, MetaWeedInfo, ms1, "Operation", 0);
  w14->Resize(8, y, W-8, 1);
  y++;

  Y += y; y=0;

  // ### Data importers

  // Kinematics:
  CREATE_ADD_GLASS(ms2, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms2->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w20, MetaWeedInfo, ms2, "<box>", "Kinematics:");
  w20->Resize(0, y, W, 1);
  w20->Color(0.7, 0.85, 0.7);
  w20->Align(true, -1, 0);
  w20->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w21, MetaWeedInfo, ms2, "ParticleSelection", 0);
  w21->Resize(6, y, W-6, 1);
  w21->Label("selection: ");
  // w20->Align(false, 0, -1);
  y++;
  CREATE_ADD_GLASS(w22, MetaWeedInfo, ms2, "SelectParticles", 0);
  w22->Resize(0, y, W, 1);
  w22->Label("Run selection ..");
  // w21->Align(false, 0, 1);
  y++;

  Y += y; y=0;

  // Hits:
  CREATE_ADD_GLASS(ms3, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms3->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w30, MetaWeedInfo, ms3, "<box>", "Hits:");
  w30->Resize(0, y, W, 1);
  w30->Color(0.7, 0.85, 0.7);
  w30->Align(true, -1, 0);
  w30->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w31, MetaWeedInfo, ms3, "HitSelection", 0);
  w31->Resize(6, y, W-6, 1);
  w31->Label("selection: ");
  // w20->Align(false, 0, -1);
  y++;
  CREATE_ADD_GLASS(w32, MetaWeedInfo, ms3, "SelectHits", 0);
  w32->Resize(0, y, W, 1);
  w32->Label("Run selection ..");
  // w21->Align(false, 0, 1);
  y++;

  Y += y; y=0;

  // Clusters:
  CREATE_ADD_GLASS(ms5, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms5->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w50, MetaWeedInfo, ms5, "<box>", "Clusters:");
  w50->Resize(0, y, W, 1);
  w50->Color(0.7, 0.85, 0.7);
  w50->Align(true, -1, 0);
  w50->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w51, MetaWeedInfo, ms5, "ClusterSelection", 0);
  w51->Resize(6, y, W-6, 1);
  w51->Label("selection: ");
  // w20->Align(false, 0, -1);
  y++;
  CREATE_ADD_GLASS(w52, MetaWeedInfo, ms5, "SelectClusters", 0);
  w52->Resize(0, y, W, 1);
  w52->Label("Run selection ..");
  // w21->Align(false, 0, 1);
  y++;

  Y += y; y=0;

  // RecTracks:
  CREATE_ADD_GLASS(ms6, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms6->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w60, MetaWeedInfo, ms6, "<box>", "RecTracks:");
  w60->Resize(0, y, W, 1);
  w60->Color(0.7, 0.85, 0.7);
  w60->Align(true, -1, 0);
  w60->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w61, MetaWeedInfo, ms6, "RecSelection", 0);
  w61->Resize(6, y, W-6, 1);
  w61->Label("selection: ");
  // w20->Align(false, 0, -1);
  y++;
  CREATE_ADD_GLASS(w62, MetaWeedInfo, ms6, "SelectRecTracks", 0);
  w62->Resize(0, y, W, 1);
  w62->Label("Run selection ..");
  // w21->Align(false, 0, 1);
  ++y;

  Y += y; y=0;

  // TPC digits
  CREATE_ADD_GLASS(ms4, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms4->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w40, MetaWeedInfo, ms4, "<box>", "TPC digits:");
  w40->Resize(0, y, W, 1);
  w40->Color(0.7, 0.85, 0.7);
  w40->Align(true, -1, 0);
  w40->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w41, MetaWeedInfo, ms4, "ShowTPCPlate", 0);
  w41->Resize(0, y, W, 1);
  w41->Label("Show TPC Plate ..");
  y++;

  return mv;
}
