#include <glass_defines.h>
#include <gl_defines.h>

// #include <exception>

class TSPupilInfo;
class Tringula;
class TringuCam;
class TringuRep;
class Statico;
class Dynamico;
class Crawler;
class ParaSurf;
class GravPlotter;
class PSMark;
class TriMesh;
class TriMeshField;
class TriMeshLightField;
class RectTerrain;
class GTSurf;
class GTSRetriangulator;
class GTSIsoMaker;
class TimeMaker;

Tringula     *tringula = 0;
TringuCam    *tricam   = 0;
TringuRep    *trirep   = 0;
Statico      *sta1     = 0;
Crawler      *dyn1     = 0;
Crawler      *dyn2     = 0;
PSMark       *mark     = 0;

ParaSurf     *parasurf  = 0;
TriMesh      *trimesh   = 0;
TriMesh      *dynmesh   = 0;
TriMesh      *stamesh   = 0;
TriMesh      *flymesh   = 0;
TriMesh      *chopmesh  = 0;
ZVector      *rndstatos  = 0;
ZVector      *rnddynos  = 0;

TSPupilInfo  *tspupil  = 0;

TriMeshField      *engfield    = 0;
TriMeshField      *metfield    = 0;
RectTerrain       *terrain     = 0;
GTSurf            *gtsurf      = 0;
GTSRetriangulator *gtsretring  = 0;
GTSIsoMaker       *gtsisomaker = 0;

TriMeshLightField *lightmap = 0;

Eventor      *eventor  = 0;
TimeMaker    *tmaker   = 0;
ScreenDumper *dumper   = 0;

ZList        *tmpdir   = 0;

// Some of the parameters are modified for some modes.

Float_t      statico_surface_fraction  = 0.05; // 0.05
Int_t        num_dynamico              = 250;  // 250
Int_t        num_flyer                 = 70;   // 70
Int_t        num_chopper               = 30;   // 30

const Text_t* trimesh_layout = "ZGlass(Name,Title[22]):TriMesh(M[8],Surf[8],COM,J)";

void tringula(Int_t mode=2)
{
  gRandom = new TRandom3(0); // Seed 0 means seed with TUUID.

  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  g_queen->SetName("TringulaQueen");
  g_queen->SetTitle("Demo of Tringula");

  // AlContext -- need it to load the sounds.
  CREATE_ADD_GLASS(alctx, AlContext, g_queen, "AlContext", 0);
  alctx->Open();

  /**************************************************************************/
  // Database objects.
  /**************************************************************************/

  tmpdir = g_queen->AssertPath("tmp", "ZNameMap");

  { // Rnr-mods
    ZList* moddir = g_queen->AssertPath("var/rnrmods", "ZNameMap");
    // Lights-off
    CREATE_ADD_GLASS(loff, ZGlLightModel, moddir, "LightsOff", 0);
    loff->SetLightModelOp(0);
  }

  { // AL buffers
    ZList* bufdir = g_queen->AssertPath("var/sounds", "ZNameMap");

    CREATE_ADD_GLASS(albuf1, AlBuffer, bufdir, "Diesel", 0);
    // albuf1->SetFile("cuckoo.ogg");
    albuf1->SetFile("../Audio1/diesel-loop.ogg");
    albuf1->Load();

    CREATE_ADD_GLASS(albuf2, AlBuffer, bufdir, "PewPew", 0);
    albuf2->SetFile("../Audio1/boom.ogg");
    albuf2->Load();

    CREATE_ADD_GLASS(albuf3, AlBuffer, bufdir, "BigExplosion", 0);
    albuf3->SetFile("sounds/big_explosion.ogg");
    albuf3->Load();
  }

  { // Glass mappings
    ZList *uidir = g_queen->AssertPath("var/glassui", "ZNameMap");
    ZList *gdir;

    {
      gdir = uidir->AssertPath("Crawler");
      CREATE_ADD_GLASS(spiritio, CrawlerSpiritio, gdir, "spiritio", 0);

      CREATE_ATT_GLASS(alsource, AlSource, spiritio, SetEngineSrc, "Engine of Crawler Spiritio", 0);
      alsource->SetLocationType(AlSource::LT_CamDelta);
      alsource->SetPitch(0.5);
      alsource->SetDefaultBuffer((AlBuffer*)g_queen->FindLensByPath("var/sounds/Diesel"));
      alsource->QueueBuffer();

      CREATE_ATT_GLASS(alsource, AlSource, spiritio, SetGunSrc, "Gun of Crawler Spiritio", 0);
      alsource->SetLocationType(AlSource::LT_CamDelta);
      alsource->SetDefaultBuffer((AlBuffer*)g_queen->FindLensByPath("var/sounds/PewPew"));
      alsource->QueueBuffer();
    }
  }

  { // Textures

    ZList* texdir = g_queen->AssertPath("var/textures", "ZNameMap");
    // Height-field
    CREATE_ADD_GLASS(hf, ZImage, texdir, "HeightField", 0);
    // GForger
    CREATE_ADD_GLASS(gforge, GForger, texdir, "GForger", 0);
    gforge->SetImage(hf);
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
    TPMERegexp re("lin_([\\w\\d]+)\\.\\w+");
    while ((file = gSystem->GetDirEntry(dirh)).IsNull() == kFALSE)
    {
      Int_t nm = re.Match(file);
      if (nm == 2) {
        CREATE_ADD_GLASS(grad, ZImage, graddir, re[1], 0);
        grad->SetFile(GForm("gradients/%s", file.Data()));
      } else {
        // printf("Load gradients: error matching file='%s', nmatch=%d.\n", file.Data(), nm);
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

    ASSIGN_ADD_GLASS(chopmesh, TriMesh, meshes, "Chopper", 0);
    chopmesh->MakeTetraChopper();
    chopmesh->StdDynamicoPostImport();
    chopmesh->SetMassFromBBox(0.15, 0.33, 1000);

    ASSIGN_ADD_GLASS(landmarkmesh, TriMesh, meshes, "LandMark", 0);
    // landmarkmesh->MakeTetraMark();
    landmarkmesh->MakeTetraMark(0.5, 1, 1, 1, 0.05);

    landmarkmesh->StdDynamicoPostImport();
    landmarkmesh->SetMassFromBBox(0.15, 0.33, 1000);

    // Random statos.
    ASSIGN_ADD_GLASS(rndstatos, ZVector, meshes, "rndstatos", 0);
    rndstatos->Reserve(20);

    Float_t min_xy = 1, max_xy =  3;
    Float_t min_h  = 5, max_h  = 15;
    for (Int_t i=0; i<20; ++i)
    {
      Float_t a = gRandom->Uniform(min_xy, max_xy),
        b = gRandom->Uniform(min_xy, max_xy),
        c = gRandom->Uniform(min_h,  max_h);
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
      Float_t l1 = gRandom->Uniform(0.4,    1.4),
        l2 = gRandom->Uniform(0,      0.5*l1),
        w  = gRandom->Uniform(0.5*l1, 1.2*l1),
        h  = gRandom->Uniform(0.5*l1, 1.2*l1);

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

  CREATE_ADD_GLASS(allis, AlListener, g_scene, "AlListener", 0);

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

  ASSIGN_ADD_GLASS(tringula, Tringula, g_queen, "Tringula 1", 0);
  // tringula->SetRnrRay(true);
  tringula->SetDefStaMesh(stamesh);
  tringula->SetDefDynMesh(dynmesh);
  tringula->SetDefFlyMesh(flymesh);
  tringula->SetDefChopMesh(chopmesh);

  switch (mode)
  {
    case  0: setup_rectangle() ;     break;
    case  1: setup_triangle();       break;
    case  2: setup_sphere_outside(); break;
    case  3: setup_sphere_inside();  break;
    case  4: setup_torus_outside();  break;
    case  5: setup_torus_inside();   break;
    case  9: setup_large_sphere();   break;
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
    dyn1 = new Crawler("Dynus Primus");
    tringula->RegisterCrawler(dyn1);
    dyn1->ref_trans() = trx;
    dyn2 = tringula->NewDynamico("Dynus Secondus");
    dyn2->ref_trans() = trx;
    dyn2->ref_trans().Move3LF(0.2, 0.01, 0.05);
    dyn2->ref_trans().RotateLF(1, 2, 0.4);

    // Removed later, shields two test-dynos from random statos.
    sta1 = tringula->NewStatico("Statos Centrus");
    tmpdir->Add(sta1);

    ASSIGN_ADD_GLASS(mark, PSMark, tringula, "PSMark", 0);
    mark->SetParaSurf(parasurf);
  }

  // TringuRep

  ASSIGN_ADD_GLASS(trirep, TringuRep, g_scene, "TringuRep", 0);
  trirep->SetTringula(tringula);
  trirep->SetPalette(pal);

  { // Fields. Should go somewhere else as they are tringula specific.
    // But then also need to differentiate true fields and field known
    // to a player (measured fields).
    ZList* meshes = g_queen->AssertPath("var/fields", "ZNameMap");

    ASSIGN_ADD_GLASS(engfield, TriMeshField, meshes, "Energy Field", 0);
    engfield->SetMesh(trimesh);
    engfield->ResizeToMesh();
    engfield->SetPalette(pal);
    engfield->FillByGaussBlobs();
    engfield->FindMinMaxField();

    ASSIGN_ADD_GLASS(metfield, TriMeshField, meshes, "Metal Field", 0);
    metfield->SetMesh(trimesh);
    metfield->ResizeToMesh();
    metfield->SetPalette(pal);
    metfield->FillByGaussBlobs();
    metfield->FindMinMaxField();

    ASSIGN_ADD_GLASS(lightmap, TriMeshLightField, meshes, "LightMap Field", 0);
    lightmap->SetMesh(trimesh);
    lightmap->ResizeToMesh();
    lightmap->SetPalette(pal);
    lightmap->SetLampPos(5, 5, 5);
    lightmap->SetDirectional(true);
    lightmap->CalculateLightField();
    lightmap->Diffuse(); lightmap->Diffuse();
    lightmap->Diffuse(); lightmap->Diffuse();
  }

  // This has to be called after tringula is fully initialized.
  trirep->ActivateField(engfield);
  // trirep->ActivateLightField(lightmap);

  // TringuCam base

  ASSIGN_ADD_GLASS(tricam, TringuCam, trirep, "TringuCam", 0);
  tricam->SetTringula(tringula);
  tricam->SetTringuRep(trirep);
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


  /**************************************************************************/
  // Eventor & Operators
  /**************************************************************************/

  ASSIGN_ADD_GLASS(eventor, Eventor, g_queen, "Eventor", 0);
  eventor->SetInterBeatMS(10);

  ASSIGN_ADD_GLASS(tmaker, TimeMaker, eventor, "TimeMaker", 0);

  ASSIGN_ADD_GLASS(dumper, ScreenDumper, eventor, "ScreenDumper", 0);
  dumper->SetWaitSignal(true);


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

  ASSIGN_ATT_GLASS(tspupil, TSPupilInfo, g_shell, AddSubShell, "Pupil of Tringula", 0);
  g_pupil = tspupil;
  g_pupil->Add(g_scene);
  g_pupil->SetWidth(1024); g_pupil->SetHeight(768);
  g_pupil->SetClearColor(0.589, 0.601, 0.836);
  // g_pupil->SetUpReference(tricam);
  // g_pupil->SetUpRefAxis(3);
  // g_pupil->SetUpRefMinAngle(2);
  g_pupil->SetCameraBase(0);
  g_pupil->SetNearClip(0.2);
  g_pupil->SetFarClip(200);
  g_pupil->SetMinZFov(5);
  g_pupil->SetMaxZFov(145);

  // g_pupil->SetBackMode(GL_FILL);

  g_pupil->SetCHSize(0);
  g_pupil->SetShowRPS(false);
  g_pupil->SetShowView(false);

  g_pupil->SetAutoRedraw(false);

  g_shell->Add(tricam);
  g_shell->Add(trirep);

  CREATE_ADD_GLASS(tringu_tester, TringulaTester, trirep, "TringulaTester", 0);
  tringu_tester->SetTringula(tringula);

  CREATE_ATT_GLASS(tricam_spiritio, TringuObserverSpiritio, tspupil, SetDefaultSpiritio,
		   "TringuObserverSpiritio", "Default Spiritio");
  tricam_spiritio->GetCamera()->RotateLF(3, 1, TMath::Pi()/8);
  tricam_spiritio->SetTringuCam(tricam);

  tspupil->SetTimeMaker(tmaker);
  tspupil->SetTringuRep(trirep);
  trirep->SetPupilInfo(tspupil);

  tricam->SetPupilInfo(tspupil);
  tricam->SetEventor(eventor);
  tricam->SetTimeMaker(tmaker);

  CREATE_ADD_GLASS(stxt1, ScreenText, g_pupil, "LftRgt", 0);
  tricam->SetInfoTxt(stxt1);

  /**************************************************************************/

  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");

  while (tspupil->GetRnrCamFix() == 0) {
    printf("."); fflush(stdout);
    gSystem->Sleep(50);
  }
  printf("\nGUI initialization complete.\n");

  Gled::theOne->LockFltk();

  dumper->SetPupil(g_pupil);

  Gled::theOne->UnlockFltk();

  setup_tspupil();
  make_overlay();

  tspupil->SelectTopMenuByName("MainMenu");

  tricam->SetMouseAction(TringuCam::MA_PickExtendios);

  /**************************************************************************/
  // Place some random statos, dynos and flyers
  /**************************************************************************/

  // Staticos
  {
    GTime timer(GTime::I_Now);
    Float_t desired_surface = statico_surface_fraction * tringula->GetParaSurf()->Surface();
    Float_t used_surface    = 0;
    Int_t   stato_cnt       = 0;
    Int_t   exc_cnt         = 0;

    while (used_surface < desired_surface)
    {
      // This does not work well ... the exception never reaches
      // the catch.
      // Need to investigate with Axel.
      /*
	try
	{
        Statico* s = tringula->RandomStatico(rndstatos);
	if (s == 0) printf("SAGR!!!!\n");
        used_surface += s->GetMesh()->GetXYArea();
        ++stato_cnt;
	}
	catch (Exc_t& exc)
	{
        printf("XYZZ %s\n", exc.Data());
        if (++exc_cnt > 20) {
	printf("Statico placement failed %d-times. Moving on.\n", exc_cnt);
	break;
	}
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
  for (int i=0; i<num_flyer; ++i) tringula->RandomAirplane(6, 12, 0.5);

  // Choppers
  for (int i=0; i<num_chopper; ++i) tringula->RandomChopper(1, 8, 0.8);

  if (mode == 99)
  {
    tringula->GetStatos()->Remove(sta1);
  }

  /**************************************************************************/
  // Populate the time-maker and start the time ...
  /**************************************************************************/

  // This will install the default spiritio.
  tspupil->SetCurrentSpiritio(0);

  tmaker->AddClient(tringula);
  tmaker->AddClient(tricam);
  tmaker->AddClient(tspupil);

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
  trimesh->ImportRectTerrain(terrain, false);
  trimesh->StdSurfacePostImport();

  // Export to GTS surface.
  trimesh->ExportGTSurf(gtsurf);

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetEdgeRule(Tringula::ER_Bounce);
  tringula->SetMesh(trimesh);

  tringula->SetMaxFlyerH (0.3 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(parasurf->CharacteristicLength());
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

  tringula->SetMaxFlyerH (0.3 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(parasurf->CharacteristicLength());
}

void setup_sphere_outside()
{
  Float_t R = 32;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSSphere, g_queen, "Sph ParaSurf", 0);
  PSSphere* pssph = dynamic_cast<PSSphere*>(parasurf);
  pssph->SetR(R);

  // Setup GTS surface.
  // gtsurf->GenerateSphere(5);
  gtsurf->GenerateSphere(7);
  gtsurf->Rescale(R);
  for (int i=0; i<5; ++i) {
    //gtsurf->Legendrofy(16, 0.25, 1.5);
    gtsurf->Legendrofy(24, 0.25, 1.25);
  }

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  trimesh->StdSurfacePostImport();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);

  tringula->SetMaxFlyerH (0.5 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(2 * parasurf->CharacteristicLength());
}

void setup_sphere_inside()
{
  Float_t R = 32;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSSphere, g_queen, "Sph ParaSurf", 0);
  PSSphere* pssph = dynamic_cast<PSSphere*>(parasurf);
  pssph->SetInside(true);
  pssph->SetR(R);

  // Setup GTS surface.
  gtsurf->GenerateSphere(4);
  gtsurf->Rescale(R);
  for (int i=0; i<5; ++i) {
    // gtsurf->Legendrofy(16, 0.25, 1.5);
    gtsurf->Legendrofy(24, 0.25, 1.25);
  }
  gtsurf->Invert();

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  trimesh->StdSurfacePostImport();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);

  tringula->SetMaxFlyerH (0.5 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(0.9 * parasurf->CharacteristicLength());
}

float tweak(TRandom* rnd, float x)
{
  return x + rnd->Uniform(0.001, 0.002);
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

  Int_t ndiv = 50, ndivz = 16; // TMath::Nint(ndiv*rS/rm);
  gtsisomaker->SetXAxis(-tweak(gRandom, rS), tweak(gRandom, rS), ndiv);
  gtsisomaker->SetYAxis(-tweak(gRandom, rS), tweak(gRandom, rS), ndiv);
  gtsisomaker->SetZAxis(-tweak(gRandom, rm), tweak(gRandom, rm), ndivz);
  printf("making surface ...\n");
  gtsisomaker->MakeSurface();
  printf("coarsening ...\n");
  gtsretring->SetStopNumber(10000); gtsretring->Coarsen();
  printf("done\n");

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  pstor->RandomizeH(trimesh, 20, 2, 0.5);
  trimesh->StdSurfacePostImport();
  trimesh->GenerateVertexNormals();
  trimesh->GenerateTriangleNormals();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);

  tringula->SetMaxFlyerH (0.5 * (rM - rm));
  tringula->SetMaxCameraH(0.9 * (rM - rm));
}

void setup_torus_inside()
{
  Float_t rM = 30, rm = 15, rS = rM + rm;

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
  gtsisomaker->SetXAxis(-tweak(gRandom, rS), tweak(gRandom, rS), ndiv);
  gtsisomaker->SetYAxis(-tweak(gRandom, rS), tweak(gRandom, rS), ndiv);
  gtsisomaker->SetZAxis(-tweak(gRandom, rm), tweak(gRandom, rm), ndivz);
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

  tringula->SetMaxFlyerH (0.5 * rm);
  tringula->SetMaxCameraH(0.9 * rm);
}

void setup_large_sphere()
{
  Float_t R = 100;
  statico_surface_fraction = 0.01;

  // Create parasurf
  ASSIGN_GLASS(parasurf, PSSphere, g_queen, "Sph ParaSurf", 0);
  PSSphere* pssph = dynamic_cast<PSSphere*>(parasurf);
  pssph->SetR(R);

  // Setup GTS surface.
  // gtsurf->GenerateSphere(4);
  gtsurf->GenerateSphere(6);
  gtsurf->Rescale(R);
  for (int i=0; i<5; ++i) {
    //gtsurf->Legendrofy(16, 0.25, 1.5);
    gtsurf->Legendrofy(24, 0.25, 1.25);
  }

  // Setup trimesh
  trimesh->SetParaSurf(parasurf);
  trimesh->ImportGTSurf(gtsurf);
  trimesh->StdSurfacePostImport();

  // Setup tringula
  tringula->SetParaSurf(parasurf);
  tringula->SetMesh(trimesh);

  tringula->SetMaxFlyerH (0.5 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(2 * parasurf->CharacteristicLength());
}

void setup_test()
{
  // A mini-version of triangle.

  // Override global settings.
  statico_surface_fraction = 0.1;
  num_dynamico = 30;
  num_flyer    = 15;
  num_chopper  =  7;

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

  tringula->SetMaxFlyerH (0.4 * parasurf->CharacteristicLength());
  tringula->SetMaxCameraH(parasurf->CharacteristicLength());
}


/******************************************************************************/
// TSPupil setup
/******************************************************************************/

void setup_tspupil()
{
  Float_t weed_dx = 130;

  Scene *menu_scene = tspupil->GetMenuScene();

  menu_scene->AddRnrMod(g_queen->FindLensByPath("var/rnrmods/LightsOff"));

  CREATE_ATT_GLASS(pp, ZGlPerspective, menu_scene, AddRnrMod, "Perspective - Ortho Pixel", 0);
  pp->StandardPixel();

  CREATE_ATT_GLASS(bfs, WGlFrameStyle, menu_scene, AddRnrMod, "Butt Frame Style", 0);
  bfs->StandardPixel();
  bfs->SetDefDx(weed_dx);
  bfs->SetTileColor(0.5, 0.15, 0.25, 0.7);
  bfs->SetBelowMColor(0.3, 0.08, 0.14, 0.9);

  // ----------------------------------------------------------------

  Scene *spiritio_scene = tspupil->GetSpiritioScene();

  spiritio_scene->AddRnrMod(g_queen->FindLensByPath("var/rnrmods/LightsOff"));

  CREATE_ATT_GLASS(bb, ZGlBlending, spiritio_scene, AddRnrMod, "Blending", 0);
  bb->SetBlendOp(ZRnrModBase::O_On);

  CREATE_ATT_GLASS(pp, ZGlPerspective, spiritio_scene, AddRnrMod, "Perspective - Ortho True Aspect", 0);
  pp->StandardTrueAspect();
  pp->SetOrthoW(1); pp->SetOrthoH(1);
}


/******************************************************************************/
// Overlay GUI
/******************************************************************************/

void make_overlay()
{
  Float_t step_dx = 140, step_dy = 26;
  
  Scene *menu_scene = tspupil->GetMenuScene();

  SGridStepper& gs = tspupil->RefGridStepper();
  gs.SetDs(step_dx, step_dy, 1);
  gs.SetNs(6, 4, 1);
  gs.SetOs(4, 4, 0);

  {
    CREATE_ADD_GLASS(main_menu, WGlWidget, menu_scene, "MainMenu", 0);

    CREATE_ADD_GLASS(sim_ctrl, WGlButton, main_menu, "SimCtrl", 0);
    gs.SetNode(sim_ctrl);
    sim_ctrl->SetCbackAlpha(sim_ctrl);
    sim_ctrl->SetCbackMethodName("MenuEnter");

    sim_ctrl->SetRnrElements(false);
    {
      SGridStepper step(gs, true);

      CREATE_ADD_GLASS(but1, WGlButton, sim_ctrl, "Suspend", 0);
      step.SetNodeAdvance(but1);
      but1->SetCbackAlpha(tricam);
      but1->SetCbackMethodName("Suspend");

      CREATE_ADD_GLASS(but2, WGlButton, sim_ctrl, "Resume", 0);
      step.SetNodeAdvance(but2);
      but2->SetCbackAlpha(tricam);
      but2->SetCbackMethodName("Resume");

      CREATE_ADD_GLASS(val1, WGlValuator, sim_ctrl, "Sleep", 0);
      step.SetNodeAdvance(val1);
      val1->SetMin(1); val1->SetMax(1000);
      val1->SetFormat("Sleep: %.0f");
      val1->SetCbackAlpha(eventor);
      val1->SetCbackMemberName("InterBeatMS");

      CREATE_ADD_GLASS(back, WGlButton, sim_ctrl, " << ", 0);
      step.SetNodeAdvance(back);
      back->SetCbackAlpha(sim_ctrl);
      back->SetCbackMethodName("MenuExit");

      CREATE_ADD_GLASS(exit_but, WGlButton, sim_ctrl, "Exit", 0);
      step.SetNode(exit_but);
      exit_but->MoveLF(1, 40);
      exit_but->SetCbackAlpha(exit_but);
      exit_but->SetCbackMethodName("ExitGled");

    }
    gs.Step();

    CREATE_ADD_GLASS(ter_ctrl, WGlButton, main_menu, "TerrainCtrl", 0);
    gs.SetNode(ter_ctrl);
    ter_ctrl->SetCbackAlpha(ter_ctrl);
    ter_ctrl->SetCbackMethodName("MenuEnter");

    ter_ctrl->SetRnrElements(false);
    {
      SGridStepper step(gs, true);

      CREATE_ADD_GLASS(but1, WGlButton, ter_ctrl, "Height", 0);
      step.SetNodeAdvance(but1);
      but1->SetCbackAlpha(trirep);
      but1->SetCbackMethodName("ColorByTerrainProps");
      but1->SetCbackValue(0);

      CREATE_ADD_GLASS(but2, WGlButton, ter_ctrl, "Flatness", 0);
      step.SetNodeAdvance(but2);
      but2->SetCbackAlpha(trirep);
      but2->SetCbackMethodName("ColorByTerrainProps");
      but2->SetCbackValue(1);

      CREATE_ADD_GLASS(but3, WGlButton, ter_ctrl, "Energy Field", 0);
      step.SetNodeAdvance(but3);
      but3->SetCbackAlpha(trirep);
      but3->SetCbackMethodName("ActivateField");
      but3->SetCbackBeta(engfield);

      CREATE_ADD_GLASS(but4, WGlButton, ter_ctrl, "Metal Field", 0);
      step.SetNodeAdvance(but4);
      but4->SetCbackAlpha(trirep);
      but4->SetCbackMethodName("ActivateField");
      but4->SetCbackBeta(metfield);

      CREATE_ADD_GLASS(but5, WGlButton, ter_ctrl, "Flip Shadows", 0);
      step.SetNodeAdvance(but5);
      but5->SetCbackAlpha(trirep);
      but5->SetCbackMethodName("SwitchLightField");
      but5->SetCbackBeta(lightmap);

      CREATE_ADD_GLASS(but6, WGlButton, ter_ctrl, "Smooth/Flat", 0);
      step.SetNodeAdvance(but6);
      but6->SetCbackAlpha(trirep);
      but6->SetCbackMethodName("SwitchSmoothShading");

      CREATE_ADD_GLASS(back, WGlButton, ter_ctrl, " << ", 0);
      step.SetNodeAdvance(back);
      back->SetCbackAlpha(ter_ctrl);
      back->SetCbackMethodName("MenuExit");
    }
    gs.Step();

    CREATE_ADD_GLASS(test_menu, WGlButton, main_menu, "TestMenu", 0);
    gs.SetNode(test_menu);
    test_menu->SetCbackAlpha(test_menu);
    test_menu->SetCbackMethodName("MenuEnter");

    test_menu->SetRnrElements(false);
    {
      SGridStepper step(gs, true);

      CREATE_ADD_GLASS(but1, WGlButton, test_menu, "Rnd Statico", 0);
      step.SetNodeAdvance(but1);
      but1->SetCbackAlpha(tricam);
      but1->SetCbackMethodName("RandomStatico");

      CREATE_ADD_GLASS(but2, WGlButton, test_menu, "New LandMark", 0);
      step.SetNodeAdvance(but2);
      but2->SetCbackAlpha(tricam);
      but2->SetCbackMethodName("MakeLandMark");

      /*
      CREATE_ADD_GLASS(val1, WGlValuator, test_menu, "Sleep", 0);
      step.SetNodeAdvance(val1);
      val1->SetMin(1); val1->SetMax(1000);
      val1->SetFormat("Sleep: %.0f");
      val1->SetCbackAlpha(eventor);
      val1->SetCbackMemberName("InterBeatMS");
      */

      CREATE_ADD_GLASS(back, WGlButton, test_menu, " << ", 0);
      step.SetNodeAdvance(back);
      back->SetCbackAlpha(test_menu);
      back->SetCbackMethodName("MenuExit");
    }
    gs.Step();

    CREATE_ADD_GLASS(help_but, WGlButton, main_menu, "Help", 0);
    gs.SetNode(help_but);
    help_but->SetCbackAlpha(tricam);
    help_but->SetCbackMethodName("Help");
  }


  // ================================================================
  // Put menu components into class data.
  // --------------------------------------------------------------
  // This still sets tricam and tspupil as Alpha to some callbacks.
  // Should be properly handled from titles at menu activation time.
  // Also, one should clone the relevant menu entries then.

  CREATE_ADD_GLASS(lens_menu, WGlWidget, menu_scene, "LensMenu", 0);
  lens_menu->SetRnrElements(false);

  { // Glass mappings
    ZList *uidir = g_queen->AssertPath("var/glassui", "ZNameMap");
    ZList *gdir;
    WGlButton   *but;
    WGlValuator *val;

    {
      gdir = uidir->AssertPath("Extendio");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(but, WGlButton, menu, "<undef>", "LensName:LensBeta:TringuCamAlpha");
      but->SetCbackAlpha(tricam);
      but->SetCbackMethodName("ExtendioDetails");

      CREATE_ADD_GLASS(but, WGlButton, menu, "Detailed View", "LensBeta");
      but->SetCbackAlpha(g_shell);
      but->SetCbackMethodName("SpawnClassView");

      CREATE_ADD_GLASS(but, WGlButton, menu, "Explode", "LensBeta:TringuCamAlpha");
      but->SetCbackAlpha(tricam);
      but->SetCbackMethodName("ExtendioExplode");
    }

    {
      gdir = uidir->AssertPath("Statico");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(val, WGlValuator, menu, "CollCount", "LensAlpha");
      val->SetConstVal(true);
      val->SetFormat("Dyno colls: %.0f");
      val->SetCbackMemberName("NDynoColls");

      CREATE_ADD_GLASS(but, WGlButton, menu, "Connect Energy", "LensBeta:TringuCamAlpha");
      but->SetCbackAlpha(tricam);
      but->SetCbackMethodName("PrepConnectStatos");
      but->SetCbackValue(0);
      but->SetCbackString("r2y");

      CREATE_ADD_GLASS(but, WGlButton, menu, "Connect Metal", "LensBeta:TringuCamAlpha");
      but->SetCbackAlpha(tricam);
      but->SetCbackMethodName("PrepConnectStatos");
      but->SetCbackValue(1);
      but->SetCbackString("coldsteel");
    }

    {
      gdir = uidir->AssertPath("Dynamico");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Speed", "LensAlpha");
      val->SetMin(-10); val->SetMax(100); val->SetStepB(100);
      val->SetFormat("Speed: %.2f");
      val->SetCbackMemberName("V");

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Rot", "LensAlpha");
      val->SetMin(-10); val->SetMax(100); val->SetStepB(100);
      val->SetFormat("Rot: %.2f");
      val->SetCbackMemberName("W");
    }

    {
      gdir = uidir->AssertPath("Crawler");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(val, WGlValuator, menu, "LevH", "LensAlpha");
      gs.SetNodeAdvance(val);
      val->SetMin(-10); val->SetMax(100); val->SetStepB(100);
      val->SetFormat("LevH: %.2f");
      val->SetCbackMemberName("LevH");

      CREATE_ADD_GLASS(but, WGlButton, menu, "Drive", "LensBeta:TringuCamAlpha");
      but->SetCbackAlpha(tricam);
      but->SetCbackMethodName("DynoDrive");
    }

    {
      gdir = uidir->AssertPath("Flyer");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Height", "LensAlpha");
      gs.SetNodeAdvance(val);
      val->SetMin(-10); val->SetMax(100); val->SetStepB(100);
      val->SetFormat("H: %.2f");
      val->SetCbackMemberName("Height");
      val->SetConstVal(true);

    }

    {
      gdir = uidir->AssertPath("LandMark");
      CREATE_ADD_GLASS(menu, ZList, gdir, "menu", 0);

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Phi", "LensAlpha");
      gs.SetNodeAdvance(val);
      val->SetStepA(1); val->SetStepB(10);
      val->SetFormat("Phi: %.1f");
      val->SetCbackMemberName("Phi");

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Sx", "LensAlpha");
      gs.SetNodeAdvance(val);
      val->SetStepA(1); val->SetStepB(10);
      val->SetFormat("Sx: %.1f");
      val->SetCbackMemberName("Sx");

      CREATE_ADD_GLASS(val, WGlValuator, menu, "Sy", "LensAlpha");
      gs.SetNodeAdvance(val);
      val->SetStepA(1); val->SetStepB(10);
      val->SetFormat("Sy: %.1f");
      val->SetCbackMemberName("Sy");
    }
  }
}


/******************************************************************************/
// Various
/******************************************************************************/

void start_recording()
{
  eventor->Stop();
  gSystem->Sleep(100);

  eventor->ResetRecursively();
  eventor->SetInterBeatMS(40);
  eventor->SetEpochType(Eventor::ET_Manual);
  eventor->SetTimeSource(Eventor::TS_IntStep);
  eventor->SetTimeEpoch(0);
  eventor->SetTimeStep(0.04); // 25 fps

  gSystem->Exec("rm -rf capture/*; mkdir -p capture");

  dumper->SetWaitSignal(true);
  dumper->SetDumpImage(true);
  dumper->SetCopyToScreen(true);
  dumper->SetFileNameFmt("capture/img%05d"); // .tga added on dump
  dumper->SetDumpID(0);

  g_pupil->SetCHSize(0);
  g_pupil->SetMPSize(0.05);
  g_pupil->SetRnrFakeOverlayInCapture(true);
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
    tricam->GetCurField()->ColorizeTvor();
    lightmap->ModulateTvor();
    tringula->ReadUnlock();
    gSystem->Sleep(sleep);
  }
}

GravPlotter* plot_grav()
{
  GravPlotter* gp = new GravPlotter(parasurf);
  gp->StandardCanvas();
  return gp;
}
