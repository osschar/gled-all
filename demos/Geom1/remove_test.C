// $Header$

// Scene to test ZQueen::RemoveLenses() method.
//

// vars: ZQueen* scenes
// libs: Geom1

#include <glass_defines.h>
#include <gl_defines.h>

class RndSMorphCreator;

ZNode**           subnodes = 0;
Eventor*          eventor = 0;
RndSMorphCreator* smorph_creator = 0;
ZNode*            top_node = 0;

void remove_test(Int_t NSN = 32)
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Geom1");

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

void create_smorphs(Int_t NSN = 32, Int_t NNN = 1024, Int_t SMS = 0)
{
  // NSN - number of sub-nodes, each of them receives NNN SMorphs
  // NNN - number of SMorphs to put into each sub-node
  // SMS - SleepMiliSeconds (while waiting for eventor to finish)

  if(SMS == 0) SMS = 0.2*NNN;
  if(subnodes) delete [] subnodes;
  subnodes = new ZNode* [NSN];

  for(int i=0; i<NSN; ++i) {
    subnodes[i] = new ZNode(Form("SubNode %d", i+1));
    top_node->GetQueen()->CheckIn(subnodes[i]);
    top_node->Add(subnodes[i]);
  }

  eventor->SetBeatsToDo(NNN);
  for(int i=0; i<NSN; ++i) {
    smorph_creator->SetTarget(subnodes[i]);
    eventor->Reset();
    GTime t(GTime::I_Now);
    eventor->Start();
    gSystem->Sleep(SMS);
    while(eventor->GetRunning()) gSystem->Sleep(SMS);
    printf("Done creation of smorphs for subnode %d (%lu).\n", i,
	   t.TimeUntilNow().ToMiliSec());
  }
}
