// $Header$

// simple scene with images and RectTerrain
//
// crystal.png bacteria.png are 512x512 AFM scans, eg:
//   image3->SetFile("bacteria.png");
//   image3->Load();
//   terrain->SetFromImage(image3);

// vars: ZQueen* scenes
// libs: Geom1
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

  Scene* images  = new Scene("Images");
  scenes->CheckIn(images);
  scenes->Add(images);

  // Images

  ZImage* image1 = new ZImage("GledLogo");
  scenes->CheckIn(image1);
  images->Add(image1);
  image1->SetFile("gledlogo.png");
  image1->Load();

  ZImage* image2 = new ZImage("Orchid");
  scenes->CheckIn(image2);
  images->Add(image2);
  image2->SetFile("orchid.jpeg");
  image2->Load();

  ZImage* image3 = new ZImage("HeightField");
  scenes->CheckIn(image3);
  images->Add(image3);
  image3->SetFile("terrain_128.png");
  image3->Load();

  // Geom elements

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(16);
  scenes->CheckIn(base_plane);
  images->Add(base_plane);


  Lamp* l = new Lamp("Lamp");
  l->SetDiffuse(0.8, 0.8, 0.8);
  l->MoveLF(3, 10); l->RotateLF(1,2, TMath::Pi());
  scenes->CheckIn(l); images->Add(l);
  images->GetGlobLamps()->Add(l);
  ZNode* n = new Sphere(0.5, "Sph");
  scenes->CheckIn(n); l->Add(n);


  Board* board1 = new Board("Board1", "Showing GledLogo");
  scenes->CheckIn(board1);
  images->Add(board1);
  board1->SetTexture(image1);
  board1->SetULen(2.5*1.75); board1->SetVLen(2.5*1);
  board1->Set3Pos(-3, 3, 0.01);

  Board* board2 = new Board("Board2", "Showing Orchid");
  scenes->CheckIn(board2);
  images->Add(board2);
  board2->SetTexture(image2);
  board2->SetULen(2.5); board2->SetVLen(2.5);
  board2->Set3Pos(3, 3, 0.01);

  RectTerrain* terrain = new RectTerrain("Terrain");
  scenes->CheckIn(terrain);
  images->Add(terrain);
  terrain->SetFromImage(image3);
  terrain->SetDx(0.05); terrain->SetDy(0.05);
  terrain->SetZScale(0.5);
  terrain->SetMinCol(1,0,0); terrain->SetMaxCol(0,1,1);
  terrain->ToCenter();
  terrain->Set3Pos(0, -3, 0.01);

}
