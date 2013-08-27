// $Header$

// alice_geom: prototype of ALICE detector visualization
//
// libs: Geom1, RootGeo

#include <glass_defines.h>
class ZGeoNode;
class GeoUserData;

#include "common_foos.C"

/**************************************************************************/
// main
/**************************************************************************/

void alice_geom(Int_t import_mode=0)
{
  // import_mode: controls how top-level volumes are grouped in the list
  //    = 0: import by regexp (hand-written)
  //    = 1: import by common volume-name-prefix (automatic)
  //
  // usage: aligled <options> -- <gled-options> 'alice_geom(1)'

  Gled::AssertMacro("sun_demos.C");

  g_queen->SetAuthMode(ZQueen::AM_None);
  g_queen->SetMapNoneTo(ZMirFilter::R_Allow);

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  //--------------------------------------------------------------

  printf("Importing geometry ...\n");
  TGeoManager::Import(file_grep("alice_fullgeo.root"));
  printf("Done importing geometry.\n");

  //--------------------------------------------------------------

  create_basic_scene(); // Returned in g_scene

  //--------------------------------------------------------------

  ZGeoNode* znode = new ZGeoNode("MasterVolume");
  znode->SetTNode(gGeoManager->GetTopNode());
  znode->SetOM(-2);
  znode->SetUseOM(true);
  g_queen->CheckIn(znode);
  g_scene->Add(znode);
  znode->SetRnrSelf(false);

  import_by_regexp(znode);
  select_some_dets(znode);

  // create an empty node to test save/load from file
  CREATE_ADD_GLASS(em_node, ZGeoNode, g_scene, "A GeoNode", "");
  em_node->SetTNode(gGeoManager->GetTopNode());
  em_node->SetOM(-2.5);
  em_node->SetUseOM(true);
  em_node->SetPos(6.5, 0, 0);
  em_node->SetDefFile("EvDisp.root");

  // create an empty OvlMgr.
  CREATE_ADD_GLASS(em_ovlm, ZGeoOvlMgr, g_scene, "A GeoOvlMgr", "");
  em_ovlm->SetOM(-2.5);
  em_ovlm->SetPos(-6.5, 0, 0);
  em_ovlm->SetUseOM(true);

  // Geo repacker
  CREATE_ADD_GLASS(repack, ZGeoRepacker, g_scene, "GeoRepacker", "");
  repack->SetRoot(znode);

  //--------------------------------------------------------------

  setup_default_gui();
  spawn_default_gui();
}

/**************************************************************************/
// Functions
/**************************************************************************/

#define DECLARE_CAST(_var_, _type_, _val_) \
  _type_* _var_ = dynamic_cast<_type_*>(_val_)

ZGeoNode* rnr_self(ZGeoNode* root, const Text_t* path, Bool_t rnr)
{
  DECLARE_CAST(x, ZGeoNode, root->FindLensByPath(path));
  if(x) {
    x->SetRnrSelf(rnr);
  }
  return x;
}

ZGeoNode* rnr_elements(ZGeoNode* root, const Text_t* path, Bool_t rnr)
{
  DECLARE_CAST(x, ZGeoNode, root->FindLensByPath(path));
  if(x) {
    x->SetRnrElements(rnr);
  }
  return x;
}

ZGeoNode* import_nodes(ZGeoNode* root, const Text_t* path)
{
  DECLARE_CAST(x, ZGeoNode, root->FindLensByPath(path));
  if(x) x->ImportNodes();
  return x;
}

/**************************************************************************/

void import_by_regexp(ZGeoNode* volt)
{
  // This demonstrates use of ZGeoNode::ImportByRegexp().
  // Relies on volume-naming conventions.

  printf("Import by regexp ...\n");

  volt->ImportByRegExp("PHOS", "^PHOS");
  volt->ImportByRegExp("RICH", "^RICH");
  volt->ImportByRegExp("ITSV", "^ITS");
  volt->ImportByRegExp("TPC",  "^TPC");

  // import, but not show
  volt->ImportByRegExp("TRD TOF", "^B"); rnr_elements(volt, "TRD TOF", false);
  volt->ImportByRegExp("EPM", "^EPM");   rnr_elements(volt, "EPM", false);
  volt->ImportByRegExp("ZDC", "^ZDC");   rnr_elements(volt, "ZDC", false);
  volt->ImportByRegExp("ZEM", "^ZEM");   rnr_elements(volt, "ZEM", false);
  volt->ImportByRegExp("FMD", "^FMD");   rnr_elements(volt, "FMD", false);

  volt->ImportByRegExp("Hall", "^H");    rnr_elements(volt, "Hall", false);

  printf("Importing remaining top-levels ...\n");
  volt->ImportUnimported("Remaining top-levels");
  rnr_elements(volt, "Remaining top-levels", false);
}

/**************************************************************************/

void import_with_collect(ZGeoNode* volt)
{
  // This demonstrates use of ZGeoNode::ImportNodesWCollect().
  // Top-level volumes are grouped by common head of their name.

  printf("Import nodes with collect\n");
  volt->ImportNodesWCollect();
}

/**************************************************************************/
/**************************************************************************/

void select_some_dets(ZGeoNode* volt)
{
  import_nodes(volt, "ITSV/ITSV_1");

  ZGeoNode* tpc1 = import_nodes(volt, "TPC/TPC_1");
  if(tpc1) {
    tpc1->RnrOffForDaughters();
    DECLARE_CAST(tpcgas, ZGeoNode, tpc1->FindLensByPath("TDGN_1"));
    if(tpcgas) {
      tpcgas->SetColor(0.3, 0.7, 0.5, 0.75);
      tpcgas->SetRnrSelf(true);
    }
  }

  for(int i=1; i<=5; ++i) rnr_self(volt, GForm("PHOS/PHOS_%d", i), true);
  for(int i=1; i<=7; ++i) rnr_self(volt, GForm("RICH/RICH_%d", i),true);
}
