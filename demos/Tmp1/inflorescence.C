#include <glass_defines.h>
#include <gl_defines.h>

const Text_t* bush_layout = "ZGlass(Name,Title[22])";

void inflorescence(int mode = 3)
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
   
   if (mode == 0)
   {
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
   }
   else if (mode == 1 || mode == 2)
   {
      DibotryoidHerb* mint = new DibotryoidHerb("Mint", "Mint");    
      if (mode == 1)
      {
         mint->SetName("Mint-Acropetal");
         mint->SetSendSignalDelay(13);
         mint->SetPlastocronMainAxis(2);
         mint->SetPlastocronLateralAxis(3);
         mint->SetSignalDelayMainAxis(1);
         mint->SetSignalDelayLateralAxis(1);
      }
      else {
         mint->SetName("Mint-Basipetal");
         mint->SetSendSignalDelay(20);
         mint->SetPlastocronMainAxis(2);
         mint->SetPlastocronLateralAxis(5);
         mint->SetSignalDelayMainAxis(1);
         mint->SetSignalDelayLateralAxis(4);
      }
      mint->SetLevel(30);
      mint->SetRnrMod(lm);
      mint->Produce();
      mint->RotateLF(1, 3, TMath::Pi()*0.5);
      mint->RotateLF(2, 3, TMath::Pi());
      float sx = 0.1;
      mint->SetUseScale(true);
      mint->SetScale(sx);        
      g_queen->CheckIn(mint);
      images->Add(mint);    
   }
   else
   {
      GrowingPanicle* panicle = new GrowingPanicle("Panicle", "GrowingPanicle");
      g_queen->CheckIn(panicle);
      images->Add(panicle);
      panicle->SetLevel(40);
      panicle->SetLateralAngle(35);
      panicle->SetRnrMod(lm);
      panicle->Produce();
      panicle->RotateLF(1, 3, TMath::Pi()*0.5);
      float sx = 0.01;
      panicle->SetUseScale(true);
      panicle->SetScale(sx);
   }
   
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
   g_pupil->SetClearColor(0.2, 0.2, 0.2);
   {
      ZList* l = g_fire_queen;
      l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
      l->Swallow(new ZGlass("Production",
                            "ZGlass(Name):ProductionRule(*)"));
      g_nest->SetLayout("ZGlass(Name):Weed(Level[6], Angle[12]):ProductionRule(Rule[20])");
      
      g_nest->SetLeafLayout(NestInfo::LL_Custom);
   }  
}