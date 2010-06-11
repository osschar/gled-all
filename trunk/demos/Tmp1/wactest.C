#include <glass_defines.h>
#include <gl_defines.h>

void wactest()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Tmp1");

  Scene* arcs  = new Scene("Arcs");
  g_queen->CheckIn(arcs);
  g_queen->Add(arcs);
  g_scene = arcs;

  CREATE_ADD_GLASS(limo, ZGlLightModel, arcs, "LighModel", 0);
  limo->SetLightModelOp(0);

  CREATE_ADD_GLASS(blend, ZGlBlending, arcs, "Blending", 0);
  blend->SetBlendOp(1);
  blend->SetAntiAliasOp(1);
  blend->SetPointSize(2);

  CREATE_ADD_GLASS(cb, Sphere, arcs, "CameraBase", 0);
  cb->SetRadius(0.005);
  cb->SetPos(0, -1, 0);
  cb->RotateLF(1, 2, TMath::PiOver2());

  CREATE_ADD_GLASS(tr, TabletReader, arcs, "TabletReader", 0);
  tr->RotateLF(2, 3, TMath::PiOver2());

  CREATE_ADD_GLASS(rect, Rect, tr, "Rect", 0);
  rect->SetVLen(0.722);
  rect->SetColor(1, 1, 1, 0.3);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();
  g_pupil->SetMoveOM(-4);
  g_pupil->SetCHSize(0.01);
  g_pupil->SetCameraBase(cb);
  g_pupil->SetupZFov(60);
  g_pupil->EmitCameraHomeRay();

  g_shell->SpawnClassView(tr);

  ZMIR* start_mir = tr->S_StartRead();
  g_saturn->ShootMIR(start_mir);
}
