// $Header$

// Spheror example: navigate Single Spheror->Amoeba->Start() to activate
//   minimization thread.
// MultiSpheror spawns several Spherors and process them on moons.
//
// vars: ZQueen* scenes
// libs: Numerica
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Numerica");

  Scene* sph_scene  = new Scene("Spheror Scene");
  scenes->CheckIn(sph_scene);
  scenes->Add(sph_scene);

  Spheror* spheror = new Spheror("Single Spheror");
  scenes->CheckIn(spheror); sph_scene->Add(spheror);
  spheror->Set3Pos(0, 0, 2.5);
  spheror->SetNVert(30); spheror->SetBeautyP(true);
  spheror->Install();
  spheror->SelfInit();

  MultiSpheror* multi_spheror = new MultiSpheror("Grid Spheror");
  scenes->CheckIn(multi_spheror); sph_scene->Add(multi_spheror);
  multi_spheror->Init();


  // Spawn GUI
  gROOT->ProcessLine(".x eye.C");
}
