// $Header$

// Spawn a standard GUI.
//

void eye(Bool_t spawn_pupil=true)
{

  if(Gled::theOne->HasGUILibs() == false) {
    printf("eye.C::eye skipping Eye and ShellInfo instantiation (no GUI libraries).\n");
    return;
  }

  Gled::AssertMacro("gled_view_globals.C");

  if(g_eye == 0) {
    register_GledCore_layouts();

    g_shell = new ShellInfo("Shell");
    g_fire_queen->CheckIn(g_shell);
    g_fire_queen->Add(g_shell);

    g_nest = new_nest();
    g_shell->SetDefSubShell(g_nest);    
  }

  if(spawn_pupil)
    g_pupil = new_pupil();

  if(g_scene != 0) {
    if(g_nest)  g_nest->Add(g_scene);
    if(g_pupil) g_pupil->Add(g_scene);
  }

  if(g_eye == 0)
    g_eye = Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");
}

/**************************************************************************/

NestInfo* new_nest(const Text_t* name="Nest", const Text_t* layout = 0)
{
  if(Gled::theOne->HasGUILibs() == false) return;

  if(g_shell == 0) {
    printf("eye.C::new_nest g_shell is null. Returning zero.\n");
    return 0;
  }

  NestInfo* ni = new NestInfo(name);
  g_fire_queen->CheckIn(ni);
  
  if(layout != 0) {
    ni->SetLayout(layout);
    ni->SetLeafLayout(NestInfo::LL_Custom);
  }

  g_shell->AddSubShell(ni);
  return ni;
}

PupilInfo* new_pupil(const Text_t* name="Pupil")
{
  if(Gled::theOne->HasGUILibs() == false) return;

  if(g_shell == 0) {
    printf("eye.C::new_pupil g_shell is null. Returning zero.\n");
    return 0;
  }
 
  PupilInfo* pi = new PupilInfo(name);
  g_fire_queen->CheckIn(pi);
  g_shell->AddSubShell(pi);

  return pi;
}

void setup_pupil_up_reference(ZNode* n=0, Int_t a=3)
{
  if(Gled::theOne->HasGUILibs() == false) return;

  if(n == 0) n = g_scene;
  if(g_pupil && n) {
    g_pupil->SetUpReference(n);
    g_pupil->SetUpRefAxis(a);
  }
}

/**************************************************************************/
/**************************************************************************/

ZList* register_GledCore_layouts(ZList* top=0)
{
  if(Gled::theOne->HasGUILibs() == false) return;

  if(top == 0) top = g_fire_queen;

  top = top->AssertPath(NestInfo::sLayoutPath, "ZNameMap");

  fill_GledCore_layouts(top);

  return top;
}

ZList* fill_GledCore_layouts(ZList* top)
{
  ZList* l = top->AssertPath("GledCore", "ZList");

  l->Swallow(new ZGlass("ZGlass",         "ZGlass(Name[20],Title[20],RefCount[6])"));
  l->Swallow(new ZGlass("ZNode",          "ZNode(Pos[18],Rot[18],UseOM[4],OM[5],UseScale[4],Sx[5],Sy[5],Sz[5],KeepParent[4])"));
  l->Swallow(new ZGlass("ZQueen Basic state",         
		       "ZQueen(State,MinID,MaxID,IDSpan,IDsUsed,IDsPurged,IDsFree,AvgPurgLen,SgmPurgLen,PurgedMS,DeletedMS,ZeroRCPolicy)"));
  l->Swallow(new ZGlass("ZQueen Authorization",         
		       "ZQueen(AuthMode,Alignment,MapNoneTo)"));
  l->Swallow(new ZGlass("Eventor",        "Eventor(Running[4],Performing[4],Start[4],Stop[4])"));
  l->Swallow(new ZGlass("Saturn monitor", "SaturnInfo(LAvg1,LAvg5,LAvg15,Memory,MFree,Swap,SFree,CU_Total[6],CU_User[6])"));

  return l;
}
