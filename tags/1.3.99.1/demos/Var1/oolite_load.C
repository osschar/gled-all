#include <glass_defines.h>
#include <gl_defines.h>

void oolite_load()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "OOlite Scene", "Testing functionality of OOlite mesh importer.");

  CREATE_ADD_GLASS(shiptex, ZImage, g_scene, "Ship texture", "");
  shiptex->SetFile("ootex/cobra3_redux.png");
  shiptex->SetEnvMode(GL_MODULATE);

  CREATE_ADD_GLASS(shipmesh, TriMesh, g_scene, "Ship mesh", "");
  shipmesh->ImportOoliteDAT("oomod/cobra3_redux.dat", true);
  shipmesh->StdDynamicoPostImport();
  shipmesh->SetDefTexture(shiptex);

  CREATE_ADD_GLASS(ship, Flyer, g_scene, "Spaceship", "");
  ship->SetMesh(shipmesh);
  ship->SetTexture(shiptex);
  ship->RefTrans().Scale(0.1, 0.1, 0.1);
  ship->update_last_data();

  Gled::Macro("eye.C");
}
