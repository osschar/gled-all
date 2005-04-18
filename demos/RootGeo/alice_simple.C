// $Header$

// alice_simple: simplified ALICE geometry for event visualization
//
// libs: Geom1, RootGeo

#include <glass_defines.h>
#include <gl_defines.h>

class ZGeoNode;

ZGeoNode* g_simple_geometry = 0;

#include "common_foos.C"

/**************************************************************************/
// main
/**************************************************************************/

Scene* alice_simple(const Text_t* geom_file = "simple_geo.root",
		    const Text_t* det_file  = "EvDisp.root")
{
  // usage: aligled <options> -- <gled-options> alice_simple.C

  if(Gled::theOne->GetSaturn() == 0) gROOT->Macro("sun.C");

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  //--------------------------------------------------------------

  Scene* rscene = alice_simple_init(geom_file, det_file);

  //--------------------------------------------------------------

  setup_default_gui(rscene);
  spawn_default_gui();

  return rscene;
}

/**************************************************************************/

Scene* alice_simple_init(const Text_t* geom_file = "simple_geo.root",
			 const Text_t* det_file  = "EvDisp.root")
{
  printf("Importing geometry ...\n");
  TGeoManager::Import(geom_file);
  printf("Done importing geometry.\n");

  //--------------------------------------------------------------

  Scene* rscene = create_basic_scene();

  ZGlLightModel* lm = (ZGlLightModel*) rscene->FindLensByPath("Var/Light Model");
  lm->SetLightModelOp(0);
  lm->SetFrontMode(GL_LINE);
  lm->SetBackMode(GL_LINE);
  lm->SetFaceCullOp(0);

  //--------------------------------------------------------------

  ZGeoNode* znode = new ZGeoNode("Geometry");
  znode->SetTNode(gGeoManager->GetTopNode());
  znode->SetOM(-2);
  znode->SetUseOM(true);
  scenes->CheckIn(znode);
  rscene->Add(znode);
  znode->SetRnrSelf(false);

  znode->SetDefFile(det_file);
  znode->LoadFromFile();
  znode->Restore();

  g_simple_geometry = znode;

  return rscene;
}
