#include <glass_defines.h>

ZStringMap *smap=0;
ZVector *vec =0;
ZGlass *a=0, *b=0, *c=0;

void string_map()
{
  Gled::AssertMacro("sun_demos.C");

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Scene", 0);
  ASSIGN_ADD_GLASS(smap, ZStringMap, g_scene, "ZStringMap", "CINT var smap");
  for(int i=1; i<=10; ++i) {
    ZGlass* l = new ZGlass(GForm("Lens %d", i));
    g_queen->CheckIn(l);
    smap->Add(l);
  }
  smap->AddLabel("Foobel");
  ASSIGN_ADD_GLASS(a, ZGlass, g_scene, "Alpha", "CINT var a");
  ASSIGN_ADD_GLASS(b, ZGlass, g_scene, "Bravo", "CINT var b");
  ASSIGN_ADD_GLASS(c, ZGlass, g_scene, "Charlie", "CINT var c");

  ASSIGN_ADD_GLASS(vec, ZVector, g_scene, "ZVector", "CINT var vec");
  vec->Add(a);
  vec->Resize(4);
  vec->SetElementById(c, 3);

  Gled::LoadMacro("eye.C");
  eye(false);
}
