// $Header$
// std_auth.C ~ set-up of standard Gled auth mechanisms
// requires: spawned saturn

#include <glass_defines.h>

void load_std_identities(ZSunQueen* sq)
{
  sq->GetOrImportIdentity("sun.absolute");
  sq->GetOrImportIdentity("saturn");
  sq->GetOrImportIdentity("neptune");
  sq->GetOrImportIdentity("venus");
  sq->GetOrImportIdentity("mercury");
}

void load_group_identities(ZSunQueen* sq)
{
  TString owd(gSystem->WorkingDirectory());
  TSystemDirectory dir("groups", GForm("%s/groups", Gled::theOne->GetAuthDir()));
  TList* ls = dir.GetListOfFiles();
  TSystemFile *file;
  TIter next(ls);
  while((file = (TSystemFile*) next())) {
    if(! file->IsDirectory())
      sq->GetOrImportIdentity(file->GetName());
  }
  delete ls;
  gSystem->ChangeDirectory(owd.Data());
}

void std_auth()
{
  // Query basic elements
  ZSunQueen* sun_queen = Gled::theOne->GetSaturn()->GetSunQueen();
  QUERY_GLASS(ids, ZNameMap, sun_queen, "Auth/Identities");
  QUERY_GLASS(idlists, ZNameMap, sun_queen, "Auth/IdentityLists");

  // Load-up standard identities and all available groups
  load_std_identities(sun_queen);
  load_group_identities(sun_queen);
  
  CREATE_GLASS(eyed_saturns, ZHashList, sun_queen,
	       "EyedSaturns", "Saturns that accept Eye connections");
  idlists->Add(eyed_saturns);
  eyed_saturns->Add(ids->Query("sun.absolute"));
  eyed_saturns->Add(ids->Query("saturn"));
  eyed_saturns->Add(ids->Query("venus"));

  CREATE_GLASS(mooned_saturns, ZHashList, sun_queen,
	       "MoonedSaturns", "Saturns that accept Moon connections");
  idlists->Add(mooned_saturns);
  mooned_saturns->Add(ids->Query("sun.absolute"));
  mooned_saturns->Add(ids->Query("saturn"));
  mooned_saturns->Add(ids->Query("neptune"));

  ZNameMap* filt = dynamic_cast<ZNameMap*>(sun_queen->Query("Auth/Filters"));
  if(filt == 0) {
    printf("std_auth: can not retrieve Auth/Filters directory\n");
    return;
  }

  CREATE_GLASS(sq_guard, ZFilterAggregator, sun_queen, "SunQueen Guard", 0);
  filt->Add(sq_guard);
  CREATE_GLASS(sq_guard_filters, ZHashList, sun_queen, "SunQueen Filters", 0);
  sq_guard->SetFilters(sq_guard_filters);
  {
    CREATE_ADD_GLASS(f, ZMethodTagPreFilter, sq_guard_filters,
		     "ID_Management", "Allow identity attachment/detachment");
    f->SetMapNoneTo(ZMirFilter::R_Allow);
    f->SetTags("SunQueen::ID_Management");
  }
  {
    CREATE_ADD_GLASS(f, ZMethodTagPreFilter, sq_guard_filters,
		     "SaturnConnections", "Allow Moon attachment/detachment");
    f->SetTags("SunQueen::SaturnConnections");
    CREATE_GLASS(ff, ZIdentityListFilter, sun_queen, "AllowMoonedSaturns", 0);
    f->SetFilter(ff);
    ff->SetIdentities(mooned_saturns);
  }
  {
    CREATE_ADD_GLASS(f, ZMethodTagPreFilter, sq_guard_filters,
		     "EyeConnections", "Allow Eye attachment/detachment");
    f->SetTags("SunQueen::EyeConnections");

    CREATE_GLASS(ff, ZIdentityListFilter, sun_queen, "AllowEyedSaturns", 0);
    f->SetFilter(ff);
    ff->SetIdentities(eyed_saturns);    
  }

  sun_queen->SetGuard(sq_guard); // Simple SunQueen Guard
}

