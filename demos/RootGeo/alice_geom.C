// $Header$

// Spheres: a bunch of spheres with a rotator
//
// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
class ZGeoNode;
class GeoUserData;

ZGeoNode* volt = 0;

void alice_geom(Bool_t impwcol=false)
{
  gSystem->IgnoreSignal(kSigSegmentationViolation, true);

  {
    if(Gled::theOne->GetSaturn() == 0) {
      Gled::theOne->SpawnSun();
    }

    Saturn* sun = Gled::theOne->GetSaturn();
    if(sun == 0) {
      printf("Sun is not spawned ...\n");
      return;
    }

    ZSunQueen* sun_queen  = sun->GetSunQueen();
    ZQueen*    fire_queen = sun->GetFireQueen();

    if(sun->GetSaturnInfo()->GetUseAuth())
      gROOT->Macro("std_auth.C");

    ZQueen* scenes = new ZQueen(16*10000, "Scenes", "Goddess of Ver");
    Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(scenes);
    scenes->SetMandatory(true);
  }

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  Gled::theOne->AddMTWLayout
    ("RootGeo/ZGeoNode",
     "ZGlass(Name,Title):"
     "ZNode(RnrSelf[4],RnrOnForDaughters[4],RnrOffForDaughters[4]):"
     "ZGeoNode(Color[4],ImportNodes[5],NNodes[4])"
    );

  Scene* rscene = new Scene("Alice Detector");
  scenes->CheckIn(rscene);
  scenes->Add(rscene);
  rscene->SetUseOM(true);

  ZNode* origin = new ZNode("Origin");
  Lamp* lamp = new Lamp("Lamp");
  lamp->SetDiffuse(1, 1, 1);
  lamp->SetAmbient(0.3, 0.3, 0.3);
  lamp->SetScale(1);
  lamp->MoveLF(3, 100); lamp->RotateLF(3,1, TMath::Pi()/2);
  
  scenes->CheckIn(origin); rscene->Add(origin);
  scenes->CheckIn(lamp);   origin->Add(lamp);
  
  rscene->GetGlobLamps()->Add(lamp);
  
  printf("Importing geometry ...\n");
  TGeoManager::Import("/afs/f9.ijs.si/home/alja/gled-dev/demos/RootGeo/alice_all.root");
  printf("Done importing geometry.\n");


  ZGeoNode* znode =  new ZGeoNode("MasterVolume");
  volt = znode;
  znode->SetVol(gGeoManager->GetMasterVolume());
  //znode->SetVol(gGeoManager->GetNode(188)->GetVolume());
  znode->SetOM(-2);
  znode->SetUseOM(true);
  scenes->CheckIn(znode);
  rscene->Add(znode);
  GeoUserData* ud == new  GeoUserData();
  znode->CreateFaceset(ud);
  znode->SetRnrSelf(false);
  import_dparts();
 {
    Eventor* el = new Eventor("Dynamo2");
    el->SetBeatsToDo(-1); el->SetInterBeatMS(100); el->SetStampInterval(10);
    scenes->CheckIn(el);  rscene->Add(el);
    Mover* mv = new Mover("S1 Rotator");
    mv->SetNode(origin); mv->SetRi(1); mv->SetRj(3); mv->SetRa(0.005);
    scenes->CheckIn(mv); el->Add(mv);
    // el->Start();
  }

  // gROOT->Macro("eye.C");
  {
  Text_t* eye_name="Eye";
  Text_t* shell_name="Shell";
  Text_t* pupil_name="Pupil";

  ShellInfo* shell = new ShellInfo(shell_name);
  fire_queen->CheckIn(shell); fire_queen->Add(shell);
  shell->ImportKings();	  // Get all Kings as top level objects

  shell->SetLayout(
     "ZGlass(Name[7]):"
     "ZGeoNode(RnrSelf[4],RnrOnForDaughters[4],RnrOffForDaughters[4],"
     "Color[4],ImportNodes[5],NNodes[4],Mat[8])");
  shell->SetLeafLayout(NestInfo::LL_Custom);

  if(pupil_name) {
    PupilInfo* pupil = new PupilInfo(pupil_name);
    pupil->SetFOV(80);
    pupil->SetCHSize(0.0);
    pupil->SetBlend(1);
    fire_queen->CheckIn(pupil);
    shell->GetPupils()->Add(pupil);
    if(scenes->First())
    pupil->Add(scenes->First());
  }

  Gled::theOne->SpawnEye(shell, eye_name);
 }
}

void import_dparts() {
  printf("Import by geometry \n");
  volt->ImportByRegExp("ITS", TRegexp("^ITS"));
  volt->ImportByRegExp("TPC", TRegexp("^TPC"));

  volt->ImportByRegExp("PHOS", TRegexp("^PHOS"));
  volt->ImportByRegExp("RICH", TRegexp("^RICH"));
  // volt->ImportByRegExp("TOF", TRegexp("^TOF"));

  volt->ImportByRegExp("ZDC", TRegexp("^ZDC"));
  volt->ImportByRegExp("ZEM", TRegexp("^ZEB"));
  volt->ImportByRegExp("FMD", TRegexp("^FMD"));

  volt->ImportByRegExp("S/S01", TRegexp("^S01"));
  volt->ImportByRegExp("S/S03", TRegexp("^S02"));
  volt->ImportByRegExp("S/S03", TRegexp("^S03"));
  volt->ImportByRegExp("S/S04", TRegexp("^S04"));
  volt->ImportByRegExp("S/S07", TRegexp("^S07"));
  volt->ImportByRegExp("S/S08", TRegexp("^S08"));
  volt->ImportByRegExp("S/S09", TRegexp("^S09"));
  volt->ImportByRegExp("S/S10", TRegexp("^S10"));
  volt->ImportByRegExp("S/SCF1", TRegexp("^S[CF]1"));
  volt->ImportByRegExp("S/SCF2", TRegexp("^S[CF]2"));
  volt->ImportByRegExp("S/SCF3", TRegexp("^S[CF]3"));
  volt->ImportByRegExp("S/SCF4", TRegexp("^S[CF]4"));

  volt->ImportByRegExp("EPM", TRegexp("^EPM"));

  volt->ImportByRegExp("Hall", TRegexp("^H"));

  volt->ImportByRegExp("L3", TRegexp("^L3"));

  volt->ImportByRegExp("Hall", TRegexp("^H"));


  volt->ImportByRegExp("B-Stuffe", TRegexp("^B"));

  // volt->ImportByRegExp("", TRegexp("^"));
  volt->ImportUnimported("REST");
}
