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

  // create an empty node to test save/load from file 
  ZGeoNode* em_node = new ZGeoNode("Empty Node");
  em_node->SetOM(-2.5);
  em_node->Set3Pos(6.5, 0, 0);
  em_node->SetUseOM(true);
  scenes->CheckIn(em_node);
  rscene->Add(em_node);

  //--------------------------------------------------------------

  spawn_default_gui(rscene);
}
