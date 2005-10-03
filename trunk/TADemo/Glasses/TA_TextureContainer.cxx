// $Header$

#include "TA_TextureContainer.h"
#include "TA_TextureContainer.c7"

/**************************************************************************/

int _TA_Texture::Load(const char* fname, int w, int h)
{
  fW = w; fH = h;
  FILE* in = fopen(fname, "r"); if(!in) return 1;
  if(pData) delete [] pData; pData = new GLubyte[fW*fH*3]; if(!pData) return 2;
  size_t ri = fread(pData, 3, fW*fH, in); fclose(in);
  if(ri != fW*fH ) {
    delete [] pData; pData = 0; fW = 0; fH = 0;
    return 3;
  }
  bTexLoaded = true;
  return 0;
}

/**************************************************************************/

ClassImp(TA_TextureContainer)

void TA_TextureContainer::_init()
{
  mDescDir = ".";
}

void TA_TextureContainer::ProcessDescFile()
{
  FILE* tf = fopen(GForm("%s/textures.dir", mDescDir.Data()), "r");
  while(!feof(tf)) {
    char name[32], fullname[256]; int w, h;
    fscanf(tf, "%s %d %d", name, &w, &h);
    sprintf(fullname, "%s/textures_8bitRGB/%s.rgb", mDescDir.Data(), name);
    _TA_Texture* tex = new _TA_Texture;
    int ret = tex->Load(fullname, w, h);
    if(ret) {
      cout <<"!@@#!!! Failed loading texture "<< name <<" retcode="<< ret <<endl;
      delete tex;
      continue;
    }
    mName2Tex[name] = tex;
  }
  fclose(tf);
}

_TA_Texture*
TA_TextureContainer::FindTexture(const TString& name) {
  hName2p_TA_Texture_i i = mName2Tex.find(name);
  if(i == mName2Tex.end()) {
    cout <<"Unresolved texture '"<< name <<"'\n";
    return 0;
  }
  return i->second;
}
