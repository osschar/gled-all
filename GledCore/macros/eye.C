// $Header$

// Spawn a standard GUI.
//
// assumes: ZQueen* scenes; ZQueen* fire_queen (as declared by sun.C)

EyeInfo*       eye          = 0;
ShellInfo*     shell        = 0;
SubShellInfo*  def_subshell = 0;
PupilInfo*     pupil        = 0;

void eye(const Text_t* eye_name="Eye of Ra",
	 const Text_t* shell_name="Example Shell",
	 const Text_t* pupil_name="Example Pupil",
	 const Text_t* scene_name=0)
{
  if(Gled::theOne->HasGUILibs() == false) {
    printf("eye.C::eye skipping Eye and ShellInfo instantiation (no GUI libraries).\n");
  }

  register_GledCore_layouts();

  EyeInfo* pre_ei = new EyeInfo(eye_name);

  shell = new ShellInfo(shell_name);
  fire_queen->CheckIn(shell);
  fire_queen->Add(shell);
  def_subshell = shell->MakeDefSubShell();

  if(pupil_name) {
    pupil = new PupilInfo(pupil_name);
    fire_queen->CheckIn(pupil);
    shell->GetSubShells()->Add(pupil);
    ZGlass* sc = 0;
    if(scene_name != 0) sc = scenes->GetElementByName(scene_name);
    if(sc == 0)         sc = scenes->First();
    if(sc != 0)
      pupil->Add(sc);
  }
  
  //shell->SetDefSubShell(pupil);

  eye = Gled::theOne->SpawnEye(pre_ei, shell, "GledCore", "FTW_Shell");
}

/**************************************************************************/

NestInfo* new_nest(const Text_t* name="Nest X", const Text_t* layout = 0)
{
  NestInfo* ni = new NestInfo(name);
  fire_queen->CheckIn(ni);
  
  if(layout != 0) {
    ni->SetLayout(layout);
    ni->SetLeafLayout(NestInfo::LL_Custom);
  }

  shell->GetNests()->Add(ni);
  return ni;
}

/**************************************************************************/
/**************************************************************************/

void register_GledCore_layouts()
{
  Gled& g = *Gled::theOne;

  g.AddMTWLayout("GledCore/ZGlass",         "ZGlass(Name[20],Title[20],RefCount[6])");
  g.AddMTWLayout("GledCore/ZNode",          "ZNode(Pos[18],Rot[18],UseOM[4],OM[5],UseScale[4],Sx[5],Sy[5],Sz[5],KeepParent[4])");
  g.AddMTWLayout("GledCore/ZQueen Basic state",         
                                            "ZQueen(State,MinID,MaxID,IDSpan,IDsUsed,IDsPurged,IDsFree,AvgPurgLen,SgmPurgLen,PurgedMS,DeletedMS,ZeroRCPolicy)");
  g.AddMTWLayout("GledCore/ZQueen Authorization",         
                                            "ZQueen(AuthMode,Alignment,MapNoneTo)");
  g.AddMTWLayout("GledCore/Eventor",        "Eventor(Running[4],Performing[4],Start[4],Stop[4])");
  g.AddMTWLayout("GledCore/Saturn monitor", "SaturnInfo(LAvg1,LAvg5,LAvg15,Memory,MFree,Swap,SFree,CU_Total[6],CU_User[6])");
}
