// simple scene with a few sound sources
//
// vars: ZQueen* g_queen
// libs: Audio1 Geom1

#include <glass_defines.h>

// *** main ***

void al_test() {
  Gled::AssertLibSet("Geom1");
  Gled::AssertLibSet("Audio1");

  Gled::AssertMacro("sun_demos.C");

  sc = new Scene("Audio1 Demo Scene");
  g_queen->CheckIn(sc);
  g_queen->Add(sc);
  g_scene = sc;

  CREATE_ADD_GLASS(lamp, Lamp, sc, "Lamp", "");
  lamp->SetDiffuse(1, 1, 1);
  lamp->MoveLF(3, 5);
  lamp->SetScale(1);
  lamp->SetOnIfOff(true);

  CREATE_ADD_GLASS(base_plane, Rect, sc, "BasePlane", "");
  base_plane->SetUV(20,20);
  base_plane->SetUStrips(20); base_plane->SetVStrips(20);

  CREATE_ADD_GLASS(alctx, AlContext, sc, "AlContext", 0);
  alctx->Open();

  CREATE_ADD_GLASS(allis, AlListener, sc, "AlListener", 0);
  allis->MoveLF(1, 5);
  allis->RotateLF(1, 2, TMath::Pi());
  CREATE_ADD_GLASS(lcone, Cylinder, allis, "Listener Cone", 0);
  lcone->SetOrientation(Cylinder::O_X);
  lcone->SetROutTop(0);
  lcone->SetColor(1, 0.1, 0.3);

  CREATE_ADD_GLASS(sphere_src, AlSource, sc, "Omnidirectional AlSource", 0);
  sphere_src->SetFile("cuckoo.wav");
  CREATE_ADD_GLASS(sphere, Sphere, sphere_src, "Sphere", 0);

  // Add directional source
  CREATE_ADD_GLASS(cone_src, AlSource, sc, "Directional AlSource", 0);
  cone_src->MoveLF(1, -5);
  cone_src->SetFile("morse.wav");
  cone_src->SetConeInnerAngle(90);
  cone_src->SetConeOuterAngle(180);
  // Cone representing inner angle.
  CREATE_ADD_GLASS(scone, Cylinder, cone_src, "Cone", 0);
  scone->MoveLF(1, 1);
  scone->SetOrientation(Cylinder::O_X);
  scone->SetRInTop(1.9);
  scone->SetROutTop(2);
  scone->SetROutBase(0);
  scone->SetColor(0.3, 0.1, 1);

  // Eventor and a Mover
  CREATE_ADD_GLASS(dynamo, Eventor, sc, "Dynamo", 0);
  dynamo->SetBeatsToDo(-1);
  dynamo->SetStampInterval(10);
  dynamo->SetInterBeatMS(50);

  CREATE_ADD_GLASS(mm, Mover, dynamo, "Rotates_LampBase", 0);
  mm->SetNode(cone_src);
  mm->SetRi(1); mm->SetRj(2); mm->SetRa(0.01745);

  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();

  ZMIR* playmir;
  playmir = sphere_src->S_Play(-1);
  g_saturn->ShootMIR(playmir);
  playmir = cone_src->S_Play(-1);
  g_saturn->ShootMIR(playmir);

  dynamo->Start();
}
