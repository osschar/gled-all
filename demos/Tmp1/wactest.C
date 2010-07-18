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

  CREATE_ADD_GLASS(limooff, ZGlLightModel, arcs, "LightsOff", 0);
  limooff->SetLightModelOp(0);

  CREATE_ADD_GLASS(blend, ZGlBlending, arcs, "Blending", 0);
  blend->SetBlendOp(1);
  blend->SetAntiAliasOp(1);
  blend->SetPointSize(2);

  CREATE_ADD_GLASS(cb, Sphere, arcs, "CameraBase", 0);
  cb->SetRadius(0.005);
  cb->SetPos(0, -1, 0);
  cb->RotateLF(1, 2, TMath::PiOver2());

  CREATE_ADD_GLASS(rmod, TabletRnrMod, arcs, "TabletRnrMod", 0);

  CREATE_ADD_GLASS(tr, TabletReader, arcs, "TabletReader", 0);
  tr->RotateLF(2, 3, TMath::PiOver2());

  CREATE_ADD_GLASS(recti, Rect, tr, "Inner Grid", 0);
  recti->SetVLen(0.722);
  recti->SetColor(1, 1, 1, 0.3);
  recti->SetPos(0, 0, -0.001);
  CREATE_ADD_GLASS(recto, Rect, tr, "Outer Grid", 0);
  recto->SetUStrips(30);
  recto->SetVStrips(30);
  recto->SetULen(3);
  recto->SetVLen(3*0.722);
  recto->SetColor(1, 1, 1, 0.1);
  recto->SetPos(0, 0, -0.001);

  CREATE_ADD_GLASS(isomaker, GTSIsoMaker, arcs, "Iso Maker", 0);
  isomaker->SetXAxis(-0.7, 0.7, 280);
  isomaker->SetYAxis(-0.7, 0.7, 280);
  isomaker->SetZAxis(-0.2, 0.2,  80);
  // Cartesian is usually better.
  // isomaker->SetAlgo(GTSIsoMaker::A_Tetra);
  isomaker->SetValue(1);

  CREATE_ADD_GLASS(retring, GTSRetriangulator, arcs, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");

  CREATE_ADD_GLASS(limoon, ZGlLightModel, arcs, "LightsOn", 0);
  limoon->SetLightModelOp(1);

  CREATE_ADD_GLASS(surf, GTSurf, arcs, "IsoSurface", 0);
  surf->SetColor(1, 0.8, 0.2);
  surf->RotateLF(2, 3, TMath::PiOver2());

  isomaker->SetTarget(surf);
  retring->SetTarget(surf);


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
