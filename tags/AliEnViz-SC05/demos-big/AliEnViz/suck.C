// $Header$

// libs: Geom1 AliEnViz

#include <glass_defines.h>
#include <gl_defines.h>

//==============================================================================

class AEVMlSucker;

//------------------------------------------------------------------------------

AEVMlSucker   *ml_sucker = 0;

//==============================================================================

void suck()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("AliEnViz");

  ZQueen* ali_queen = new ZQueen(128*1024, "AliQueen", "Goddess of Ver");
  g_sun_king->Enthrone(ali_queen);
  ali_queen->SetMandatory(true);
  g_queen = ali_queen;

  ASSIGN_ADD_GLASS(ml_sucker, AEVMlSucker, ali_queen, "MonaLisa Sucker", 0);

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Scene", "Testing AEVMlSucker.");

  //==============================================================================

  Gled::Macro("eye.C");

  g_nest->PushFront(g_queen);
  {
    ZList* layouts = g_queen->AssertPath("var/layouts", "ZNameMap");
    fill_GledCore_layouts(layouts);

    layouts->Swallow("AliEnViz", new ZGlass("Jobs", "ZGlass(Name[12],Title[32])"));
    layouts->Swallow("AliEnViz", new ZGlass("Sites", "ZGlass(Name):AEVSite(*)"));
    layouts->Swallow("AliEnViz", new ZGlass("FlatSpace",
					    "Board(ULen[5],VLen[5],TexX0,TexY0,TexX1,TexY1):"
					    "AEVFlatSSpace(Theta0,DTheta,Phi0,DPhi)"));
    layouts->Swallow("AliEnViz", new ZGlass("ProcessingMonitor",
					    "AEVEventBatch(EvState,NWorkers,DataSizeMB)"));
    layouts->Swallow("WeaverSymbols", new ZGlass("ZNode:WS_Point",
						 "ZNode(Pos[18],Rot[18]):"
						 "WSPoint(W[4],S[4],T[4],Twist[4],Stretch[4])" ));

    g_nest->SetLayoutList(layouts);
  }

}
