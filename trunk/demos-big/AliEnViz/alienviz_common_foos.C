void alienviz_common_foos()
{
  // Do nothing ... just to know it has been loaded.
}

/**************************************************************************/
// Loading and finding of fonts, textures and rnrmods
/**************************************************************************/

ZQueen   *aev_queen    = 0;
Scene    *aev_scene    = 0;

ZNameMap *aev_lib      = 0;
ZNameMap *aev_fonts    = 0;
ZNameMap *aev_rnrmods  = 0;
ZNameMap *aev_textures = 0;

void alienviz_setup_lib_objects(Int_t queen_size)
{
  aev_queen = new ZQueen(queen_size, "AEVQueen", "Goddess of Ver");
  g_sun_king->Enthrone(aev_queen);
  aev_queen->SetMandatory(true);

  g_queen = aev_queen;

  ASSIGN_ADD_GLASS(aev_lib, ZNameMap, g_queen, "lib", 0);

  ASSIGN_ADD_GLASS(aev_fonts,    ZNameMap, aev_lib, "fonts", 0);
  ASSIGN_ADD_GLASS(aev_rnrmods,  ZNameMap, aev_lib, "rnrmods", 0);
  ASSIGN_ADD_GLASS(aev_textures, ZNameMap, aev_lib, "textures", 0);

  // Textures
  //---------

  CREATE_ADD_GLASS(earth_tex, ZImage, aev_textures, "Earth", 0);
  init_texture(earth_tex, "imgs/earth_texture2048x1024.png");
  CREATE_ADD_GLASS(photon, ZImage, aev_textures, "Photon", 0);
  init_texture(photon, "imgs/photon-blurred.png");
  photon->SetEnvColor(0.6, 0.6, 0.6);
  CREATE_ADD_GLASS(checker, ZImage, aev_textures, "Checker", 0);
  init_texture(checker, "imgs/checker_8a.png");
  checker->SetEnvColor(0.6, 0.6, 0.6);

  // Spherical textures
  CREATE_ADD_GLASS(gled_tex, ZImage, aev_textures, "Gled", 0);
  init_texture(gled_tex, "imgs/Star_Nursery.jpg");
  CREATE_ADD_GLASS(glite_tex, ZImage, aev_textures, "gLite", 0);
  // init_texture(glite_tex, "imgs/icky_intestines.jpg");
  init_texture(glite_tex, "imgs/marble.jpg");
  CREATE_ADD_GLASS(proof_tex, ZImage, aev_textures, "PROOF", 0);
  init_texture(proof_tex, "imgs/fractal_northpole.jpg");
  CREATE_ADD_GLASS(dauser_tex, ZImage, aev_textures, "DA User", 0);
  init_texture(dauser_tex, "imgs/chrome_circuit.jpg");
  // 1D textured for tubes
  CREATE_ADD_GLASS(abs1_tex, ZImage, aev_textures, "Abstract1", 0);
  init_texture(abs1_tex, "gradients/lin_abstract1.png");
  CREATE_ADD_GLASS(abs2_tex, ZImage, aev_textures, "Abstract2", 0);
  init_texture(abs2_tex, "gradients/lin_abstract2.png");
  CREATE_ADD_GLASS(abs3_tex, ZImage, aev_textures, "Abstract3", 0);
  init_texture(abs3_tex, "gradients/lin_abstract3.png");
  CREATE_ADD_GLASS(r2y_tex, ZImage, aev_textures, "Red2Yellow", 0);
  init_texture(r2y_tex, "gradients/lin_r2y.png");
  CREATE_ADD_GLASS(r2c_tex, ZImage, aev_textures, "Red2Cyan", 0);
  init_texture(r2c_tex, "gradients/lin_r2c.png");
  CREATE_ADD_GLASS(landsea_tex, ZImage, aev_textures, "LandSea", 0);
  init_texture(landsea_tex, "gradients/lin_landsea.png");


  // Fonts
  //------

  CREATE_ADD_GLASS(largefont, ZRlFont, aev_fonts, "LargeFont", 0);
  largefont->SetFontFile("fonts/helvetica34.txf");


  // RnrMods
  //--------

  CREATE_ADD_GLASS(backculler, ZGlLightModel, aev_rnrmods, "BackCuller", 0);
  backculler->SetFaceCullOp(1);

  CREATE_ADD_GLASS(nameson, ZRlNodeMarkup, aev_rnrmods, "NamesOn", 0);
  nameson->SetNodeMarkupOp(1);
  nameson->SetTileCol(0, 0.2, 0.7, 0.5);
  nameson->SetTilePos("t");
  CREATE_ADD_GLASS(namesoff, ZRlNodeMarkup, aev_rnrmods, "NamesOff", 0);
  namesoff->SetNodeMarkupOp(0);

  CREATE_ADD_GLASS(dirfs, WGlFrameStyle, aev_rnrmods, "DirectoryFrameStyle", 0);
  dirfs->StandardPixel();
  dirfs->SetDefDx(140);
  dirfs->SetTextYSize(16);
  dirfs->SetYBorder(2);

  CREATE_ADD_GLASS(menufs, WGlFrameStyle, aev_rnrmods, "MenuFrameStyle", 0);
  menufs->StandardPixel();
  menufs->SetDefDx(160);
  menufs->SetTextYSize(20);
  menufs->SetYBorder(2);
}

ZImage* find_texture(const TString& name)
{
  return dynamic_cast<ZImage*>(aev_textures->GetElementByName(name));
}

ZRlFont* find_font(const TString& name)
{
  return dynamic_cast<ZRlFont*>(aev_fonts->GetElementByName(name));
}

ZRnrModBase* find_rnrmod(const TString& name)
{
  return dynamic_cast<ZRnrModBase*>(aev_rnrmods->GetElementByName(name));
}

/**************************************************************************/
// Scene objects
/**************************************************************************/

void alienviz_setup_scene()
{
  ASSIGN_ADD_GLASS(aev_scene, Scene, aev_queen, "AEVScene", "");

  g_scene = aev_scene;

  CREATE_ADD_GLASS(l, Lamp, aev_scene, "Lamp", 0);
  l->SetRnrSelf(false);
  l->SetLampScale(0);
  l->SetPos(0, 2, 10);
  l->SetAmbient(0.5, 0.5, 0.5);
  aev_scene->GetGlobLamps()->Add(l);

  CREATE_ADD_GLASS(bases, ZNode, aev_queen, "CameraBases", 0);

  aev_scene->Add(bases);

  CREATE_ADD_GLASS(cam_base, Sphere, bases, "Home", 0);
  cam_base->SetPos(0, -6.5, 5);
  cam_base->SetRotByDegrees(90, -30, 90);
  cam_base->SetRadius(0.01);
  cam_base->SetRnrSelf(false);

  CREATE_ADD_GLASS(view_base_1, Sphere, bases, "View1", 0);
  view_base_1->SetPos(2.6, -4.95, 0.9);
  view_base_1->SetRadius(0.01);
  view_base_1->SetRnrSelf(false);

  CREATE_ADD_GLASS(view_base_2, Sphere, bases, "View2", 0);
  view_base_2->SetPos(5.8, -2.25, 0.7);
  view_base_2->SetRadius(0.01);
  view_base_2->SetRnrSelf(false);
}

ZNode* find_cambase(const TString& name)
{
  TString path("CameraBases/"); path += name;
  ZNode* ret = dynamic_cast<ZNode*>(aev_scene->FindLensByPath(path));
  if (ret == 0)
    Warning("find_cambase", "failed for path '%s'.", path.Data());
  return ret;
}

/**************************************************************************/
// Eye spawner
/**************************************************************************/

void alienviz_spawn_eye()
{
  printf("Instantiating GUI ...\n");

  Gled::AssertMacro("gled_view_globals.C");

  //g_saturn->LockMIRShooters(true);
  g_shell = new ShellInfo("AliEnViz Shell");
  // fire_queen->CheckIn(shell); fire_queen->Add(shell);
  g_queen->CheckIn(g_shell);
  g_queen->Add(g_shell);

  ASSIGN_ATT_GLASS(g_nest, NestInfo, g_shell, SetDefSubShell, "Nest", 0);
  g_nest->Add(g_queen);
  // g_nest->Add(g_scene);
  // g_nest->ImportKings();   // Get all Kings as top level objects

  ASSIGN_ATT_GLASS(g_pupil, PupilInfo, g_shell, AddSubShell, "AEVPupil", 0);
  g_pupil->Add(g_scene);
  g_pupil->SetWidth(800); g_pupil->SetHeight(600);
  g_pupil->SetClearColor(0.209, 0.269, 0.248);
  g_pupil->SetUpReference(g_scene);
  g_pupil->SetUpRefAxis(3);
  g_pupil->SetCameraBase(find_cambase("Home"));
  g_pupil->SetBackMode(GL_FILL);
  g_pupil->SetCHSize(0);

  g_pupil->SetShowRPS(false);
  g_pupil->SetShowView(false);

  Gled::LoadMacro("eye.C");

  ZList* layouts = register_GledCore_layouts(g_shell);

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

  Gled::theOne->SpawnEye(0, g_shell, "GledCore", "FTW_Shell");
}

/**************************************************************************/
// Lens-init foos
/**************************************************************************/

void init_texture(ZImage* tex, const char* fname, bool loadp=true)
{
  tex->SetMagFilter(GL_LINEAR);
  tex->SetMinFilter(GL_LINEAR);
  tex->SetEnvMode(GL_MODULATE);
  tex->SetFile(fname);
  if(loadp) {
    tex->Load();
    tex->SetLoadAdEnlight(true);
  }
}

/**************************************************************************/
// Somewhat usefull scene elements
/**************************************************************************/

  /*
  CREATE_ADD_GLASS(atube2, AEVN2NTube, ss, "Test Tube 2", 0);
  atube2->SetTLevel(20); atube2->SetPLevel(20);
  atube2->SetTexture(photon);
  atube2->SetNodeA((ZNode*) g_mapviz->FindLensByPath("Globe/Bari"));
  atube2->SetNodeB((ZNode*) g_mapviz->FindLensByPath("Globe/Houston"));
  atube2->Connect();
  */

AEVMapViz* globe_map(AList* parent=0)
{
  if (parent == 0) parent = g_scene;

  CREATE_ADD_GLASS(mapviz, AEVMapViz, parent, "Globe MapViz", 0);
  mapviz->SetPos(5, 2, 2);
  { // Manually create the single SphereSSpace
    CREATE_ADD_GLASS(globe, AEVSphereSSpace, mapviz, "Globe", 0);
    globe->SetScale(2);
    globe->SetTLevel(40); globe->SetPLevel(40);
    globe->SetTexture(find_texture("Earth"));
    globe->SetRotByDegrees(70, 100, 230);
  }

  return mapviz;
}

AEVMapViz* mercator_map(AList* parent=0)
{
  if (parent == 0) parent = g_scene;

  CREATE_ADD_GLASS(mapviz, AEVMapViz, parent, "Mercator MapViz", 0);
  mapviz->SetPos(-4, 2, 2);
  mapviz->SetRotByDegrees(0, 0, 60);
  { // Manually create the single FlatSSpace ...
    CREATE_ADD_GLASS(earth_ss, AEVFlatSSpace, mapviz, "PlanarEarth", 0);
    earth_ss->SetTexture(find_texture("Earth"));
    earth_ss->SetULen(7);
    earth_ss->SetVLen(3.5);
  }

  return mapviz;
}
