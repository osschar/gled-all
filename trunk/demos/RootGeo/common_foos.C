// $Id$

// Common functions for ROOT geometry demos.

#include <glass_defines.h>

Text_t* default_nest_layout = 0;

const Text_t* geometry_layout =
  "ZNode(RnrSelf[5],RnrElements[5],"
  "RnrOnForDaughters[8],RnrOffForDaughters[4]):"
  "ZGeoNode(Color[4],ImportNodes[4],NNodes[4],Material[7]):ZGeoOvl(Overlap[7])";

const Text_t* geometry_mini_layout =
  "ZNode(RnrSelf[5],RnrElements[5],"
  "RnrOnForDaughters[8],RnrOffForDaughters[4]):"
  "ZGeoNode(Color[4],Material[7])";

/**************************************************************************/

Scene* create_basic_scene()
{
  Scene* rscene = new Scene("Alice Detector Scene");
  g_queen->CheckIn(rscene);
  g_queen->Add(rscene);
  rscene->SetUseOM(true);

  g_scene = rscene;

  // Lamps.

  CREATE_ADD_GLASS(origin, ZNode, rscene, "Lamp Origin", "");

  CREATE_ADD_GLASS(lamp1, Lamp, origin, "Lamp1", "");
  lamp1->SetDiffuse(1, 1, 1);
  lamp1->SetAmbient(0.3, 0.3, 0.3);
  lamp1->SetLampScale(0);
  lamp1->SetPos(0, 12, 12);

  CREATE_ADD_GLASS(lamp2, Lamp, origin, "Lamp2", "");
  lamp2->SetDiffuse(0.65, 0.35, 0.5);
  lamp2->SetAmbient(0, 0, 0);
  lamp2->SetLampScale(0);
  lamp2->SetPos(12, 12, 0);

  rscene->GetGlobLamps()->Add(lamp1);
  rscene->GetGlobLamps()->Add(lamp2);

  CREATE_ADD_GLASS(el, Eventor, origin, "Dynamo", "Rotates Lamp Origin");
  el->SetBeatsToDo(-1); el->SetInterBeatMS(100); el->SetStampInterval(10);
  CREATE_ADD_GLASS(mv, Mover, el, "Lamp Origin Rotator", "");
  mv->SetNode(origin); mv->SetRi(1); mv->SetRj(3); mv->SetRa(0.005);

  // Camera bases.

  CREATE_ADD_GLASS(mark, ZNode, rscene, "Markers", "");
  CREATE_ADD_GLASS(cams, ZNode, mark, "CameraInfos", "");
  CREATE_ADD_GLASS(cam_bases, ZNode, mark, "CameraBases", "");

  CREATE_ATT_GLASS(nms, ZRlNodeMarkup, cam_bases, SetRnrMod, "NamesOn", 0);
  nms->SetRnrTiles(false); nms->SetRnrFrames(false);
  nms->SetTextCol(0.62, 1, 0.64);
  nms->SetTileCol(0,0,0,0);

  Float_t XCam = 10, YCam = 10, ZCam = 10;

  {
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "", "");
    base->SetPos(7, 4, 7);
    base->SetRotByDegrees(180, -45, 90);
    base->RotateLF(3, 1, 22*TMath::DegToRad());
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Default", "");
    info->SetCameraBase(base);
    info->SetUpReference(rscene);
    info->SetUpRefAxis(2);
    info->SetProjMode(CameraInfo::P_Perspective);
    info->SetupZFov(80);
  }

  { // Z-views
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "Z+", "");
    base->SetPos(0, 0, ZCam);
    base->SetRotByDegrees(0, -90, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto Z+", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }{
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "Z-", "");
    base->SetPos(0, 0, -ZCam);
    base->SetRotByDegrees(0, 90, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto Z-", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }
  { // Y-views
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "Y+", "");
    base->SetPos(0, YCam, 0);
    base->SetRotByDegrees(270, 0, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto Y+", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }{
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "Y-", "");
    base->SetPos(0, -YCam, 0);
    base->SetRotByDegrees(90, 0, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto Y-", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }
  { // X-views
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "X+", "");
    base->SetPos(XCam, 0, 0);
    base->SetRotByDegrees(0, 180, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto X+", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }{
    CREATE_ADD_GLASS(base, Sphere, cam_bases, "X-", "");
    base->SetPos(-XCam, 0, 0);
    base->SetRotByDegrees(0, 0, 270);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cams, "Orto X-", "");
    info->SetCameraBase(base);
    info->SetProjMode(CameraInfo::P_Orthographic);
    info->SetupZSize(12);
  }

  // Var.

  CREATE_ADD_GLASS(var, ZNode, rscene, "Var", 0);

  CREATE_ADD_GLASS(pointmod, ZGlBlending, var, "Blending", 0);
  pointmod->SetAntiAliasOp(1);
  pointmod->SetPointSize(4);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, var, "Light Model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(1);

  // y=0 plane.

  CREATE_ADD_GLASS(xzplane, Rect, rscene, "X-Z Plane", "");
  xzplane->SetRotByDegrees(180, 0, -90);
  xzplane->SetUnitSquare(20);
  xzplane->SetColor(0.4, 1, 0.45, 0.4);

  return rscene;
}

/**************************************************************************/

void setup_default_gui()
{
  Gled::AssertMacro("gled_view_globals.C");
  Gled::LoadMacro("eye.C");
  {
    ZList* laytop = register_GledCore_layouts();
    laytop->Swallow("RootGeo", new ZGlass("MiniGeometry", geometry_mini_layout));
    laytop->Swallow("RootGeo", new ZGlass("Geometry", geometry_layout));
  }
  if(default_nest_layout == 0)
    default_nest_layout = geometry_layout;

  Text_t* eye_name   = "Eye";
  Text_t* shell_name = "Shell";
  Text_t* pupil_name = "Pupil";

  if(g_shell == 0) {

    ASSIGN_ADD_GLASS(g_shell, ShellInfo, g_fire_queen, shell_name, "");

    CREATE_ATT_GLASS(g_nest, NestInfo, g_shell, SetDefSubShell, "Nest", 0);
    g_nest->Add(g_scene);
    g_nest->SetLayout(default_nest_layout);
    g_nest->SetLeafLayout(NestInfo::LL_Custom);

  }

  CREATE_ATT_GLASS(gui_pupil, GuiPupilInfo, g_shell, AddSubShell, "GuiPupil", "");

  CREATE_ATT_GLASS(pupil, PupilInfo, gui_pupil, SetPupil, pupil_name, "");
  pupil->SetupZFov(80);
  pupil->SetCHSize(0.03);
  pupil->SetBlend(1);
  pupil->Add(g_scene);

  gui_pupil->SetCameras((ZList*)g_scene->FindLensByPath("Markers/CameraInfos"));
  pupil->ImportCameraInfo((CameraInfo*)gui_pupil->GetCameras()->FrontElement());
}

void spawn_default_gui()
{
  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");
}
