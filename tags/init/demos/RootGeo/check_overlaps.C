// $Header$

// Spheres: a bunch of spheres with a rotator
//
// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
class ZGeoNode;
class GeoUserData;

ZGeoNode* volt = 0;

void check_overlaps(Float_t epsilon=1)
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
     "ZGeoNode(RnrSelf[3],RnrOnRec[5],RnrOffRe5[4],"
     "Color[4],ImportNodes[4],NNodes[4],Mat[8]):ZGeoOvl(Overlap[4])");

  /**************************************************************************/

  printf("Importing geometry.\n");
  TGeoManager::Import("/afs/f9.ijs.si/home/alja/gled-dev/demos/RootGeo/alice_all.root");
  /**************************************************************************/

  Scene* rscene = new Scene("Alice Detector");
  scenes->CheckIn(rscene);
  scenes->Add(rscene);
  rscene->SetUseOM(true);

  ZNode* origin = new ZNode("Origin");
  Lamp* lamp = new Lamp("Lamp first");
  lamp->SetDiffuse(1, 1, 1);
  lamp->SetAmbient(0.3, 0.3, 0.3);
  lamp->SetScale(1);
  lamp->MoveLF(2, 100); lamp->RotateLF(3,1, TMath::Pi()/2);
  scenes->CheckIn(origin); rscene->Add(origin);
  scenes->CheckIn(lamp);   origin->Add(lamp);

  
  Lamp* lamp_op = new Lamp("Lamp optional");
  lamp_op->MoveLF(1, 150); lamp->RotateLF(1,1, TMath::Pi()/2);
  scenes->CheckIn(lamp_op);   origin->Add(lamp_op);
  
  rscene->GetGlobLamps()->Add(lamp);
  rscene->GetGlobLamps()->Add(lamp_op);

  CREATE_ADD_GLASS(pointmod, ZGlBlending, rscene, "Point modificator", 0);
  pointmod->SetAntiAliasOp(1);
  pointmod->SetPointSize(10);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, rscene, "Light model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(1);

  ZGeoOvlMgr* ovl = new ZGeoOvlMgr("Overlap Mng.");
  scenes->CheckIn(ovl);
  rscene->Add(ovl);  
  ovl->SetGeoManager(gGeoManager);
  ovl->RecalculateOvl();
  ovl->SetUseOM(1);
  ovl->SetOM(-3);

  {
    Eventor* el = new Eventor("Dyn.amo2");
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
    shell->Add(rscene);
    //shell->ImportKings();	  // Get all Kings as top level objects

    shell->SetLayout(
		     "ZGeoNode(RnrSelf[3],RnrOnRec[5],RnrOffRec[5],"
		     "Color[4],ImportNodes[5],NNodes[4],Mat[8]):ZGeoOvl(Overlap[4])");

    shell->SetLeafLayout(NestInfo::LL_Custom);

    if(pupil_name) {
      PupilInfo* pupil = new PupilInfo(pupil_name);
      pupil->SetFOV(60);
      pupil->SetBlend(1);
      fire_queen->CheckIn(pupil);
      shell->GetPupils()->Add(pupil);
      if(scenes->First())
	pupil->Add(scenes->First());
    }

    Gled::theOne->SpawnEye(shell, eye_name);
  }
}
