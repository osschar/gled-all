// $Header$

// Spheror example: navigate Single Spheror->Amoeba->Start() to activate
//   minimization thread.
// MultiSpheror spawns several Spherors and process them on moons.
//
// vars: ZQueen* g_queen
// libs: Numerica
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Numerica");

  Scene* sph_scene  = new Scene("Spheror Scene");
  g_queen->CheckIn(sph_scene);
  g_queen->Add(sph_scene);
  g_scene = sph_scene;

  Spheror* spheror = new Spheror("Single Spheror");
  g_queen->CheckIn(spheror); sph_scene->Add(spheror);
  spheror->SetPos(0, 0, 2.5);
  spheror->SetNVert(30); spheror->SetBeautyP(true);
  spheror->Install();
  spheror->SelfInit();

  MultiSpheror* multi_spheror = new MultiSpheror("Grid Spheror");
  g_queen->CheckIn(multi_spheror); sph_scene->Add(multi_spheror);
  multi_spheror->Init();


  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();
}
