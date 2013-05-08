// $Id$

// libs: Geom1 AliEnViz

#include <glass_defines.h>
#include <gl_defines.h>

//==============================================================================

class TimeMaker;

class AEVManager;
class AEVMlSucker;
class AEVSiteList;

//------------------------------------------------------------------------------

AEVManager    *manager   = 0;
AEVMlSucker   *ml_sucker = 0;
AEVSiteList   *sites     = 0;
Eventor       *animator  = 0;
TimeMaker     *timemaker = 0;

const Int_t g_rps = 60;

//==============================================================================

// This deosn't work ... AEVManager only works with solar system.
// Would make sense to upgrade it to also support map-vizes.

void suck()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("AliEnViz");

  Gled::LoadMacro("alienviz_common_foos.C");

  alienviz_setup_lib_objects(128*1024);

  ASSIGN_ADD_GLASS(manager, AEVManager, aev_queen, "AEVManager", 0);
  aev_queen->SetKeepSorted(true);

  ASSIGN_ADD_GLASS(ml_sucker, AEVMlSucker, aev_queen, "MonaLisa Sucker", 0);
  ml_sucker->SetManager(manager);
  manager->SetSucker(ml_sucker);

  ASSIGN_ADD_GLASS(sites, AEVSiteList, aev_queen, "Sites", 0);
  sites->SetKeepSorted(true);
  manager->SetSites(sites);

  ASSIGN_ADD_GLASS(animator, Eventor, aev_queen, "Animator", 0);

  ASSIGN_ADD_GLASS(timemaker, TimeMaker, animator, "Animation TimeMaker", 0);

  //============================================================================

  alienviz_setup_scene();

  //----------------------------------------------------------------------------

  CREATE_ADD_GLASS(c_mapviz, AEVMapViz, g_scene, "MapViz", 0);
  c_mapviz->SetPos(0, -3, 0);
  c_mapviz->SetScale(6); c_mapviz->SetUseScale(true);
  c_mapviz->CutEarth_PDC04(find_texture("Earth"));
  c_mapviz->SetSites(sites);

  AEVMapViz* g_mapviz = globe_map();
  g_mapviz->SetSites(sites);

  AEVMapViz* m_mapviz = mercator_map();
  m_mapviz->SetSites(sites);

  //----------------------------------------------------------------------------

  CREATE_ADD_GLASS(gled_viz_node, ZNode, aev_scene, "GledNode", 0);
  gled_viz_node->SetPos(0, -3, 5);
  gled_viz_node->SetRnrMod(find_rnrmod("NamesOn"));
  CREATE_ADD_GLASS(gled_viz, WSSeed, gled_viz_node, "Central Services", 0);
  // Could move it somewhat and rotate it in parent frame.
  gled_viz->SetPos(0, 0, 0);
  gled_viz->RotateLF(3, 1, 0.35*TMath::Pi());
  gled_viz->SetRnrMod(find_rnrmod("NamesOff"));
  gled_viz->SetTexture(find_texture("Gled"));
  gled_viz->MakeLissajou(0, TMath::TwoPi(), 16,
			 0.5, 2, 0,
			 0.5, 3, 0,
			 0.5, 2, TMath::Pi()/2,
			 0.1);
  gled_viz->SetDtexU(0.1);
  gled_viz->SetRenormLen(true);

  timemaker->AddClient(gled_viz);

  CREATE_ADD_GLASS(gled_rot, Mover, animator, "Gled Rotator", 0);
  gled_rot->SetRi(1);
  gled_rot->SetRj(2);
  gled_rot->SetRa(0.03);
  gled_rot->SetNode(gled_viz);

  //============================================================================

  alienviz_spawn_eye();

  //============================================================================

  animator->SetInterBeatMS(TMath::Nint(1000.0/g_rps));
  animator->Start();

  // Direct connection:
  // ml_sucker->SetSuckHost("pcalimonitor.cern.ch");
  // ml_sucker->SetSuckPort(7015);

  // ssh tunnel: ssh -L 7777:pcalimonitor.cern.ch:7015 lxplus.cern.ch cat -
  ml_sucker->SetSuckHost("localhost");
  ml_sucker->SetSuckPort(7777);
  // Can also do capture and play it back later:
  // 1. nc localhost 7777 > nc-dump
  // 2. nc -l localhost 7777 < nc-dump

  // ml_sucker->SetFooSleep(10);

  ml_sucker->StartSucker();
}
