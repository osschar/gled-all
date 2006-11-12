#include <glass_defines.h>
#include <gl_defines.h>

class Tringula;
class Dynamico;
class TriMesh;
class RectTerrain;
class GTSurf;
class GTSRetriangulator;
class GTSIsoMaker;
class TimeMaker;

Tringula*    tringula = 0;
Dynamico*    dyn1     = 0;
TriMesh*     trimesh  = 0;
TriMesh*     carmesh  = 0;
RectTerrain* terrain  = 0;
GTSurf*      gtsurf   = 0;
GTSRetriangulator* gtsretring  = 0;
GTSIsoMaker*       gtsisomaker = 0;

Eventor*     eventor  = 0;
TimeMaker*   tmaker   = 0;

void tringula()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Tringula Scene", "Testing functionality of Var1::Tringula glass.");

  CREATE_ADD_GLASS(texdir, ZList, g_scene, "Textures", 0);
  // Height-field
  CREATE_ADD_GLASS(hf, ZImage, texdir, "HeightField", 0);
  // GForger
  CREATE_ADD_GLASS(gforge, GForger, texdir, "GForger", 0);
  gforge->SetImage(hf);

  CREATE_ADD_GLASS(sphere, Sphere, g_scene, "Sphere", "");
  sphere->SetRadius(.3);
  sphere->SetLOD(20);
  sphere->SetColor(.8, .2, .6);

  CREATE_ADD_GLASS(lamp, Lamp, g_scene, "Lamp", "");
  lamp->SetPos(5, 5, 5);
  lamp->SetDrawLamp(false);
  // lamp->SetLampScale(1);
  // This crashes my laptop together with glDrawArrays of tringtvor ... badly.
  // Must be a weird ATI bug.
  g_scene->GetGlobLamps()->Add(lamp);

  CREATE_ADD_GLASS(base_plane, Rect, g_scene, "BasePlane", 0);
  base_plane->SetUnitSquare(20);
  base_plane->SetColor(0.6, 0.8, 0.6);

  ASSIGN_ADD_GLASS(terrain, RectTerrain, g_scene, "RectTerrain", 0);
  terrain->SetDx(2);
  terrain->SetDy(2);
  terrain->SetMinCol(0.175, 0.689, 0.338);
  terrain->SetMaxCol(0.139, 0.384, 1.000);
  //hf->SetFile("gforge.png");
  //terrain->SetFromImage(hf, 20);
  hf->SetFile("gforge-16x16.png");
  terrain->SetFromImage(hf, 5);
  //terrain->SetRnrMode(RectTerrain::RM_SmoothTring);
  terrain->SetRnrMode(RectTerrain::RM_FlatTring);
  terrain->SetUseTringStrips(false);
  terrain->SetRnrSelf(false);
  gforge->SetTerrain(terrain);

  ASSIGN_ADD_GLASS(gtsurf, GTSurf, g_scene, "GTSsurf", 0);
  gtsurf->SetFile("torus.gts");
  // gtsurf->Load();
  ASSIGN_ADD_GLASS(gtsretring,  GTSRetriangulator, g_scene, "GTSRetriangulator", 0);
  gtsretring->SetTarget(gtsurf);
  ASSIGN_ADD_GLASS(gtsisomaker, GTSIsoMaker, g_scene, "GTSIsoMaker", 0);
  gtsisomaker->SetTarget(gtsurf);


  ASSIGN_ADD_GLASS(trimesh, TriMesh, g_scene, "Terrain TriMesh", 0);
  trimesh->ImportRectTerrain(terrain);

  ASSIGN_ADD_GLASS(carmesh, TriMesh, g_scene, "Car TriMesh", 0);
  carmesh->MakeTetrahedron();

  ASSIGN_ADD_GLASS(tringula, Tringula, g_scene, "Tringula 1", 0);
  tringula->SetMesh(trimesh);
  tringula->MakeOpcodeModel();
  tringula->SetEdgePlanes(terrain);
  tringula->RayCollide();
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetDefDynMesh(carmesh);

  ASSIGN_ADD_GLASS(dyn1, Dynamico, tringula->GetDynos(), "Dynamico Primus", 0);
  dyn1->SetMesh(carmesh);

  // Best coarsening:
  gtsurf->SetRnrSelf(false);
  trimesh->ExportGTSurf(gtsurf);
  gtsretring->SetStopNumber(800);
  gtsretring->SetCostOpts(GTSRetriangulator::CO_Volume);
  gtsretring->SetVO_ShapeWght(1e-8);
  gtsretring->SetMidvertOpts(GTSRetriangulator::MO_Volume);
  //gtsretring->Set

  /**************************************************************************/
  // Eventor & Operators
  /**************************************************************************/

  ASSIGN_ADD_GLASS(eventor, Eventor, g_queen, "Eventor", 0);
  eventor->SetInterBeatMS(10);

  ASSIGN_ADD_GLASS(tmaker, TimeMaker, eventor, "TimeMaker", 0);
  tmaker->AddClient(tringula);
  
  CREATE_ADD_GLASS(dumper, ScreenDumper, eventor, "ScreenDumper", 0);
  dumper->SetFileNameFmt("");
  dumper->SetWaitDump(true);

  /**************************************************************************/
  // Spawn the eye
  /**************************************************************************/

  printf("Instantiating GUI ...\n");

  //g_saturn->LockMIRShooters(true);
  g_shell = new ShellInfo("Player Shell");
  // fire_queen->CheckIn(shell); fire_queen->Add(shell);
  g_queen->CheckIn(g_shell);
  g_queen->Add(g_shell);

  ASSIGN_ATT_GLASS(g_nest, NestInfo, g_shell, SetDefSubShell, "Nest", 0);
  g_nest->Add(g_queen);
  // g_nest->Add(g_scene);
  // g_nest->ImportKings();   // Get all Kings as top level objects

  ASSIGN_ATT_GLASS(g_pupil, PupilInfo, g_shell, AddSubShell, "Pupil of Tringula", 0);
  g_pupil->Add(g_scene);
  g_pupil->SetWidth(800); g_pupil->SetHeight(480);
  g_pupil->SetClearColor(0.06, 0.01, 0.02);
  g_pupil->SetUpReference(base_plane);
  g_pupil->SetUpRefAxis(3);
  g_pupil->SetCameraBase(0);
  // g_pupil->SetBackMode(GL_FILL);
  // g_pupil->SetCHSize(0);

  g_pupil->SetShowRPS(false);
  g_pupil->SetShowView(false);

  g_pupil->SetAutoRedraw(false);
  // g_pupil->SetOverlay(overlay);

  /**************************************************************************/

  Gled::AssertMacro("gled_view_globals.C");
  Gled::LoadMacro("eye.C");

  /**************************************************************************/

  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");

  /**************************************************************************/
  // And start the time ...
  /**************************************************************************/

  dumper->SetPupil(g_pupil);
  eventor->Start();
}
