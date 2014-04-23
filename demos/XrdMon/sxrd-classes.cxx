// Preamble for SXrdClasses.cxx, generated by the extract-sxrd-classes.pl script.

#include "SXrdClasses.h"
#include "TMath.h"
#include "TTree.h"

void dump_xrdfar_tree_ioinfo(TTree *t, Long64_t ev)
{
  if (ev < 0 || ev >= t->GetEntriesFast())
  {
    fprintf(stderr, "Error, event %lld out of range (max=%lld).\n",
	    ev, t->GetEntriesFast());
    return;
  }

  SXrdIoInfo *xp = 0;
  t->SetBranchAddress("I.", &xp);
  if (xp == 0)
  {
    fprintf(stderr, "Error, branch with detailed info not found.\n",
	    ev, t->GetEntriesFast());
    return;
  }


  t->GetEntry(ev);

  xp->Dump(2);
}

