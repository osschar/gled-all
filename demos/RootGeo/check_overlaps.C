// $Header$

// check_overlaps: visualization of overlaps in ALICE geometry
//
// libs: Geom1, RootGeo

#include <glass_defines.h>
class ZGeoNode;
class GeoUserData;

ZGeoNode* volt = 0;

const Text_t* default_layout =
  "ZGeoNode(RnrSelf[3],RnrOnRec[5],RnrOffRec[4],"
  "Color[4],ImportNodes[4],NNodes[4],Mat[8]):ZGeoOvl(Overlap[4])";

#include "common_foos.C"

/**************************************************************************/
// main
/**************************************************************************/

void check_overlaps(Float_t epsilon=1)
{
  // epsilon: minimal overlap that is imported for inspection
  //
  // usage: ./aligled <options> -- <gled-options> 'check_overlaps(0.1)'

  if(Gled::theOne->GetSaturn() == 0) gROOT->Macro("sun.C");

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  Gled::theOne->AddMTWLayout("RootGeo/ZGeoNode", default_layout);

  //--------------------------------------------------------------

  printf("Importing geometry ...\n");
  TGeoManager::Import("alice_all.root");
  printf("Done importing geometry.\n");

  //--------------------------------------------------------------

  Scene* rscene = create_basic_scene();

  //--------------------------------------------------------------

  ZGeoOvlMgr* ovl = new ZGeoOvlMgr("Overlap Mgr");
  scenes->CheckIn(ovl);
  rscene->Add(ovl);  
  ovl->SetGeoManager(gGeoManager);
  ovl->RecalculateOvl();
  ovl->SetUseOM(1);
  ovl->SetOM(-2);

  ovl->RnrOnRec();

  //--------------------------------------------------------------

  // Spawn GUI
  {
    Text_t* eye_name   = "Eye";
    Text_t* shell_name = "Shell";
    Text_t* pupil_name = "Pupil";

    ShellInfo* shell = new ShellInfo(shell_name);
    fire_queen->CheckIn(shell); fire_queen->Add(shell);
    shell->Add(rscene);
    //shell->ImportKings();	  // Get all Kings as top level objects

    shell->SetLayout(default_layout);
    shell->SetLeafLayout(NestInfo::LL_Custom);

    CREATE_ADD_GLASS(pupil, PupilInfo, shell->GetPupils(), pupil_name, "");
    pupil->SetFOV(60);
    pupil->SetBlend(1);
    pupil->Add(rscene);
    pupil->SetCameraBase((ZNode*)rscene->GetElementByName("Camera Base"));

    Gled::theOne->SpawnEye(shell, eye_name);
  }
}
