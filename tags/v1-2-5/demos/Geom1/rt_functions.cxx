// $Header$

#include "rt_functions.h"

#include <Gled/GTime.h>
#include <Glasses/ZQueen.h>
#include <Glasses/Eventor.h>
#include <Glasses/Scene.h>
#include <Glasses/SMorph.h>
#include <Glasses/RndSMorphCreator.h>

#include <TSystem.h>

ZNode**           subnodes = 0;
Eventor*          eventor = 0;
RndSMorphCreator* smorph_creator = 0;
ZNode*            top_node = 0;

void create_smorphs(Int_t NSN, Int_t NNN, Int_t SMS)
{
  // NSN - number of sub-nodes, each of them receives NNN SMorphs
  // NNN - number of SMorphs to put into each sub-node
  // SMS - SleepMiliSeconds (while waiting for eventor to finish)

  if(SMS == 0) SMS = Int_t(0.05*NNN);
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
  sleep_label:
    gSystem->Sleep(SMS);
    if(eventor->GetRunning())
      goto sleep_label;
    printf("Done creation of smorphs for subnode %d (%lu).\n", i,
	   t.TimeUntilNow().ToMiliSec());
  }
}
