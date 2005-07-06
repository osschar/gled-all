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

/**************************************************************************/

class MultiBoard;
class ZImage;

/**************************************************************************/

void AliLoader(const Text_t* dirname = 0,
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
  mcrst->SetTexture(image2);
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
  //
  MetaViewInfo* mvi = make_zaliload_metagui();
  // mvi->SetExpertP(true);

  spawn_default_gui();

  g_shell->SpawnMetaGui(al, mvi);

  if(dirname != 0) {
    al->SetDataDir(dirname);
    // Shoot a MIR to have functional GUI during processing: 	 
    ZMIR* setup_mir = al->S_LoadVSD(); 	 
    g_saturn->ShootMIR(setup_mir);
  }
}

/**************************************************************************/
// Stripped down ZAliLoad view.
/**************************************************************************/

MetaViewInfo* make_zaliload_metagui()
{
  int Y = 0, W = 40, H = 28;

  CREATE_ADD_GLASS(mv, MetaViewInfo, g_fire_queen, "MetaGui for ZAliLoad", 0);
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
  CREATE_ADD_GLASS(w13, MetaWeedInfo, ms1, "CreateVSD", 0);
  w13->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w17, MetaWeedInfo, ms1, "VSDFile", 0);
  w17->Resize(6, y, W-12-6, 1);
  CREATE_ADD_GLASS(w18, MetaWeedInfo, ms1, "LoadVSD", 0);
  w18->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w18, MetaWeedInfo, ms1, "ClearData", 0);
  w18->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w15, MetaWeedInfo, ms1, "<box>", "Current operation:");
  w15->Resize(0, y, W, 1);
  w15->Color(0.7, 0.7, 0.85);
  w15->Align(true, -1, 0);
  w15->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w14, MetaWeedInfo, ms1, "Operation", 0);
  w14->Resize(0, y, W, 1);
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
  w22->Resize(W-12, y, 12, 1);
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
  y++;
  CREATE_ADD_GLASS(w32, MetaWeedInfo, ms3, "SelectHits", 0);
  w32->Resize(W-12, y, 12, 1);
  w32->Label("Run selection ..");
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
  y++;
  CREATE_ADD_GLASS(w52, MetaWeedInfo, ms5, "SelectClusters", 0);
  w52->Resize(W-12, y, 12, 1);
  w52->Label("Run selection ..");
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
  y++;
  CREATE_ADD_GLASS(w62, MetaWeedInfo, ms6, "SelectRecTracks", 0);
  w62->Resize(W-12, y, 12, 1);
  w62->Label("Run selection ..");
  ++y;

  Y += y; y=0;

  // V0:
  CREATE_ADD_GLASS(ms8, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms8->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w80, MetaWeedInfo, ms8, "<box>", "V0:");
  w80->Resize(0, y, W, 1);
  w80->Color(0.7, 0.85, 0.7);
  w80->Align(true, -1, 0);
  w80->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w81, MetaWeedInfo, ms8, "V0Selection", 0);
  w81->Resize(6, y, W-6, 1);
  w81->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w82, MetaWeedInfo, ms8, "SelectV0", 0);
  w82->Resize(W-12, y, 12, 1);
  w82->Label("Run selection ..");
  ++y;

  Y += y; y=0;

  // GenInfo:
  CREATE_ADD_GLASS(ms7, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms7->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w70, MetaWeedInfo, ms7, "<box>", "GenInfo:");
  w70->Resize(0, y, W, 1);
  w70->Color(0.7, 0.85, 0.7);
  w70->Align(true, -1, 0);
  w70->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w71, MetaWeedInfo, ms7, "GISelection", 0);
  w71->Resize(6, y, W-6, 1);
  w71->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w72, MetaWeedInfo, ms7, "SelectGenInfo", 0);
  w72->Resize(W-12, y, 12, 1);
  w72->Label("Run selection ..");
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
  w41->Label("Show TPC Digits ..");
  y++;
 CREATE_ADD_GLASS(w41, MetaWeedInfo, ms4, "ShowITSDet", 0);
  w41->Resize(0, y, W, 1);
  w41->Label("Show ITS Digits ..");
  y++;
  return mv;
}
