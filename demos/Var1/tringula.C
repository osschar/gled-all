#include <glass_defines.h>
#include <gl_defines.h>

class Tringula;
class TringuCam;
class Dynamico;
class TriMesh;
class TriMeshField;
class TriMeshLightField;
class RectTerrain;
class GTSurf;
class GTSRetriangulator;
class GTSIsoMaker;
class TimeMaker;

Tringula*     tringula = 0;
TringuCam*    tricam   = 0;
Dynamico*     dyn1     = 0;
Dynamico*     dyn2     = 0;
TriMesh*      trimesh  = 0;
TriMesh*      carmesh  = 0;
TriMeshField* tmfield  = 0;
RectTerrain * terrain  = 0;
GTSurf*       gtsurf   = 0;
GTSRetriangulator* gtsretring  = 0;
GTSIsoMaker*       gtsisomaker = 0;

TriMeshLightField* lightmap = 0;

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
  // Ribbon
  CREATE_ADD_GLASS(ribbon, ZRibbon, texdir, "Jungle Ribbon", 0);
  ribbon->SetPOVFile("ribbon1.pov");
  ribbon->LoadPOV();
  // RGBAPalette
  CREATE_ADD_GLASS(pal, RGBAPalette, texdir, "Spectrum Palette", 0);
  pal->SetUnderflowAction(RGBAPalette::LA_Wrap);
  pal->SetOverflowAction (RGBAPalette::LA_Wrap);
  gStyle->SetPalette(1, 0);
  pal->SetMarksFromgStyle();

  CREATE_ADD_GLASS(sphere, Sphere, g_scene, "Sphere", "");
  sphere->SetRadius(.3);
  sphere->SetLOD(20);
  sphere->SetColor(.8, .2, .6);

  CREATE_ADD_GLASS(lamp, Lamp, g_scene, "Lamp", "");
  lamp->SetPos(5, 5, 5);
  // lamp->SetDrawLamp(false);
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
  hf->SetFile("gforge.png");
  terrain->SetFromImage(hf, 40); // 5 for 32x32, 20 for 256x256 file "gforge.png"
  //hf->SetFile("gforge-2x2.png");
  //terrain->SetFromImage(hf, 1);
  //terrain->SetRnrMode(RectTerrain::RM_SmoothTring);
  terrain->SetRnrMode(RectTerrain::RM_FlatTring);
  terrain->SetBorderCond(RectTerrain::BC_Wrap);
  terrain->ApplyBorderCondition();
  terrain->SetBorder(true);
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
  trimesh->StdSurfacePostImport();

  ASSIGN_ADD_GLASS(carmesh, TriMesh, g_scene, "Car TriMesh", 0);
  carmesh->MakeTetrahedron();
  carmesh->StdDynamicoPostImport();

  ASSIGN_ADD_GLASS(tringula, Tringula, g_scene, "Tringula 1", 0);
  tringula->SetMesh(trimesh);
  tringula->SetPalette(pal);
  tringula->SetEdgePlanes(terrain);
  tringula->SetRnrRay(true);
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetDefDynMesh(carmesh);

  dyn1 = tringula->NewDynamico("Dynus Primus");
  dyn2 = tringula->NewDynamico("Dynus Secondus");
  dyn2->ref_trans().Move3LF(0.2, 0.01, 0.05);
  dyn2->ref_trans().RotateLF(1, 3, 0.4);
  

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
  /************************g**************************************************/

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

  Gled::AssertMacro("gled_view_globals.C");
  Gled::LoadMacro("eye.C");

  /**************************************************************************/

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
  g_pupil->SetNearClip(2);
  g_pupil->SetFarClip(200);

  // g_pupil->SetBackMode(GL_FILL);
  // g_pupil->SetCHSize(0);

  g_pupil->SetShowRPS(false);
  g_pupil->SetShowView(false);

  g_pupil->SetAutoRedraw(false);
  // g_pupil->SetOverlay(overlay);

  ASSIGN_ADD_GLASS(tricam, TringuCam, tringula, "TringuCam", 0);
  tricam->SetTringula(tringula);
  tricam->SetPos(4, 2, 10);
  tricam->RotateLF(1, 2, 0.25*TMath::Pi());

  ASSIGN_ADD_GLASS(tmfield, TriMeshField, tricam, "TMField", 0);
  tmfield->SetMesh(trimesh);
  tmfield->ResizeToMesh();
  tmfield->SetPalette(pal);
  tmfield->FillByXYGaussBlobs();
  tmfield->FindMinMaxField();
  tmfield->SetMinValue(0);
  tmfield->SetMaxValue(TMath::Ceil(tmfield->GetMaxValue()));

  tricam->SetCurField(tmfield);
  tmfield->ColorizeTvor();

  ASSIGN_ADD_GLASS(lightmap, TriMeshLightField, tricam, "TMLightField", 0);
  lightmap->SetMesh(trimesh);
  lightmap->ResizeToMesh();
  lightmap->SetLampPos(5, 5, 20);
  lightmap->SetPalette(pal);
  lightmap->CalculateLightField();

  g_shell->Add(tricam);
  g_pupil->SetEventHandler(tricam);
  g_pupil->SetCameraBase(tricam);

  tmaker->AddClient(tricam);

  CREATE_ADD_GLASS(stxt1, ScreenText, g_pupil, "LftRgt", 0);
  tricam->SetTxtLftRgt(stxt1);

  /**************************************************************************/

  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");

  /**************************************************************************/
  // And start the time ...
  /**************************************************************************/

  dumper->SetPupil(g_pupil);
  eventor->Start();
}

void sunrise(Float_t phi=45, Float_t dt=0.005, Int_t sleep=100)
{
  // phi given in degrees.

  phi *= TMath::DegToRad();

  tringula->SetLightMesh(false);
  lightmap->SetDirectional(true);

  for (Double_t t=0.1; t<TMath::PiOver2(); t+=dt)
  {
    lightmap->SetupLampDir(tringula, t, phi);
    lightmap->CalculateLightField();
    tringula->ReadLock();
    tmfield->ColorizeTvor();
    lightmap->ModulateTvor();
    tringula->ReadUnlock();
    gSystem->Sleep(sleep);
  }
}
