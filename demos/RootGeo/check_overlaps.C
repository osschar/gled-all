// $Header$

// check_overlaps: visualization of overlaps in ALICE geometry
//
// libs: Geom1, RootGeo

#include <glass_defines.h>
class ZGeoNode;
class GeoUserData;

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
  ovl->SetResol(epsilon);
  ovl->RecalculateOvl();
  ovl->SetUseOM(1);
  ovl->SetOM(-2);
  ovl->RnrOnRec();

  // create an empty node to test save/load from file 
  ZGeoNode* em_node = new ZGeoNode("Empty Node");
  em_node->SetOM(-2.5);
  em_node->Set3Pos(6.5, 0, 0);
  em_node->SetUseOM(true);
  scenes->CheckIn(em_node);
  rscene->Add(em_node);

  //--------------------------------------------------------------

  // Spawn GUI
  {
    const Text_t* default_layout =
      "ZGeoNode(RnrSelf[4],RnrOnRec[5],RnrOffRec[5],"
      "Color[4],ImportNodes[4],NNodes[4],Mat[8]):ZGeoOvl(Overlap[4])";

    Gled::theOne->AddMTWLayout("RootGeo/ZGeoNode", default_layout);
    gROOT->LoadMacro("eye.C");
    register_GledCore_layouts();

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
    pupil->SetFOV(80);
    pupil->SetCHSize(0.03);
    pupil->SetBlend(1);
    pupil->Add(rscene);
    pupil->SetCameraBase((ZNode*)rscene->GetElementByName("Camera Base"));
    pupil->SetUpReference(rscene);
    pupil->SetUpRefAxis(2);

    Gled::theOne->SpawnEye(shell, eye_name);
  }
}
