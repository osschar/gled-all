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

  ZGeoNode* znode = new ZGeoNode("MasterVolume");
  znode->SetTNode(gGeoManager->GetTopNode());
  znode->SetOM(-2);
  znode->SetUseOM(true);
  scenes->CheckIn(znode);
  rscene->Add(znode);
  znode->SetRnrSelf(false);

  // create an empty node to test save/load from file 
  CREATE_ADD_GLASS(em_node, ZGeoNode, rscene, "A GeoNode", "");
  em_node->SetTNode(gGeoManager->GetTopNode());
  em_node->SetOM(-2.5);
  em_node->Set3Pos(6.5, 0, 0);

  // create an empty OvlMgr.
  CREATE_ADD_GLASS(em_ovlm, ZGeoOvlMgr, rscene, "A GeoOvlMgr", "");
  em_ovlm->SetOM(-2.5);
  em_ovlm->Set3Pos(-6.5, 0, 0);
  em_ovlm->SetUseOM(true);

  import_by_regexp(znode); 
  select_some_dets(znode);

  //--------------------------------------------------------------

  // Spawn GUI
  {
    const Text_t* default_layout =
      "ZNode(RnrSelf[5],RnrElements[5]):"
      "ZGeoNode(RnrOnForDaughters[5],RnrOffForDaughters[5],Color[4],ImportNodes[4],NNodes[4],Mat[8])";

    Gled::theOne->AddMTWLayout("RootGeo/ZGeoNode", default_layout);
    gROOT->LoadMacro("eye.C");
    register_GledCore_layouts();

    Text_t* eye_name   = "Eye";
    Text_t* shell_name = "Shell";
    Text_t* pupil_name = "Pupil";

    CREATE_ADD_GLASS(shell, ShellInfo, fire_queen, shell_name, "");
    shell->Add(rscene);

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

  printf("Import by regexp\n");

  volt->ImportByRegExp("PHOS", TRegexp("^PHOS"));
  volt->ImportByRegExp("RICH", TRegexp("^RICH"));
  volt->ImportByRegExp("ITSV", TRegexp("^ITS"));
  volt->ImportByRegExp("TPC", TRegexp("^TPC")); 

  // import, but not show
  volt->ImportByRegExp("TRD&TOF", TRegexp("^B")); rnr_elements(volt, "TRD&TOF", false);
  volt->ImportByRegExp("EPM", TRegexp("^EPM")); rnr_elements(volt, "EPM", false);
  volt->ImportByRegExp("ZDC", TRegexp("^ZDC")); rnr_elements(volt, "ZDC", false);
  volt->ImportByRegExp("ZEM", TRegexp("^ZEM")); rnr_elements(volt, "ZEM", false);
  volt->ImportByRegExp("FMD", TRegexp("^FMD")); rnr_elements(volt, "FMD", false);
  volt->ImportByRegExp("Hall", TRegexp("^H"));  rnr_elements(volt, "Hall", false);

  printf("call Import unimported \n");
  volt->ImportUnimported("Remaining top-levels"); rnr_elements(volt, "Remaining top-levels", false);
  printf("END call Import unimported \n");
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
