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

  Gled::AssertMacro("sun_demos.C");

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  //--------------------------------------------------------------

  printf("Importing geometry ...\n");
  TGeoManager::Import(file_grep("alice_fullgeo.root"));
  printf("Done importing geometry.\n");

  //--------------------------------------------------------------

  create_basic_scene(); // Returned in g_scene

  //--------------------------------------------------------------

  ZGeoOvlMgr* ovl = new ZGeoOvlMgr("Overlap Mgr");
  g_queen->CheckIn(ovl);
  g_scene->Add(ovl);
  ovl->SetResol(epsilon);
  ovl->RecalculateOvl();
  ovl->SetUseOM(1);
  ovl->SetOM(-2);

  // create an empty node to test save/load from file
  ZGeoNode* em_node = new ZGeoNode("Empty Node");
  em_node->SetOM(-2.5);
  em_node->SetPos(6.5, 0, 0);
  em_node->SetUseOM(true);
  g_queen->CheckIn(em_node);
  g_scene->Add(em_node);

  //--------------------------------------------------------------

  setup_default_gui();
  spawn_default_gui();
}
