// $Header$

// libs: Geom1 AliEnViz

#include <glass_defines.h>
#include <gl_defines.h>

//==============================================================================

class AEVMlSucker;

//------------------------------------------------------------------------------

AEVMlSucker   *ml_sucker = 0;
ZNameMap      *sites     = 0;

//==============================================================================

void suck()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("AliEnViz");

  Gled::LoadMacro("alienviz_common_foos.C");

  alienviz_setup_lib_objects(128*1024);

  ASSIGN_ADD_GLASS(ml_sucker, AEVMlSucker, aev_queen, "MonaLisa Sucker", 0);

  ASSIGN_ADD_GLASS(sites, AEVSiteList, aev_queen, "Sites", 0);

  //============================================================================

  alienviz_setup_scene();

  CREATE_ADD_GLASS(c_mapviz, AEVMapViz, g_scene, "MapViz", 0);
  c_mapviz->SetPos(0, -3, 0);
  c_mapviz->SetScale(6); c_mapviz->SetUseScale(true);
  c_mapviz->CutEarth_PDC04(find_texture("Earth"));
  c_mapviz->SetSites(sites);

  AEVMapViz* g_mapviz = globe_map();
  g_mapviz->SetSites(sites);

  AEVMapViz* m_mapviz = mercator_map();
  m_mapviz->SetSites(sites);

  //============================================================================

  alienviz_spawn_eye();

  //============================================================================

  ml_sucker->StartSucker();
}
