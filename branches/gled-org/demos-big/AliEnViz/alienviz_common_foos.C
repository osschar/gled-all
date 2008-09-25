void alienviz_common_foos()
{
  // Do nothing ... just to know it has been loaded.
}

/**************************************************************************/
// Loading and finding of fonts, textures and rnrmods
/**************************************************************************/

ZNameMap *aev_lib      = 0;
ZNameMap *aev_fonts    = 0;
ZNameMap *aev_rnrmods  = 0;
ZNameMap *aev_textures = 0;

void alienviz_setup_lib_objects()
{
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
  nameson->SetTileCol(0, 0.2, 0.7, 0.7);
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

class AEVAlienUI;

void globe_map(ZList* ss, AEVAlienUI* alien_ui)
{
  //g_saturn->LockMIRShooters(true);
  CREATE_ADD_GLASS(g_mapviz, AEVMapViz, ss, "Globe PDC-2004 MapViz", 0);
  g_mapviz->SetPos(3, -2, 0);
  g_mapviz->SetAlienUI(alien_ui);
  { // Manually create the single SphereSSpace
    CREATE_ADD_GLASS(globe, AEVSphereSSpace, g_mapviz, "Globe", 0);
    globe->SetScale(2);
    globe->SetTLevel(40); globe->SetPLevel(40);
    globe->SetTexture(earth_tex);
    globe->SetRotByDegrees(90, 50, 230);

  }
  g_mapviz->PopulateWSites(true);
  //g_saturn->UnlockMIRShooters();

  /*
  CREATE_ADD_GLASS(atube2, AEVN2NTube, ss, "Test Tube 2", 0);
  atube2->SetTLevel(20); atube2->SetPLevel(20);
  atube2->SetTexture(photon);
  atube2->SetNodeA((ZNode*) g_mapviz->FindLensByPath("Globe/Bari"));
  atube2->SetNodeB((ZNode*) g_mapviz->FindLensByPath("Globe/Houston"));
  atube2->Connect();
  */
}

void mercator_map(ZList* ss, AEVAlienUI* alien_ui)
{
  //g_saturn->LockMIRShooters(true);
  CREATE_ADD_GLASS(m_mapviz, AEVMapViz, ss, "Mercator PDC-2004 MapViz", 0);
  m_mapviz->SetPos(-3, -2, 0);
  m_mapviz->SetAlienUI(alien_ui);
  { // Manually create the single FlatSSpace ...
    CREATE_ADD_GLASS(earth_ss, AEVFlatSSpace, m_mapviz, "PlanarEarth", 0);
    earth_ss->SetTexture(earth_tex);
    earth_ss->SetULen(7);
    earth_ss->SetVLen(3.5);
  }
  m_mapviz->PopulateWSites(true);
  //g_saturn->UnlockMIRShooters();
}
