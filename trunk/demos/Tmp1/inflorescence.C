#include <glass_defines.h>
#include <gl_defines.h>

const Text_t* bush_layout = "ZGlass(Name,Title[22])";

void inflorescence()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("GledCore");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("Tmp1");
  
  ZGlLightModel* lm = new ZGlLightModel("ZGlLightModel 1");
  g_queen->CheckIn(lm);
  lm->SetShadeModelOp(1);
  lm->SetFrontMode(GL_FILL);
  lm->SetBackMode(GL_FILL);
  lm->SetFaceCullOp(0);
  
  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;
  
  CREATE_ADD_GLASS(base, Sphere, images, "cam base", "");
  base->SetPos(0, 0, 10);
  base->SetRotByDegrees(0, -90, 0);
  base->SetRadius(0.002);
  
  MonopodialHerb* bp = new MonopodialHerb("Bursa", "Bursa Pastoralis");
  g_queen->CheckIn(bp);
  images->Add(bp);
  bp->SetLevel(15);
  bp->SetRnrMod(lm);
  bp->Produce();
  bp->RotateLF(1, 3, TMath::Pi()*0.5);
  float sx = 0.01;
  bp->SetUseScale(true);
  bp->SetScale(sx);
  
  //______________________________________________________________________________  
  
  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(2);
  g_queen->CheckIn(base_plane);
  images->Add(base_plane);
  base_plane->SetColor(0.7, 0.7, 0.7);
  
  //______________________________________________________________________________
  
  // Spawn GUI
  Gled::Macro("eye.C");  
  g_pupil->SetMoveOM(-3);
  g_pupil->EmitCameraHomeRay();
  g_pupil->SetCHSize(0);

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l->Swallow(new ZGlass("Production",
                          "ZGlass(Name):ProductionRule(*)"));
    g_nest->SetLayout("ZGlass(Name):Weed(Level[6], Angle[12]):ProductionRule(Rule[20])");
    
    g_nest->SetLeafLayout(NestInfo::LL_Custom);
  }  
}