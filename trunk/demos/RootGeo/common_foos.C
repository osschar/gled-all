// $Header$

// Common functions for ROOT geometry demos.

Scene* create_basic_scene()
{
  Scene* rscene = new Scene("Alice Detector Scene");
  scenes->CheckIn(rscene);
  scenes->Add(rscene);
  rscene->SetUseOM(true);

  CREATE_ADD_GLASS(origin, ZNode, rscene, "Lamp Origin", "");

  CREATE_ADD_GLASS(lamp1, Lamp, origin, "Lamp1", "");
  lamp1->SetDiffuse(1, 1, 1);
  lamp1->SetAmbient(0.3, 0.3, 0.3);
  lamp1->SetScale(0);
  lamp1->Set3Pos(0, 10, 10);
  
  CREATE_ADD_GLASS(lamp2, Lamp, origin, "Lamp2", "");
  lamp2->SetDiffuse(0.65, 0.35, 0.5);
  lamp2->SetAmbient(0, 0, 0);
  lamp2->SetScale(0);
  lamp2->Set3Pos(10, 10, 0);
 
  rscene->GetGlobLamps()->Add(lamp1);
  rscene->GetGlobLamps()->Add(lamp2);
 
  CREATE_ADD_GLASS(el, Eventor, origin, "Dynamo", "Rotates Lamp Origin");
  el->SetBeatsToDo(-1); el->SetInterBeatMS(100); el->SetStampInterval(10);
  CREATE_ADD_GLASS(mv, Mover, el, "Lamp Origin Rotator", "");
  mv->SetNode(origin); mv->SetRi(1); mv->SetRj(3); mv->SetRa(0.005);
 
  CREATE_ADD_GLASS(cam_base, Sphere, rscene, "Camera Base", "");
  cam_base->Set3Pos(7, 4, 7);
  cam_base->SetRotByDegrees(180, -45, 90);
  cam_base->RotateLF(3, 1, 22*TMath::DegToRad());
  cam_base->SetRadius(0.01);

  CREATE_ADD_GLASS(pointmod, ZGlBlending, rscene, "Point modificator", 0);
  pointmod->SetAntiAliasOp(1);
  pointmod->SetPointSize(10);

  CREATE_ADD_GLASS(lightmod, ZGlLightModel, rscene, "Light model", 0);
  lightmod->SetShadeModelOp(1);
  lightmod->SetFaceCullOp(1);

  CREATE_ADD_GLASS(xzplane, Rect, rscene, "X-Z Plane", "");
  xzplane->SetRotByDegrees(180, 0, -90);
  xzplane->SetUnitSquare(20);
  xzplane->SetColor(0.4, 1, 0.45, 0.4);

  return rscene;
}
