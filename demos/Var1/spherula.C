// $Id$

// A simple scene demonstrating GTSTorus.

// vars: ZQueen* g_queen
// libs: Geom1 GledGTS

#include <glass_defines.h>
#include <gl_defines.h>

class GTSurf;
class LegendreCoefs;

class Statico;
class TriMesh;
class HTriMesh;
class Tringula;
class Planetes;

GTSurf            *gtsurf      = 0;
LegendreCoefs     *legendcoefs = 0;

Statico           *stato       = 0;
TriMesh           *mesh        = 0;
Planetes          *planet      = 0;
HTriMesh          *hmesh       = 0;

// Tringula          *tring       = 0;

void make_std();

void spherula()
{
  //Gled::theOne->AssertLibSet("Geom1");
  //Gled::theOne->AssertLibSet("GledGTS");
  Gled::theOne->AssertLibSet("Var1");

  Gled::AssertMacro("sun_demos.C");

  CREATE_ADD_GLASS(scene, Scene, g_queen, "Spherula Scene", "");
  g_scene = scene;

  // Geom elements

  CREATE_ADD_GLASS(base_plane, Rect, scene, "BasePlane", "");
  base_plane->SetUnitSquare(16);

  CREATE_ADD_GLASS(lamp, Lamp, scene, "Lamp", "");
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->SetPos(-2, -6, 6);
  lamp->RotateLF(1,2, TMath::Pi());

  scene->GetGlobLamps()->Add(lamp);

  ASSIGN_ADD_GLASS(mesh, TriMesh, scene, "TriMesh", 0);
  mesh->MakeIcosahedron();
  mesh->GenerateTriangleNormals();

  // Set as simple tringula (no observer):
  // mesh->GetTTvor()->GenerateVertexNormals();
  // ASSIGN_ADD_GLASS(tring, Tringula, scene, "Tringula", 0);
  // tring->SetMesh(mesh);
  // tring->MoveLF(1, 2.5);

  // Set as Statico
  ASSIGN_ADD_GLASS(stato, Statico, scene, "Statico with TriMesh", 0);
  stato->SetMesh(mesh);
  stato->ref_trans().MoveLF(1, 2.5);
  stato->SetColor(.5, .75, 1);


  // Hierarhico

  ASSIGN_ADD_GLASS(hmesh, HTriMesh, scene, "HTriMesh", 0);
  hmesh->MakeIcosahedron();
  hmesh->Subdivide(4, 2);
  hmesh->NormalizeVertices();
  hmesh->GenerateTriangleNormals();

  ASSIGN_ADD_GLASS(planet, Planetes, scene, "Planetes", 0);
  planet->SetMesh(hmesh);
  planet->ref_trans().MoveLF(2, 2.5);
  planet->SetColor(.75, .25, 0.5);

  // GTS models

  ASSIGN_ADD_GLASS(gtsurf, GTSurf, scene, "GTSurf", 0);
  gtsurf->MoveLF(1, -2.5);
  gtsurf->SetColor(1,0.8,0.2);

  ASSIGN_ADD_GLASS(legendcoefs, LegendreCoefs, scene, "LegendreCoefs", 0);
  legendcoefs->ReadEgmFile("EGM-2008.data", 1000);

  make_std();

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  g_shell->SpawnClassView(gtsurf);
  g_shell->SpawnClassView(legendcoefs);
}

void make_std()
{
  gtsurf->GenerateSphere(7);
  // gtsurf->Rescale(10);
}

void make_rnd(Int_t max_l=30, Double_t abs_scale=0.1, Double_t pow_scale=1.5)
{
  gtsurf->GenerateSphereThetaConst(7);
  gtsurf->LegendrofyScaleRandomMulti(max_l, abs_scale, pow_scale);
}

void make_leg1()
{
  gtsurf->GenerateSphere(7);
  gtsurf->LegendrofyAdd(legendcoefs, 1e-5, 200);
  // gtsurf->Rescale(10);
}

void make_leg2()
{
  gtsurf->GenerateSphere(7);
  gtsurf->LegendrofyMulti(legendcoefs, 1e-5, 400);
  // gtsurf->Rescale(10);
}
