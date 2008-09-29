// $Header$

// libs: Geom1 AliEnViz

#include <glass_defines.h>
#include <gl_defines.h>

/**************************************************************************/
/**************************************************************************/

class ZImage;
class Amphitheatre;
Amphitheatre* amph = 0;

class AEVDemoDriver;
class AEVMlClient;
class AEVDistAnRep;
AEVDemoDriver* demo_driver = 0;
AEVMlClient*   ml_client   = 0;
AEVDistAnRep*  distan_rep  = 0;

ScreenDumper* movie_dumper = 0;

void fake_replay()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("AliEnViz");

  Gled::LoadMacro("alienviz_common_foos.C");

  alienviz_setup_lib_objects(128*1024);

  ASSIGN_ADD_GLASS(ml_client, AEVMlClient, aev_queen, "MonaLisa Client", 0);

  /**************************************************************************/

  alienviz_setup_scene();

  Scene* ss = g_scene;

  CREATE_ADD_GLASS(blend, ZGlBlending, ss, "GL Blending", 0);
  blend->SetBlendOp(ZRnrModBase::O_On);
  blend->SetAntiAliasOp(ZRnrModBase::O_On);
  blend->SetLineWidth(1);
  blend->SetPointSize(16);
  // fog
  blend->SetFogOp(ZRnrModBase::O_On);
  blend->SetFogMode(GL_LINEAR);
  blend->SetFogColor(0.222,0.049,0.115,1.000);
  blend->SetFogBeg(6);
  blend->SetFogEnd(16);

  CREATE_ADD_GLASS(plane_xy, Rect, ss, "Plane XY", 0);
  plane_xy->SetUnitSquare(20);
  plane_xy->SetColor(0,0,1,0.5);
  plane_xy->SetPos(0,0,-0.05);
  plane_xy->SetRotByDegrees(0,0,0);
  plane_xy->SetWidth(1);

  ASSIGN_ADD_GLASS(amph, Amphitheatre, ss, "Amphitheatre", 0);
  amph->SetPos(0, 0, 0);
  amph->SetRotByDegrees(90,0,0);
  amph->SetChairSize(0.4);
  amph->SetGuestSize(0.8);
  amph->SetRnrStage(false);
  amph->SetChairCol(0.111, 0.932, 1.000);
  // Chair creation
  amph->CreateChairs(5, -4, 1.5, 60, 4);
  amph->CreateChairs(6, -4, 2.5, 80, 6);
  amph->CreateChairs(7, -4, 3.5, 90, 8);
  // Chair hunt
  amph->SetInnerHunt(false);

  CREATE_ADD_GLASS(c_mapviz, AEVMapViz, ss, "Chunked PDC-2004 MapViz", 0);
  c_mapviz->SetPos(1, -3, 0);
  c_mapviz->SetScale(6); c_mapviz->SetUseScale(true);
  c_mapviz->CutEarth_PDC04(find_texture("Earth"));
  // globe_map(ss, alien_ui)     // Globe map
  // mercator_map(ss, alien_ui); // Mercator all-earth map

  ASSIGN_ADD_GLASS(distan_rep, AEVDistAnRep, ss, "Distributed Analysis", 0);
  AEVDistAnRep* da = distan_rep;

  printf("Creating visualization objects ...\n");

  CREATE_ADD_GLASS(mona_viz_node, ZNode, ss, "MonaLisaNode", 0);
  mona_viz_node->SetPos(-4, 0, 0.5);
  mona_viz_node->SetRnrMod(find_rnrmod("NamesOn"));
  CREATE_ADD_GLASS(mona_viz, SMorph, mona_viz_node, "MonALISA", 0);
  mona_viz->SetScale(0.9);
  mona_viz->SetColor(1.000, 0.713, 0.269);
  mona_viz->SetTLevel(30); mona_viz->SetPLevel(30);
  mona_viz->SetTexture(find_texture("PROOF"));
  mona_viz->SetTexXC(2);
  mona_viz->SetTexYC(2);

  CREATE_ADD_GLASS(gled_viz_node, ZNode, ss, "GledNode", 0);
  gled_viz_node->SetPos(4, 0, 0.5);
  gled_viz_node->SetRnrMod(find_rnrmod("NamesOn"));
  CREATE_ADD_GLASS(gled_viz, WSSeed, gled_viz_node, "Gled", 0);
  // Could move it somewhat and rotate it in parent frame.
  gled_viz->SetPos(0, 0, 0);
  gled_viz->RotateLF(3, 1, 0.35*TMath::Pi());
  gled_viz->SetRnrMod(find_rnrmod("NamesOff"));
  gled_viz->SetTexture(find_texture("Gled"));
  gled_viz->MakeLissajou(0, TMath::TwoPi(), 16,
			 0.7, 2, 0,
			 0.7, 3, 0,
			 0.7, 2, TMath::Pi()/2,
			 0.1);
  gled_viz->SetDtexU(0.05);
  gled_viz->SetRenormLen(true);

  /*
    CREATE_ADD_GLASS(stoneball, SMorph, ss, "StoneBall", 0);
    stoneball->SetPos(0, 0, -0.2);
    stoneball->SetScales(1.2, 1.2, 0.1);
    stoneball->SetTLevel(30); stoneball->SetPLevel(30);
    // stoneball->SetColor(1, 0, 1);
    stoneball->SetTexture(glite_tex);
  */

  /**************************************************************************/
  // Setup demo-driver
  /**************************************************************************/

  CREATE_ADD_GLASS(dd, AEVDemoDriver, ss, "DemoDriver", 0);
  demo_driver = dd;
  dd->SetFakeProcessing(true); // !!! Fake ON !!!
  dd->SetPos(0, -5.55, 0);
  dd->SetCenter(true);
  dd->SetAlpha(true);
  dd->SetBackPoly(false);
  dd->SetFramePoly(false);
  dd->SetBGCol(0, 0, 0, 0); // Completely transparent!
  dd->SetFGCol(0.1, 1, 0.1);
  dd->SetFont(find_font("LargeFont"));

  dd->SetMonaClient(ml_client);
  dd->SetDistAnRep(distan_rep);
  dd->SetTheatre(amph);
  dd->SetMapViz(c_mapviz);
  dd->SetMonaViz(mona_viz);
  dd->SetGledViz(gled_viz);
  dd->SetTexCont(aev_textures);

  dd->InitAnimationStuff();

  dd->SetRnrSelf(false); // !!! Text now in info_bar

  // Cross-links

  da->SetDemoDriver(dd);
  c_mapviz->SetSites(da->GetSites());

  // Gled, amph, (mona now still) animation

  dd->GetAnimTimer()->AddClient(gled_viz);
  dd->GetAnimTimer()->AddClient(amph);

  CREATE_ATT_GLASS(gled_rot, Mover, dd->GetAnimator(), Add, "Gled Rotator", 0);
  gled_rot->SetRi(1);
  gled_rot->SetRj(2);
  gled_rot->SetRa(0.03);
  gled_rot->SetNode(gled_viz);


  ASSIGN_ATT_GLASS(movie_dumper, ScreenDumper, dd->GetAnimator(), Add, "Movie Dumper", 0);
  movie_dumper->SetOpActive(false);

  /**************************************************************************/
  // Overlay
  /**************************************************************************/

  CREATE_ADD_GLASS(overlay, Scene, g_queen, "Overlay", 0);

  CREATE_ADD_GLASS(ovl_lm, ZGlLightModel, overlay, "LightOff", 0);
  ovl_lm->SetLightModelOp(0);

  Float_t weed_dx = 160, step_dx = 180, step_dy = 26;

  CREATE_ADD_GLASS(ibpp, ZGlPerspective, overlay, "InfoPerspective", 0);
  ibpp->StandardFixed();

  CREATE_ADD_GLASS(info_bar, Text, overlay, "InfoText", 0);
  info_bar->SetPos(5, 9.4, 0);
  info_bar->SetScales(0.5, 0.8, 1);
  info_bar->SetFont(find_font("LargeFont"));
  info_bar->SetBackPoly(false);
  info_bar->SetFramePoly(false);
  info_bar->SetFGCol(0.6, 1, 0.6);

  dd->SetInfoBar(info_bar);

  CREATE_ADD_GLASS(blurp_bar, Text, overlay, "BlurpText", 0);
  blurp_bar->SetPos(5, 9, 0);
  blurp_bar->SetScales(0.25, 0.4, 1);
  // blurp_bar->SetFont(find_font("LargeFont"));
  blurp_bar->SetBackPoly(false);
  blurp_bar->SetFramePoly(false);
  blurp_bar->SetFGCol(0.6, 0.6, 1);

  dd->SetBlurpBar(blurp_bar);

  CREATE_ADD_GLASS(pp, ZGlPerspective, overlay, "Perspective", 0);
  pp->StandardPixel();
  pp->SetOx(5); pp->SetOy(13*step_dy); pp->SetOz(0);

  CREATE_ADD_GLASS(gl_dir, WGlDirectory, overlay, "Job Directory", "");
  gl_dir->SetRnrMod(find_rnrmod("DirectoryFrameStyle"));
  gl_dir->SetRnrSelf(false);
  gl_dir->SetContents(ml_client);
  gl_dir->StandardPixel();
  gl_dir->SetNy(10);
  gl_dir->SetDy(-step_dy);
  gl_dir->SetDrawTitle(true);
  gl_dir->SetNameFraction(0.15);
  gl_dir->SetTextDx(600);
  gl_dir->SetTextDy(0); // Take from frame
  gl_dir->SetCbackAlpha(dd);
  gl_dir->SetCbackMethodName("QueryJobDetails");
  gl_dir->SetCbackBetaType("");

  dd->SetJobDirectory(gl_dir);

  // Menus
  //------

  CREATE_ADD_GLASS(mpp, ZGlPerspective, overlay, "MenuPerspective", 0);
  mpp->StandardPixel();
  mpp->SetOx(5); mpp->SetOy(5); mpp->SetOz(0);

  {
    CREATE_ADD_GLASS(top_menu, ZNode, overlay, "MainMenu", 0);
    top_menu->SetRnrMod(find_rnrmod("MenuFrameStyle"));
    dd->SetMenuFrameStyle(dynamic_cast<WGlFrameStyle*>(find_rnrmod("MenuFrameStyle")));

    SGridStepper top_step(0);
    top_step.SetDs(step_dx, step_dy, 1);
    top_step.SetNs(4, 4, 1);

    // ---

    CREATE_ADD_GLASS(mona_connect, WGlButton, top_menu, "Connect MonALISA", 0);
    top_step.SetNode(mona_connect);
    mona_connect->SetCbackAlpha(dd);
    mona_connect->SetCbackMethodName("ConnectMonaLisa");

    top_step.Step();

    CREATE_ADD_GLASS(job_menu, WGlButton, top_menu, "Job Menu", 0);
    top_step.SetNode(job_menu);
    job_menu->SetCbackAlpha(job_menu);
    job_menu->SetCbackMethodName("MenuEnter");

    job_menu->SetRnrElements(false);
    {
      SGridStepper step(0);
      step.Subtract(top_step);
      step.SetDs(step_dx, step_dy, 1);
      step.SetNs(4, 4, 1);

      CREATE_ADD_GLASS(but1, WGlButton, job_menu, "Querry Jobs ...", 0);
      step.SetNodeAdvance(but1);
      but1->SetCbackAlpha(dd);
      but1->SetCbackMethodName("QueryJobs");

      CREATE_ADD_GLASS(but2, WGlButton, job_menu, "Replay Job", 0);
      step.SetNode(but2);
      but2->SetCbackAlpha(dd);
      but2->SetCbackBeta(but2);
      but2->SetCbackMethodName("ReplayJob");

      but2->SetRnrElements(false);
      {
	SGridStepper sstep(0);
	sstep.Subtract(top_step);
	sstep.SetDs(step_dx, step_dy, 1);
	sstep.SetNs(4, 4, 1);

	CREATE_ADD_GLASS(but12, WGlButton, but2, "Pause", 0);
	sstep.SetNodeAdvance(but12);
	but12->SetCbackAlpha(dd);
	but12->SetCbackMethodName("PauseJobReplay");

	CREATE_ADD_GLASS(but13, WGlButton, but2, "Resume", 0);
	sstep.SetNodeAdvance(but13);
	but13->SetCbackAlpha(dd);
	but13->SetCbackMethodName("DoJobReplay");

	CREATE_ADD_GLASS(but11, WGlButton, but2, "Finalize", 0);
	sstep.SetNodeAdvance(but11);
	but11->SetCbackAlpha(dd);
	but11->SetCbackMethodName("FinalizeJob");

	CREATE_ADD_GLASS(back, WGlButton, but2, "Exit Replay ", 0);
	sstep.SetNodeAdvance(back);
	back->SetCbackAlpha(dd);
	back->SetCbackBeta(but2);
	back->SetCbackMethodName("ExitReplayMenu");
      }
      step.Step();

      CREATE_ADD_GLASS(but3, WGlButton, job_menu, "Cycle Job", 0);
      step.SetNode(but3);
      but3->SetCbackAlpha(dd);
      but3->SetCbackBeta(but2);
      but3->SetCbackMethodName("CycleJob");

      but3->SetRnrElements(false);
      step.Step();

      /*

      CREATE_ADD_GLASS(but3, WGlButton, job_menu, "Follow Last", 0);
      step.SetNodeAdvance(but3);
      //but2->SetCbackAlpha(rot_eventor);
      //but2->SetCbackMethodName("Stop");

      CREATE_ADD_GLASS(but4, WGlButton, job_menu, "Replay&Follow Last", 0);
      step.SetNodeAdvance(but4);
      //but2->SetCbackAlpha(rot_eventor);
      //but2->SetCbackMethodName("Stop");

      CREATE_ADD_GLASS(but5, WGlButton, job_menu, "Wait New", 0);
      step.SetNodeAdvance(but5);
      //but2->SetCbackAlpha(rot_eventor);
      //but2->SetCbackMethodName("Stop");

      */

      CREATE_ADD_GLASS(back, WGlButton, job_menu, " << ", 0);
      step.SetNodeAdvance(back);
      back->SetCbackAlpha(job_menu);
      back->SetCbackMethodName("MenuExit");
    }

    top_step.Step();

    // ----
    CREATE_ADD_GLASS(mona_disconnect, WGlButton, top_menu, "Disconnect MonALISA", 0);
    top_step.SetNode(mona_disconnect);
    mona_disconnect->SetCbackAlpha(dd);
    mona_disconnect->SetCbackMethodName("DisconnectMonaLisa");

    top_step.Step();

    CREATE_ADD_GLASS(exit_but, WGlButton, top_menu, "Exit", 0);
    top_step.SetNode(exit_but);
    exit_but->MoveLF(1, 40);
    exit_but->SetCbackAlpha(exit_but);
    exit_but->SetCbackMethodName("ExitGled");

    top_step.Step();
  }

  /**************************************************************************/

  /*
    CREATE_ADD_GLASS(etor, Eventor, aev_queen, "Movie Maker", 0);
    etor->SetBeatsToDo(-1);
    etor->SetInterBeatMS(100); etor->SetStampInterval(10);
    CREATE_ADD_GLASS(sdumper, ScreenDumper, etor, "DumpRayEmitter", 0);
    sdumper->SetPupil(pupil);
    // sdumper->SetFileNameFmt("someplace/img%05d.tga");
    */

  dd->ResetDemo();

  /**************************************************************************/
  // Spawn the eye
  /**************************************************************************/

  alienviz_spawn_eye();

  g_pupil->SetLookAt(aev_scene);
  g_pupil->SetOverlay(overlay);

  movie_dumper->SetPupil(g_pupil);

  /**************************************************************************/

  { // Buttons for camera positions.

    CREATE_ADD_GLASS(mpp, ZGlPerspective, overlay, "CameraNavPerspective", 0);
    mpp->StandardPixel();
    mpp->SetOx(5); mpp->SetOy(-(5+2*step_dy)); mpp->SetOz(0);

    CREATE_ADD_GLASS(locfs, WGlFrameStyle, overlay, "CameraNavStyle", 0);
    locfs->StandardPixel();
    locfs->SetDefDx(weed_dx/2.5);
    locfs->SetTextYSize(18);
    locfs->SetYBorder(2);

    SGridStepper step(0);
    step.SetDs(step_dx, -step_dy, 1);
    step.SetNs(1, 4, 1);

    CREATE_ADD_GLASS(b_home, WGlButton, overlay, "Home", 0);
    step.SetNodeAdvance(b_home);
    b_home->SetCbackAlpha(g_pupil);
    b_home->SetCbackBeta(find_cambase("Home"));
    b_home->SetCbackMethodName("SmoothCameraHome");

    CREATE_ADD_GLASS(b_pos_1, WGlButton, overlay, "Pos A", 0);
    step.SetNodeAdvance(b_pos_1);
    b_pos_1->SetCbackAlpha(g_pupil);
    b_pos_1->SetCbackBeta(find_cambase("View1"));
    b_pos_1->SetCbackMethodName("SmoothCameraHome");

    CREATE_ADD_GLASS(b_pos_2, WGlButton, overlay, "Pos B", 0);
    step.SetNodeAdvance(b_pos_2);
    b_pos_2->SetCbackAlpha(g_pupil);
    b_pos_2->SetCbackBeta(find_cambase("View2"));
    b_pos_2->SetCbackMethodName("SmoothCameraHome");
  }

  // x1();
  // x2();
}

/**************************************************************************/

void x1()
{
  ml_client->FindJobs();

  distan_rep->SetJobName("37303");
  distan_rep->InitJob();
  distan_rep->StudyJobHistory();
}

void x2()
{
  distan_rep->SendEvBatchesToTheatre();
  amph->StartHunt();
}

void movie_capture()
{
  Eventor* e = demo_driver->GetAnimator();
  e->Stop();
  e->Reset();
  e->SetInterBeatMS(1);
  e->SetEpochType(Eventor::ET_Manual);
  e->SetTimeSource(Eventor::TS_IntStep);
  e->SetTimeEpoch(0);
  e->SetTimeStep(0.04); // 25 fps

  demo_driver->SetFPS(25); // 25 fps
  demo_driver->SetRPS(7);  // 800x600 = 1.4M, 10Mb/s write speed

  movie_dumper->SetFileNameFmt("/svoanet/foo/aev-capture/v2/img-%04d");
  movie_dumper->SetWaitDump(true);
  movie_dumper->SetDumpID(0);
  movie_dumper->SetOpActive(true);

  g_pupil->SetMPSize(0.05);
  g_pupil->SetHomeAnimTime(3.5);
  g_pupil->SetRnrFakeOverlayInCapture(true);

  // Is there a way to tell shell to raise/show full-class-view of a given lens?
}
