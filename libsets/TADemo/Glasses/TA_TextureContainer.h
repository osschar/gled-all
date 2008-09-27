// $Header$
#ifndef TADemo_TA_TextureContainer_H
#define TADemo_TA_TextureContainer_H

#include <Glasses/ZGlass.h>

#ifndef __CINT__

#include <FL/gl.h>

struct _TA_Texture {
  GLubyte*	pData;
  int		fW, fH;
  GLuint	fTexture;
  bool		bTexLoaded, bTexInited;

  _TA_Texture() : pData(0), fTexture(0), bTexLoaded(false), bTexInited(false) {}
  int  Load(const char* fname, int w, int h);
  void GL_Use();
  void GL_UnUse();
};

typedef hash_map<TString, _TA_Texture*>			hName2p_TA_Texture_t;
typedef hash_map<TString, _TA_Texture*>::iterator	hName2p_TA_Texture_i;

#else

class _TA_Texture;

#endif

/**************************************************************************/

class TA_TextureContainer : public ZGlass {
private:
  void _init();

protected:
  TString		mDescDir;	  // X{GS} 7 Filor()
#ifndef __CINT__
  hName2p_TA_Texture_t	mName2Tex;
#endif

public:
  TA_TextureContainer(Text_t* n="TA_TextureContainer", Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  void ProcessDescFile(); // X{E} 7 MButt()
  _TA_Texture* FindTexture(const TString& name);

#include "TA_TextureContainer.h7"
  ClassDef(TA_TextureContainer, 1)
}; // endclass TA_TextureContainer


#endif
