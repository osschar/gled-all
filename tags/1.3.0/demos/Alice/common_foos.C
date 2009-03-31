// $Header$

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

void create_basic_scene()
{
  g_scene = new Scene("Alice Detector Scene");
  g_queen->CheckIn(g_scene);
  g_queen->Add(g_scene);
  g_scene->SetUseOM(true);

  // Lamps.

  CREATE_ADD_GLASS(origin, ZNode, g_scene, "Lamp Origin", "");

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

  g_scene->GetGlobLamps()->Add(lamp1);
  g_scene->GetGlobLamps()->Add(lamp2);

  CREATE_ADD_GLASS(el, Eventor, origin, "Dynamo", "Rotates Lamp Origin");
  el->SetBeatsToDo(-1); el->SetInterBeatMS(100); el->SetStampInterval(10);
  CREATE_ADD_GLASS(mv, Mover, el, "Lamp Origin Rotator", "");
  mv->SetNode(origin); mv->SetRi(1); mv->SetRj(3); mv->SetRa(0.005);

  // y=0 plane.

  CREATE_ADD_GLASS(xzplane, Rect, g_scene, "X-Z Plane", "");
  xzplane->SetRotByDegrees(180, 0, -90);
  xzplane->SetUnitSquare(20);
  xzplane->SetColor(0.4, 1, 0.45, 0.4);

  // Camera bases.

  Gled::LoadMacro("demo_scene_elements.C");
  dse_make_camera_bases(g_scene, g_scene, 2, 10, 7, 4, 7);

  // Var.

  CREATE_ADD_GLASS(var, ZNode, g_scene, "Var", 0);

  CREATE_ADD_GLASS(pointmod, ZGlBlending, var, "Blending", 0);
  pointmod->SetAntiAliasOp(1);
  pointmod->SetPointSize(4);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, var, "Light Model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(1);
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
    g_shell->SetDefSourceVis(false);

    ASSIGN_ATT_GLASS(g_nest, NestInfo, g_shell, SetDefSubShell, "Nest", 0);
    g_nest->Add(g_scene);
    g_nest->SetLayout(default_nest_layout);
    g_nest->SetLeafLayout(NestInfo::LL_Custom);

  }

  CREATE_ATT_GLASS(gui_pupil, GuiPupilInfo, g_shell, AddSubShell, "GuiPupil", "");

  CREATE_ATT_GLASS(pupil, PupilInfo, gui_pupil, SetPupil, pupil_name, "");
  pupil->SetZFov(80);
  pupil->SetCHSize(0.03);
  pupil->SetBlend(1);
  pupil->Add(g_scene);
  g_pupil = pupil;

  gui_pupil->SetCameras((ZList*)g_scene->FindLensByPath("Markers/CameraInfos"));
  pupil->ImportCameraInfo((CameraInfo*)gui_pupil->GetCameras()->FrontElement());
}

void spawn_default_gui()
{
  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");
}

/**************************************************************************/

Text_t* g_share_path = 0;

const Text_t* file_grep(const Text_t* file, const Text_t* pref="data")
{
  if(g_share_path == 0) {
    g_share_path = gSystem->Getenv("ALIGLEDSHARE");
    if(g_share_path == 0) {
      g_share_path = gSystem->WorkingDirectory();
    }
  }

  if(gSystem->AccessPathName(file, kReadPermission) == false)
    return file;

  const Text_t* f1 = GForm("%s/%s", pref, file);
  if(gSystem->AccessPathName(f1, kReadPermission) == false)
    return f1;

  const Text_t* f2 = GForm("%s/%s/%s", g_share_path, pref, file);
  if(gSystem->AccessPathName(f2, kReadPermission) == false)
    return f2;

  const Text_t* f3 = GForm("%s/%s", g_share_path, file);
  if(gSystem->AccessPathName(f3, kReadPermission) == false)
    return f3;

  // If not found ... let the consumer fail with local filename.
  return file;
}
