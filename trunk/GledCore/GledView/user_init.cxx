#include <Gled/Gled.h>

void libGledCore_GLED_user_init_View()
{
  // The following should, in principle, be somewhere else.
  // Preferably in some user start-up script like eye.C

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

void *GledCore_GLED_user_init_View = (void*)libGledCore_GLED_user_init_View;
