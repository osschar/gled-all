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

void alice_simple(const Text_t* geom_file = "alice_minigeo.root",
		  const Text_t* det_file  = "def_geoview.root")
{
  // usage: aligled <options> -- <gled-options> alice_simple.C

  Gled::AssertMacro("sun_demos.C");

  g_queen->SetAuthMode(ZQueen::AM_None);
  g_queen->SetMapNoneTo(ZMirFilter::R_Allow);

  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");

  //--------------------------------------------------------------

  alice_simple_init(geom_file, det_file);

  //--------------------------------------------------------------

  setup_default_gui();
  spawn_default_gui();
}

/**************************************************************************/

void alice_simple_init(const Text_t* geom_file = "alice_minigeo.root",
		       const Text_t* det_file  = "def_geoview.root")
{
  printf("Importing geometry ...\n");
  TGeoManager::Import(file_grep(geom_file));
  printf("Done importing geometry.\n");

  //--------------------------------------------------------------

  create_basic_scene(); // Returned in g_scene

  ZGlLightModel* lm = (ZGlLightModel*) g_scene->FindLensByPath("Var/Light Model");
  lm->SetLightModelOp(0);
  lm->SetFrontMode(GL_LINE);
  lm->SetBackMode(GL_LINE);
  lm->SetFaceCullOp(0);

  //--------------------------------------------------------------

  ZGeoNode* znode = new ZGeoNode("Geometry");
  znode->SetTNode(gGeoManager->GetTopNode());
  znode->SetOM(-2);
  znode->SetUseOM(true);
  g_queen->CheckIn(znode);
  g_scene->Add(znode);
  znode->SetRnrSelf(false);

  if (det_file != 0)
  {
    znode->SetDefFile(file_grep(det_file));
    znode->LoadFromFile();
    znode->Restore();
  }
  else
  {
    znode->ImportNodesRec(3);
  }

  g_simple_geometry = znode;
}
