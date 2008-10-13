// $Header$

// libs: Geom1 AliEnViz

#include <glass_defines.h>
#include <gl_defines.h>

//==============================================================================

class SolarSystem;
class CosmicBall;
class ODECrawler;
class TimeMaker;

class AEVManager;
class AEVMlSucker;
class AEVSiteList;

//------------------------------------------------------------------------------

Scene        *overlay  = 0;

// SolarSystem vars

SolarSystem  *ss       = 0;
CosmicBall   *stella   = 0;
ODECrawler   *ode      = 0;

// MonaSucker vars

AEVManager    *manager   = 0;
AEVMlSucker   *ml_sucker = 0;
AEVSiteList   *sites     = 0;
Eventor       *animator  = 0;
TimeMaker     *timemaker = 0;

const Int_t g_rps = 60;

//==============================================================================

void solar_suck()
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

  /*
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
  */

  //----------------------------------------------------------------------------

  ASSIGN_ADD_GLASS(aev_scene, Scene, aev_queen, "AEVScene", "");

  g_scene = aev_scene;

  CREATE_ADD_GLASS(lamp, Lamp, g_scene, "Lamp", "");
  lamp->SetPos(0, 0, 0);
  lamp->SetDiffuse(1, 1, 0.6);
  lamp->SetDrawLamp(false);
  lamp->SetLampScale(1);
  // This crashes my laptop together with glDrawArrays of tringtvor ... badly.
  // Must be a weird ATI bug. 2007
  g_scene->GetGlobLamps()->Add(lamp);

  CREATE_ADD_GLASS(bases, ZNode, aev_queen, "CameraBases", 0);

  aev_scene->Add(bases);

  CREATE_ADD_GLASS(cam_base, Sphere, bases, "Home", 0);
  cam_base->SetPos(0, -350, 350);
  cam_base->SetRotByDegrees(90, -45, 0);
  cam_base->SetRadius(1);
  cam_base->SetRnrSelf(false);

  //----------------------------------------------------------------------------

  ASSIGN_ADD_GLASS(ss, SolarSystem, g_scene, "SolarSystem", "");
  ss->SetColor(0.8, 0.8, 0.8, 0.8);
  ss->SetMaxTheta(10);
  ss->SetMaxEcc(0.00);
  ss->SetTimeFac(25);

  ode = ss->GetODECrawler();

  manager->SetSolarSystem(ss);
  timemaker->AddClient(ss);

  ss->MakeStar();
  stella = dynamic_cast<CosmicBall*>(ss->GetBalls()->GetElementById(0));

  //----------------------------------------------------------------------------

  //CREATE_ADD_GLASS(gled_viz_node, ZNode, aev_scene, "GledNode", 0);
  //gled_viz_node->SetPos(0, -3, 5);
  //gled_viz_node->SetRnrMod(find_rnrmod("NamesOn"));

  ss->SetRnrMod(find_rnrmod("NamesOn"));

  stella->SetName("AliEn");
  stella->SetRnrSelf(false);

  CREATE_ATT_GLASS(emission, ZGlMaterial, stella, SetRnrMod, "Emission", 0);
  emission->SetEmission(1, 1, 0.4);

  CREATE_ADD_GLASS(gled_viz, WSSeed, stella, "", 0);
  // Could move it somewhat and rotate it in parent frame.
  gled_viz->SetPos(0, 0, 0);
  gled_viz->RotateLF(3, 1, 0.35*TMath::Pi());
  gled_viz->SetRnrMod(find_rnrmod("NamesOff"));
  gled_viz->SetTexture(find_texture("Gled"));
  gled_viz->MakeLissajou(0, TMath::TwoPi(), 16,
			 50, 2, 0,
			 50, 3, 0,
			 50, 2, TMath::Pi()/2,
			 10);
  gled_viz->SetDtexU(0.1);
  gled_viz->SetRenormLen(true);

  timemaker->AddClient(gled_viz);

  CREATE_ADD_GLASS(gled_rot, Mover, animator, "Gled Rotator", 0);
  gled_rot->SetRi(1);
  gled_rot->SetRj(2);
  gled_rot->SetRa(0.03);
  gled_rot->SetNode(gled_viz);

  CREATE_ADD_GLASS(dumper, ScreenDumper, animator, "ScreenDumper", 0);
  dumper->SetWaitSignal(true);

  //============================================================================

  alienviz_spawn_eye();

  g_pupil->SetNearClip(1);
  g_pupil->SetFarClip(5000);
  g_pupil->SetMoveOM(-1);
  g_pupil->SetBlend(true);
  g_pupil->SetClearColor(0,0,0);

  g_pupil->SetLookAt(g_scene);

  dumper->SetPupil(g_pupil);

  //============================================================================

  make_overlay();
  g_pupil->SetOverlay(overlay);

  manager->MakeInfoText();

  //============================================================================

  ode->SetH1(1); ode->SetHmin(1e-8);
  ode->SetAcc(1e-3);

  ss->SetDesiredRHack(true);
  ss->SetDesiredRHackT0(5);
  ss->StartStepIntegratorThread();

  ss->SetTimeFac(50);
  ss->SetBallHistorySize(200);

  animator->SetInterBeatMS(TMath::Nint(1000.0/g_rps));
  animator->Start();

  // To be used without network, in conjuction with:
  //   nc -l localhost 9999 < nc-dump
  // ml_sucker->SetSuckHost("localhost");
  // ml_sucker->SetSuckPort(9999);
  // ml_sucker->SetFooSleep(10);

  ml_sucker->StartSucker();
}

/******************************************************************************/
// Overlay GUI
/******************************************************************************/

void make_overlay()
{
  Float_t weed_dx = 130, step_dx = 140, step_dy = 26;

  ASSIGN_ADD_GLASS(overlay, Scene, g_queen, "AEVOverlay", 0);

  CREATE_ADD_GLASS(ovl_lm, ZGlLightModel, overlay, "LightOff", 0);
  ovl_lm->SetLightModelOp(0);

  //----------------------------------------------------------------------------

  CREATE_ADD_GLASS(ibpp, ZGlPerspective, overlay, "InfoPerspective", 0);
  ibpp->StandardFixed();

  CREATE_ADD_GLASS(info_bar, Text, overlay, "InfoText", 0);
  info_bar->SetPos(5, 9.4, 0);
  info_bar->SetScales(0.5, 0.8, 1);
  info_bar->SetFont(find_font("LargeFont"));
  info_bar->SetBackPoly(false);
  info_bar->SetFramePoly(false);
  info_bar->SetFGCol(0.6, 1, 0.6);

  CREATE_ADD_GLASS(blurp_bar, Text, overlay, "SubInfoText", 0);
  blurp_bar->SetPos(5, 8.9, 0);
  blurp_bar->SetScales(0.25, 0.4, 1);
  // blurp_bar->SetFont(find_font("LargeFont"));
  blurp_bar->SetBackPoly(false);
  blurp_bar->SetFramePoly(false);
  blurp_bar->SetFGCol(0.6, 0.6, 1);

  //----------------------------------------------------------------------------

  CREATE_ADD_GLASS(pp, ZGlPerspective, overlay, "Perspective", 0);
  pp->StandardPixel();

  CREATE_ADD_GLASS(bfs, WGlFrameStyle, overlay, "Butt Frame Style", 0);
  bfs->StandardPixel();
  bfs->SetDefDx(weed_dx);
  bfs->SetTileColor(0.5, 0.15, 0.25, 0.7);
  bfs->SetBelowMColor(0.3, 0.08, 0.14, 0.9);

  SGridStepper gs(0);
  gs.SetDs(step_dx, step_dy, 1);
  gs.SetNs(6, 4, 1);
  gs.SetOs(4, 4, 0);

  WGlButton* b;
  {
    CREATE_ADD_GLASS(main_menu, WGlWidget, overlay, "MainMenu", 0);

    ASSIGN_ADD_GLASS(b, WGlButton, main_menu, "Manager", 0);
    gs.SetNodeAdvance(b);
    b->SetCbackAlpha(g_shell);
    b->SetCbackBeta (manager);
    b->SetCbackMethodName("SpawnClassView");

    ASSIGN_ADD_GLASS(b, WGlButton, main_menu, "SolarSystem", 0);
    gs.SetNodeAdvance(b);
    b->SetCbackAlpha(g_shell);
    b->SetCbackBeta (ss);
    b->SetCbackMethodName("SpawnClassView");
  }
}
