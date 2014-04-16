#include <glass_defines.h>
#include <gl_defines.h>

class GTSIsoMaker;
class GTSRetriangulator;
class TabletReader;

GTSIsoMaker       *g_isomaker = 0;
GTSRetriangulator *g_retriangulator = 0;

TabletReader      *g_reader = 0;

const Double_t LimXY = 0.7;
const Double_t LimZ  = 0.2;
const Double_t Delta = 0.005;

void setup_grid(GTSIsoMaker* maker,   Double_t delta=Delta,
		Double_t limxy=LimXY, Double_t limz=LimZ)
{
  Int_t nxy = TMath::Nint(2.0*LimXY/Delta);
  Int_t nz  = TMath::Nint(2.0*LimZ /Delta);
  maker->SetXAxis(-limxy, limxy, nxy);
  maker->SetYAxis(-limxy, limxy, nxy);
  maker->SetZAxis(-limz,  limz,  nz);
}


void wactest()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Tmp1");

  Scene* arcs  = new Scene("Arcs");
  g_queen->CheckIn(arcs);
  g_queen->Add(arcs);
  g_scene = arcs;

  CREATE_ADD_GLASS(l, Lamp, arcs, "Lamp", "");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->SetPos(-2, -6, 6);
  l->RotateLF(1,2, TMath::Pi());

  arcs->GetGlobLamps()->Add(l);

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
  g_reader = tr;

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
  setup_grid(isomaker);
  // Cartesian is usually better.
  // isomaker->SetAlgo(GTSIsoMaker::A_Tetra);
  isomaker->SetValue(1);
  isomaker->SetInvertCartesian(true);
  g_isomaker = isomaker;

  CREATE_ADD_GLASS(retring, GTSRetriangulator, arcs, "GTS Retriangulator", "Coarsens and refines GTS Surfaces");
  g_retriangulator = retring;

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

void make_high_res(Double_t fac=0.5, const Text_t* stem=0)
{
  setup_grid(g_isomaker, Delta * fac);

  g_isomaker->MakeSurface();

  g_retriangulator->SetStopOpts(GTSRetriangulator::SO_Cost);

  g_retriangulator->SetStopCost(Delta * fac);
  g_retriangulator->SetCostOpts(GTSRetriangulator::CO_Length);
  g_retriangulator->Coarsen();

  g_retriangulator->SetStopCost(0.4);
  g_retriangulator->SetCostOpts(GTSRetriangulator::CO_Angle);
  g_retriangulator->Coarsen();

  if (stem)
  {
    GTSurf *surf = g_isomaker->GetTarget();
    surf->MakeZSplitSurfaces(0.0, stem, true);
  }

  setup_grid(g_isomaker);
}
