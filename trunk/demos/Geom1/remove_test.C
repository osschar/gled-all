// $Header$

// Scene to test ZQueen::RemoveLenses() method.
//

// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

#include "rt_functions.h"

class RndSMorphCreator;

void remove_test(Int_t NSN = 32)
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");
  gSystem->Exec("make rt_functions.so");
  gSystem->Load("rt_functions.so");

  Scene* images  = new Scene("Images");
  scenes->CheckIn(images);
  scenes->Add(images);

  // Top-node to receive subnodes (children) and smorphs (grand-children).

  CREATE_ADD_GLASS(rs_node, ZNode, images, "RndSmorph Node", 0);
  top_node = rs_node;

  // Eventor and smorph creator.

  CREATE_ADD_GLASS(rs_eventor, Eventor, images, "Eventor", 0);
  rs_eventor->SetInterBeatMS(0);
  CREATE_ADD_GLASS(rs_op, RndSMorphCreator, rs_eventor, "SMorph Creator", 0);
  rs_op->SetReportID(false);
  rs_op->SetGetResult(false);

  rs_op->SetTarget(rs_node);

  eventor        = rs_eventor;
  smorph_creator = rs_op;

  create_smorphs(NSN);

  // Spawn GUI
  gROOT->LoadMacro("eye.C");
  eye("Eyeeous", "Shellious", 0);
}
