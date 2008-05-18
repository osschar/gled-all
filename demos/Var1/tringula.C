#include <glass_defines.h>
#include <gl_defines.h>

// #include <exception>

class Tringula;
class TringuCam;
class Statico;
class Dynamico;
class ParaSurf;
class PSMark;
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
Statico*      sta1     = 0;
Dynamico*     dyn1     = 0;
Dynamico*     dyn2     = 0;
PSMark*       mark     = 0;

ParaSurf*     parasurf = 0;
TriMesh*      trimesh  = 0;
TriMesh*      dynmesh  = 0;
TriMesh*      stamesh  = 0;
TriMesh*      flymesh  = 0;
ZVector*      rndstatos = 0;
ZVector*      rnddynos = 0;

Scene*        overlay  = 0;

TriMeshField* tmfield  = 0;
RectTerrain * terrain  = 0;
GTSurf*       gtsurf   = 0;
GTSRetriangulator* gtsretring  = 0;
GTSIsoMaker*       gtsisomaker = 0;

TriMeshLightField* lightmap = 0;

Eventor*     eventor  = 0;
TimeMaker*   tmaker   = 0;

ZList*       tmpdir   = 0;

// Some of the parameters are modified for some modes.

Float_t      statico_surface_fraction  = 0.05; // 0.05
Int_t        num_dynamico              = 250;  // 250
Int_t        num_flyer                 = 100;  // 100
Float_t      max_flyer_h               = 10;   // def=10. Changed for some surfaces in surf-specific init.

TRandom      g_rnd(0);

const Text_t* trimesh_layout = "ZGlass(Name,Title[22]):TriMesh(M[8],Surf[8],COM,J)";

void tringula(Int_t mode=0)
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  g_queen->SetName("TringulaQueen");
  g_queen->SetTitle("Demo of Tringula");

  /**************************************************************************/
  // Database objects.
  /**************************************************************************/

  tmpdir = g_queen->AssertPath("tmp", "ZNameMap");

  { // Textures

    ZList* texdir = g_queen->AssertPath("var/textures", "ZNameMap");
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
  }
  { // Color gradients

    ZList* graddir = g_queen->AssertPath("var/gradients", "ZNameMap");

    void*   dirh = gSystem->OpenDirectory("gradients");
    TString file;
    TArrayI mp;
    TPRegexp re("lin_([\\w\\d]+)\\.\\w+");
    while ((file = gSystem->GetDirEntry(dirh)).IsNull() == kFALSE)
    {
      Int_t nm = re.Match(file, "", 0, 30, &mp);
      if (nm == 2)
      {
        TString name(file(mp[2], mp[3] - mp[2]));
        printf("file='%s' nm = %d, name='%s'\n", file.Data(), nm, name.Data());

        CREATE_ADD_GLASS(grad, ZImage, graddir, name, 0);
        grad->SetFile(GForm("gradients/%s", file.Data()));
      }
    }
    gSystem->FreeDirectory(dirh);
  }
  { // Meshes

    ZList* meshes = g_queen->AssertPath("var/meshes", "ZNameMap");

    // Terrain, setup mode specific.
    ASSIGN_ADD_GLASS(trimesh, TriMesh, meshes, "Terrain", 0);

    // Default extendios.
    ASSIGN_ADD_GLASS(stamesh, TriMesh, meshes, "Statico", 0);
    stamesh->MakeBox();
    stamesh->StdDynamicoPostImport();
    stamesh->SetMassFromBBox(1, 1, 1000);

    ASSIGN_ADD_GLASS(dynmesh, TriMesh, meshes, "Dynamico", 0);
    dynmesh->MakeTetrahedron();
    dynmesh->StdDynamicoPostImport();
    dynmesh->SetMassFromBBox(0.5, 0.33, 1000);

    ASSIGN_ADD_GLASS(flymesh, TriMesh, meshes, "Flyer", 0);
    flymesh->MakeTetraFlyer();
    flymesh->StdDynamicoPostImport();
    flymesh->SetMassFromBBox(0.15, 0.33, 1000);

    // Random statos.
    ASSIGN_ADD_GLASS(rndstatos, ZVector, meshes, "rndstatos", 0);
    rndstatos->Reserve(20);

    Float_t min_xy = 1, max_xy =  3;
    Float_t min_h  = 5, max_h  = 15;
    for (Int_t i=0; i<20; ++i)
    {
      Float_t a = g_rnd.Uniform(min_xy, max_xy),
        b = g_rnd.Uniform(min_xy, max_xy),
        c = g_rnd.Uniform(min_h,  max_h);
      CREATE_ADD_GLASS(m, TriMesh, rndstatos,
                       GForm("rndstato %d", i),
                       GForm("a=%.2f, b=%.2f, c=%.2f", a, b, c));

      m->MakeBox(a, b, c);
      m->StdDynamicoPostImport();
      m->SetVolume(a*b*c);
      m->SetXYArea(a*b);
      m->SetMassAndSpeculate(1000*m->GetVolume()); // rho = 1000 kg/m^3, say

      if (i == 0) {
        min_xy = 2, max_xy = 5;
        min_h  = 1, max_h  = 5;
      }
    }

    // Random dynos.
    ASSIGN_ADD_GLASS(rnddynos, ZVector, meshes, "rnddynos", 0);
    rnddynos->Reserve(20);

    // Float_t min_dyno_vol = TMath::Power(0.4, 3) * 0.5 * 0.5 / 6.0;
    // Float_t max_dyno_vol = 1.4 * 1.4 * 2.1      * 1.2 * 1.2 / 6.0;

    // printf("********* calculated dynamico volume min=%f, max=%f\n", min_dyno_vol, max_dyno_vol);

    for (Int_t i=0; i<20; ++i)
    {
      Float_t l1 = g_rnd.Uniform(0.4,    1.4),
        l2 = g_rnd.Uniform(0,      0.5*l1),
        w  = g_rnd.Uniform(0.5*l1, 1.2*l1),
        h  = g_rnd.Uniform(0.5*l1, 1.2*l1);

      CREATE_ADD_GLASS(m, TriMesh, rnddynos,
                       GForm("rndyno %d", i),
                       GForm("l1=%.2f, l2=%.2f, w=%.2f, h=%.2f", l1, l2, w, h));

      m->MakeTetrahedron(l1, l2, w, h);
      m->StdDynamicoPostImport();
      m->SetXYArea(0.5f * (l1 + l2) * w);
      m->SetVolume(m->GetXYArea() * h / 3.0f);
      m->SetMassAndSpeculate(1000*m->GetVolume()); // rho = 1000 kg/m^3, say
    }
  }


  /**************************************************************************/
  // Scene and scene objecs.
  /**************************************************************************/

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Tringula Scene", "Testing functionality of Var1::Tringula glass.");

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

  // CREATE_ADD_GLASS(base_plane, Rect, g_scene, "BasePlane", 0);
  // base_plane->SetUnitSquare(20);
  // base_plane->SetColor(0.6, 0.8, 0.6);

  ASSIGN_ADD_GLASS(terrain, RectTerrain, g_scene, "RectTerrain", 0);
  terrain->SetRnrSelf(false);
  terrain->SetDx(2);
  terrain->SetDy(2);
  terrain->SetMinCol(0.175, 0.689, 0.338);
  terrain->SetMaxCol(0.139, 0.384, 1.000);
  hf->SetFile("gforge.png");
  terrain->SetFromImage(hf, 30); // 5 for 32x32, 20 for 256x256 file "gforge.png"
  terrain->SetRnrMode(RectTerrain::RM_FlatTring);
  terrain->SetBorderCond(RectTerrain::BC_Wrap);
  terrain->ApplyBorderCondition();
  terrain->SetBorder(true);
  gforge->SetTerrain(terrain);


  // GTSurface, iso-maker, retriangulator.

  ASSIGN_ADD_GLASS(gtsurf, GTSurf, g_scene, "GTSsurf", 0);
  // gtsurf->SetFile("some-surface.gts");
  // gtsurf->Load();
  gtsurf->SetRnrSelf(false);
  ASSIGN_ADD_GLASS(gtsisomaker, GTSIsoMaker, g_scene, "GTSIsoMaker", 0);
  gtsisomaker->SetTarget(gtsurf);
  ASSIGN_ADD_GLASS(gtsretring,  GTSRetriangulator, g_scene, "GTSRetriangulator", 0);
  gtsretring->SetTarget(gtsurf);
  // Best coarsening
  gtsretring->SetStopNumber(800);
  gtsretring->SetCostOpts(GTSRetriangulator::CO_Volume);
  gtsretring->SetVO_ShapeWght(1e-8);
  gtsretring->SetMidvertOpts(GTSRetriangulator::MO_Volume);

  // Tringula
  CREATE_ADD_GLASS(light_mod, ZGlLightModel, g_scene, "Light Mod", 0);
  light_mod->SetFaceCullOp(ZRnrModBase::O_On);
  light_mod->SetFaceCullMode(GL_BACK);

  ASSIGN_ADD_GLASS(tringula, Tringula, g_scene, "Tringula 1", 0);
  tringula->SetPalette(pal);
  // tringula->SetRnrRay(true);
  tringula->SetDefStaMesh(stamesh);
  tringula->SetDefDynMesh(dynmesh);
  tringula->SetDefFlyMesh(flymesh);

  switch (mode)
  {
    case  0: setup_rectangle() ;     break;
    case  1: setup_triangle();       break;
    case  2: setup_sphere_outside(); break;
    case  3: setup_sphere_inside();  break;
    case  4: setup_torus_outside();  break;
    case  5: setup_torus_inside();   break;
    case 99: setup_test();           break;
  }

  if (mode == 99)
  {
    // Two intersecting dynos at the origin.
    Float_t oos2 = 1.0/TMath::Sqrt(2);
    HTransF trx; // !!!! was trans, with latest cint interferes with trans below
    trx.SetBaseVec(1,  oos2, oos2, 0);
    trx.SetBaseVec(2,  0, 0, 1);
    trx.SetBaseVecViaCross(3);
    dyn1 = tringula->NewDynamico("Dynus Primus");
    dyn1->ref_trans() = trx;
    dyn2 = tringula->NewDynamico("Dynus Secondus");
    dyn2->ref_trans() = trx;
    dyn2->ref_trans().Move3LF(0.2, 0.01, 0.05);
    dyn2->ref_trans().RotateLF(1, 2, 0.4);

    // Removed later, shields the dynos from random statos.
    sta1 = tringula->NewStatico("Statos Centrus");
    tmpdir->Add(sta1);

    ASSIGN_ADD_GLASS(mark, PSMark, tringula, "PSMark", 0);
    mark->SetParaSurf(parasurf);
  }

  // Camera base

  ASSIGN_ADD_GLASS(tricam, TringuCam, tringula, "TringuCam", 0);
  tricam->SetTringula(tringula);
  {
    ZTrans& trans = tricam->ref_trans();

    parasurf->origin_trans(trans);
    Float_t h = tringula->PlaceAboveTerrain(trans, 1, 1.1);
    trans.RotateLF(1, 2, TMath::PiOver2());
    printf("Camera placed:\n");
    trans.Print();

    tricam->SetHeight(h);
    tricam->StampReqTrans();
  }

  // Fields

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
  // Tringula is flat shaded by default, so we need triangle colors.
  // This should be made an option or global var.
  trimesh->GetTTvor()->GenerateTriangleColorsFromVertexColors();

  ASSIGN_ADD_GLASS(lightmap, TriMeshLightField, tricam, "TMLightField", 0);
  lightmap->SetMesh(trimesh);
  lightmap->ResizeToMesh();
  lightmap->SetLampPos(5, 5, 20);
  lightmap->SetPalette(pal);
  lightmap->CalculateLightField();


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

  printf("Initializing GUI "); fflush(stdout);

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
  {
    ZList* layouts = g_queen->AssertPath("var/layouts", "ZNameMap");
    fill_GledCore_layouts(layouts);
    layouts->Swallow("Var1", new ZGlass("TriMesh", trimesh_layout));

    g_nest->SetLayoutList(layouts);
  }

  ASSIGN_ATT_GLASS(g_pupil, PupilInfo, g_shell, AddSubShell, "Pupil of Tringula", 0);
  g_pupil->Add(g_scene);
  g_pupil->SetWidth(800); g_pupil->SetHeight(480);
  g_pupil->SetClearColor(0.589, 0.601, 0.836);
  g_pupil->SetUpReference(tricam);
  g_pupil->SetUpRefAxis(3);
  g_pupil->SetUpRefMinAngle(2);
  g_pupil->SetCameraBase(0);
  g_pupil->SetNearClip(0.2);
  g_pupil->SetFarClip(200);

  // g_pupil->SetBackMode(GL_FILL);
  // g_pupil->SetCHSize(0);

  g_pupil->SetShowRPS(false);
  g_pupil->SetShowView(false);

  g_pupil->SetAutoRedraw(false);

  g_shell->Add(tricam);
  g_pupil->SetEventHandler(tricam);
  g_pupil->SetCameraBase(tricam);

  tricam->SetPupilInfo(g_pupil);
  tricam->SetEventor(eventor);
  tricam->SetTimeMaker(tmaker);
  tmaker->AddClient(tricam);

  CREATE_ADD_GLASS(stxt1, ScreenText, g_pupil, "LftRgt", 0);
  tricam->SetInfoTxt(stxt1);

  /**************************************************************************/

  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");

  while (tricam->GetCamFix() == 0) {
    printf("."); fflush(stdout);
    gSystem->Sleep(50);
  }
  printf("\nGUI initialization complete.\n");

  Gled::theOne->LockFltk();

  dumper->SetPupil(g_pupil);
  // Look slightly down.
  tricam->GetCamFix()->RotateLF(3, 1, TMath::Pi()/8);

  Gled::theOne->UnlockFltk();

  make_overlay();
  g_pupil->SetOverlay(overlay);
  tricam->SetOverlay(overlay);
  tricam->SelectTopMenuByName("SimCtrl");
  tricam->SetMouseAction(TringuCam::MA_PickExtendios);

  /**************************************************************************/
  // Place some random statos, dynos and flyers
  /**************************************************************************/

  // Staticos
  {
    GTime timer(GTime::I_Now);
    Float_t desired_surface = statico_surface_fraction * tringula->GetParaSurf()->surface();
    Float_t used_surface    = 0;
    Int_t   stato_cnt       = 0;
    Int_t   exc_cnt         = 0;

    while (used_surface < desired_surface)
    {
      /*
      try
      {
        Statico* s = tringula->RandomStatico(rndstatos);
        used_surface += s->GetMesh()->GetXYArea();
        ++stato_cnt;
      }
      catch (Exc_t& exc)
      {
        printf("XYZZ %s\n", exc.Data());
        if (++exc_cnt > 20)
          break;
      }
      */

      Statico* s = tringula->RandomStatico(rndstatos);
      if (s != 0)
      {
        used_surface += s->GetMesh()->GetXYArea();
        ++stato_cnt;
      }
      else
      {
        if (++exc_cnt > 20) {
          printf("Statico placement failed %d-times. Moving on.\n", exc_cnt);
          break;
        }
      }
    }
    printf("RandomStaticos cnt=%d, exc_cnt=%d; desired_surface=%f, used_surface=%f; time=%fs\n",
           stato_cnt, exc_cnt, desired_surface, used_surface, timer.TimeUntilNow().ToDouble());
  }

  // Dynamicos
  for (int i=0; i<num_dynamico; ++i)
  {
    Dynamico* d = tringula->RandomDynamico(rnddynos, 0.5, 4, 0.25);
    Float_t v  = d->GetMesh()->GetVolume();
    Float_t vf = TMath::Max(1.0 - 3*v, 0.1); // volume ~ 0.01 -> 1
    // printf("dyno vol=%f, vf=%f\n", v, vf);
    d->SetV(vf*d->GetV());
    d->SetW(vf*d->GetW());
  }

  // Flyers
  for (int i=0; i<num_flyer; ++i) tringula->RandomFlyer(2, 12, 0.5, max_flyer_h);

  if (mode == 99)
  {
    tringula->GetStatos()->Remove(sta1);
  }

  /**************************************************************************/
  // And start the time ...
  /**************************************************************************/

  eventor->Start();
}


/******************************************************************************/
// Surface init
/******************************************************************************/

void setup_rectangle()
{
  // Make parasurf
  ASSIGN_GLASS(parasurf, PSRectangle, g_queen, "Rect ParaSurf", 0);
  PSRectangle* psrect = dynamic_cast<PSRectangle*>(parasurf);
  psrect->SetMinX(terrain->GetMinX());
  psrect->SetMaxX(terrain->GetMaxX());
  psrect->SetMinY(terrain->GetMinY());
  psrect->SetMaxY(terrain->GetMaxY());
  parasurf->SetupEdgePlanes();

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportRectTerrain(terrain);
  trimesh->StdSurfacePostImport();

  // Export to GTS surface.
  trimesh->ExportGTSurf(gtsurf);

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetMesh(trimesh);
}

void setup_triangle()
{
  Float_t base = 128;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSTriangle, g_queen, "Tri ParaSurf", 0);
  PSTriangle* pstri = dynamic_cast<PSTriangle*>(parasurf);
  pstri->Scale(base);
  parasurf->SetupEdgePlanes();

  // Setup GTS surface.
  gtsurf->GenerateTriangle(base);
  gtsurf->Tessellate(6);

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  parasurf->RandomizeH(trimesh, 4, 1, 0.8, 0, 1.1);
  trimesh->StdSurfacePostImport();
  trimesh->GenerateVertexNormals();
  trimesh->GenerateTriangleNormals();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetMesh(trimesh);
}

void setup_sphere_outside()
{
  Float_t R = 32;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSSphere, g_queen, "Sph ParaSurf", 0);
  PSSphere* pssph = dynamic_cast<PSSphere*>(parasurf);
  pssph->SetR(R);

  // Setup GTS surface.
  gtsurf->GenerateSphere(4);
  gtsurf->Rescale(R);
  for (int i=0; i<5; ++i)
    gtsurf->Legendrofy(16, 0.25, 1.5);

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  trimesh->StdSurfacePostImport();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);
}

void setup_sphere_inside()
{
  Float_t R = 32;

  max_flyer_h = 10;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSSphere, g_queen, "Sph ParaSurf", 0);
  PSSphere* pssph = dynamic_cast<PSSphere*>(parasurf);
  pssph->SetInside(true);
  pssph->SetR(R);

  // Setup GTS surface.
  gtsurf->GenerateSphere(4);
  gtsurf->Rescale(R);
  for (int i=0; i<5; ++i)
    gtsurf->Legendrofy(16, 0.25, 1.5);
  gtsurf->Invert();

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  trimesh->StdSurfacePostImport();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);
}

float tweak(TRandom& rnd, float x)
{
  return x + rnd.Uniform(0.001, 0.002);
}

void setup_torus_outside()
{
  Float_t rM = 30, rm = 15, rS = rM + rm;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSTorus, g_queen, "Tor ParaSurf", 0);
  PSTorus* pstor = dynamic_cast<PSTorus*>(parasurf);
  pstor->SetRM(rM);
  pstor->SetRm(rm);

  // Setup GTS surface.
  gtsisomaker->SetFormula(Form("(%f - sqrt(x^2 + y^2))^2 + z^2", rM));
  gtsisomaker->SetValue(rm*rm);

  Int_t ndiv = 25, ndivz = 8; // TMath::Nint(ndiv*rS/rm);
  gtsisomaker->SetXAxis(-tweak(g_rnd, rS), tweak(g_rnd, rS), ndiv);
  gtsisomaker->SetYAxis(-tweak(g_rnd, rS), tweak(g_rnd, rS), ndiv);
  gtsisomaker->SetZAxis(-tweak(g_rnd, rm), tweak(g_rnd, rm), ndivz);
  printf("making surface ...\n");
  gtsisomaker->MakeSurface();
  printf("coarsening ...\n");
  gtsretring->SetStopNumber(2000); gtsretring->Coarsen();
  printf("done\n");

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  pstor->RandomizeH(trimesh, 20, 2, 0.7);
  trimesh->StdSurfacePostImport();
  trimesh->GenerateVertexNormals();
  trimesh->GenerateTriangleNormals();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);
}

void setup_torus_inside()
{
  Float_t rM = 30, rm = 15, rS = rM + rm;

  max_flyer_h = 10;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSTorus, g_queen, "Tor ParaSurf", 0);
  PSTorus* pstor = dynamic_cast<PSTorus*>(parasurf);
  pstor->SetInside(true);
  pstor->SetRM(rM);
  pstor->SetRm(rm);

  // Setup GTS surface.
  gtsisomaker->SetFormula(Form("(%f - sqrt(x^2 + y^2))^2 + z^2", rM));
  gtsisomaker->SetValue(rm*rm);

  Int_t ndiv = 25, ndivz = 8; // TMath::Nint(ndiv*rS/rm);
  gtsisomaker->SetXAxis(-tweak(g_rnd, rS), tweak(g_rnd, rS), ndiv);
  gtsisomaker->SetYAxis(-tweak(g_rnd, rS), tweak(g_rnd, rS), ndiv);
  gtsisomaker->SetZAxis(-tweak(g_rnd, rm), tweak(g_rnd, rm), ndivz);
  printf("making surface ...\n");
  gtsisomaker->MakeSurface();
  printf("coarsening ...\n");
  gtsretring->SetStopNumber(2000); gtsretring->Coarsen();
  printf("done\n");
  gtsurf->Invert();

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  pstor->RandomizeH(trimesh, 20, 1, 0.8);
  trimesh->StdSurfacePostImport();
  trimesh->GenerateVertexNormals();
  trimesh->GenerateTriangleNormals();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);
}

void setup_test()
{
  // A mini-version of triangle.

  // Override global settings.
  statico_surface_fraction = 0.05;
  num_dynamico = 30;
  num_flyer    = 15;
  max_flyer_h  = 5;

  Float_t base = 32;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSTriangle, g_queen, "Tri ParaSurf", 0);
  PSTriangle* pstri = dynamic_cast<PSTriangle*>(parasurf);
  pstri->Scale(base);
  parasurf->SetupEdgePlanes();

  // Setup GTS surface.
  gtsurf->GenerateTriangle(base);
  gtsurf->Tessellate(4);

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  parasurf->RandomizeH(trimesh, 4, 1, 0.8, 0, 1.1);
  trimesh->StdSurfacePostImport();
  trimesh->GenerateVertexNormals();
  trimesh->GenerateTriangleNormals();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetMesh(trimesh);
}


/******************************************************************************/
// Overlay GUI
/******************************************************************************/

void make_overlay()
{
  Float_t weed_dx = 130, step_dx = 140, step_dy = 26;

  ASSIGN_ADD_GLASS(overlay, Scene, g_queen, "Overlay", 0);

  CREATE_ADD_GLASS(ovl_lm, ZGlLightModel, overlay, "LightOff", 0);
  ovl_lm->SetLightModelOp(0);

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

  {
    CREATE_ADD_GLASS(sim_ctrl, WGlWidget, overlay, "SimCtrl", 0);
    sim_ctrl->SetRnrElements(false);
    //sim_ctrl->SetRnrMod(menufs);
    //dd->SetMenuFrameStyle(menufs);

    CREATE_ADD_GLASS(but1, WGlButton, sim_ctrl, "Suspend", 0);
    gs.SetNodeAdvance(but1);
    but1->SetCbackAlpha(tricam);
    but1->SetCbackMethodName("Suspend");

    CREATE_ADD_GLASS(but2, WGlButton, sim_ctrl, "Resume", 0);
    gs.SetNodeAdvance(but2);
    but2->SetCbackAlpha(tricam);
    but2->SetCbackMethodName("Resume");

    CREATE_ADD_GLASS(val1, WGlValuator, sim_ctrl, "Sleep", 0);
    gs.SetNodeAdvance(val1);
    val1->SetMin(1); val1->SetMax(1000);
    val1->SetFormat("Sleep: %.0f");
    val1->SetCbackAlpha(eventor);
    val1->SetCbackMemberName("InterBeatMS");
  }

  gs.Reset();

  {
    CREATE_ADD_GLASS(dyno_ctrl, WGlWidget, overlay, "DynoCtrl", 0);
    dyno_ctrl->SetRnrElements(false);

    CREATE_ADD_GLASS(but1, WGlButton, dyno_ctrl, "<undef>", "LensName:LensBeta");
    gs.SetNodeAdvance(but1);
    but1->SetCbackAlpha(tricam);
    but1->SetCbackMethodName("DynoDetails");

    CREATE_ADD_GLASS(val1, WGlValuator, dyno_ctrl, "Speed", "LensAlpha");
    gs.SetNodeAdvance(val1);
    val1->SetMin(-10); val1->SetMax(100); val1->SetStepB(100);
    val1->SetFormat("Speed: %.2f");
    val1->SetCbackMemberName("V");

    CREATE_ADD_GLASS(val2, WGlValuator, dyno_ctrl, "Rot", "LensAlpha");
    gs.SetNodeAdvance(val2);
    val2->SetMin(-10); val2->SetMax(100); val2->SetStepB(100);
    val2->SetFormat("Rot: %.2f");
    val2->SetCbackMemberName("W");

    CREATE_ADD_GLASS(val3, WGlValuator, dyno_ctrl, "LevH", "LensAlpha");
    gs.SetNodeAdvance(val3);
    val3->SetMin(-10); val3->SetMax(100); val3->SetStepB(100);
    val3->SetFormat("LevH: %.2f");
    val3->SetCbackMemberName("LevH");
  }

  gs.Reset();

  {
    CREATE_ADD_GLASS(stato_ctrl, WGlWidget, overlay, "StatoCtrl", 0);
    stato_ctrl->SetRnrElements(false);

    CREATE_ADD_GLASS(but1, WGlButton, stato_ctrl, "<undef>", "LensName:LensBeta");
    gs.SetNodeAdvance(but1);
    but1->SetCbackAlpha(tricam);
    but1->SetCbackMethodName("StatoDetails");

    CREATE_ADD_GLASS(val1, WGlValuator, stato_ctrl, "CollCount", "LensAlpha");
    gs.SetNodeAdvance(val1);
    val1->SetConstVal(true);
    val1->SetFormat("Dyno colls: %.0f");
    val1->SetCbackMemberName("NDynoColls");

    CREATE_ADD_GLASS(but2, WGlButton, stato_ctrl, "Connect Energy", "LensBeta");
    gs.SetNodeAdvance(but2);
    but2->SetCbackAlpha(tricam);
    but2->SetCbackMethodName("PrepConnectStatosEnergy");

    CREATE_ADD_GLASS(but3, WGlButton, stato_ctrl, "Connec Matter", "LensBeta");
    gs.SetNodeAdvance(but3);
    but3->SetCbackAlpha(tricam);
    but3->SetCbackMethodName("PrepConnectStatosMatter");
  }
}


/******************************************************************************/
// Various
/******************************************************************************/

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
