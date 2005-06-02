#include <Gled/GledTypes.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TInterpreter.h>

void libAlice_GLED_user_init()
{
  const Text_t* alishare = gSystem->Getenv("ALIGLEDSHARE");
  if(alishare) {
    gROOT->SetMacroPath(GForm("%s:%s/macros", gROOT->GetMacroPath(),
			      alishare));
    gInterpreter->AddIncludePath(GForm("%s/macros", alishare));
  }
  printf("USERINITDONE\n");
}

void *Alice_GLED_user_init = (void*)libAlice_GLED_user_init;
