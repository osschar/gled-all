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
  em_node->SetUseOM(true);
  em_node->SetRnrSelf(false);

  // create an empty OvlMgr.
  CREATE_ADD_GLASS(em_ovlm, ZGeoOvlMgr, rscene, "A GeoOvlMgr", "");
  em_ovlm->SetOM(-2.5);
  em_ovlm->Set3Pos(-6.5, 0, 0);
  em_ovlm->SetUseOM(true);

  switch(import_mode) {
  case  0: { 
    import_by_regexp(znode); 
    select_some_dets(znode);
    break; 
  }
  case  1:
  default: { import_with_collect(znode); break; }
  }

  //--------------------------------------------------------------

  // Spawn GUI
  {
    const Text_t* default_layout =
      "ZNode(RnrSelf[5],RnrElements[5]):"
      "ZGeoNode(Color[4],ImportNodes[4],NNodes[4],Mat[8])";

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

ZGeoNode* rnr_self_on(ZGeoNode* root, const Text_t* path)
{
  DECLARE_CAST(x, ZGeoNode, root->FindLensByPath(path));
  if(x) x->SetRnrSelf(true);
  return x;
}

ZGeoNode* import_nodes(ZGeoNode* root, const Text_t* path)
{
  DECLARE_CAST(x, ZGeoNode, root->FindLensByPath(path));
  if(x) x->ImportNodes();
  return x;
}

/**************************************************************************/
/**************************************************************************/

void import_by_regexp(ZGeoNode* volt)
{
  // This demonstrates use of ZGeoNode::ImportByRegexp().
  // Relies on volume-naming conventions.

  printf("Import by regexp\n");

  volt->ImportByRegExp("Hall", TRegexp("^H"));
  volt->ImportByRegExp("L3", TRegexp("^L3"));

  volt->ImportByRegExp("PHOS", TRegexp("^PHOS"));
  volt->ImportByRegExp("RICH", TRegexp("^RICH"));

  volt->ImportByRegExp("ITSV", TRegexp("^ITS"));
  volt->ImportByRegExp("TPC", TRegexp("^TPC"));

  volt->ImportByRegExp("EPM", TRegexp("^EPM"));

  volt->ImportByRegExp("ZDC", TRegexp("^ZDC"));
  volt->ImportByRegExp("ZEM", TRegexp("^ZEM"));
  volt->ImportByRegExp("FMD", TRegexp("^FMD"));
  volt->ImportByRegExp("DDIP", TRegexp("^DDIP"));

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

  volt->ImportByRegExp("TRD&TOF", TRegexp("^B"));

  printf("call Import unimported \n");
  volt->ImportUnimported("Remaining top-levels");
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
    tpc1->SetRnrElements(false);
    DECLARE_CAST(tpcgas, ZGeoNode, tpc1->FindLensByPath("TDGN_1"));
    if(tpcgas) {
      tpcgas->SetColor(0.3, 0.7, 0.5, 0.75);
      tpcgas->SetRnrSelf(true);
    }
  }

  for(int i=1; i<=5; ++i) rnr_self_on(volt, GForm("PHOS/PHOS_%d", i));

  for(int i=1; i<=7; ++i) rnr_self_on(volt, GForm("RICH/RICH_%d", i));
}
