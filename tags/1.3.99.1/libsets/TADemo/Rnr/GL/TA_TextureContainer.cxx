#include <Glasses/TA_TextureContainer.h>

// This is utterly wrong.
// Must redo TextureContainer

void _TA_Texture::GL_Use() {
  if(bTexLoaded && !bTexInited) {
    glGenTextures(1, &fTexture);
    glBindTexture(GL_TEXTURE_2D, fTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fW, fH, 0,
		 GL_RGB, GL_UNSIGNED_BYTE, pData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    bTexInited = true; delete [] pData; pData = 0; bTexLoaded = false;
  }
  if(fTexture) {
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, fTexture);
  }
}

void _TA_Texture::GL_UnUse() {
  if(fTexture) { glDisable(GL_TEXTURE_2D); }
}
