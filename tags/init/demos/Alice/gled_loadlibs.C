{
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("RootGeo");
  gROOT->Macro("loadlibs.C");
  Gled::theOne->AssertLibSet("Alice");
  Gled::theOne->AssertLibSet("AliDet");
}
