// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MD2Object_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <TMath.h>

#include <GL/glew.h>

/**************************************************************************/

void MD2Object_GL_Rnr::_init()
{}

namespace {

MD2Object::Vector3 QNorms[] = {
  { -0.525731f,  0.000000f,  0.850651f },
  { -0.442863f,  0.238856f,  0.864188f },
  { -0.295242f,  0.000000f,  0.955423f },
  { -0.309017f,  0.500000f,  0.809017f },
  { -0.162460f,  0.262866f,  0.951056f },
  {  0.000000f,  0.000000f,  1.000000f },
  {  0.000000f,  0.850651f,  0.525731f },
  { -0.147621f,  0.716567f,  0.681718f },
  {  0.147621f,  0.716567f,  0.681718f },
  {  0.000000f,  0.525731f,  0.850651f },
  {  0.309017f,  0.500000f,  0.809017f },
  {  0.525731f,  0.000000f,  0.850651f },
  {  0.295242f,  0.000000f,  0.955423f },
  {  0.442863f,  0.238856f,  0.864188f },
  {  0.162460f,  0.262866f,  0.951056f },
  { -0.681718f,  0.147621f,  0.716567f },
  { -0.809017f,  0.309017f,  0.500000f },
  { -0.587785f,  0.425325f,  0.688191f },
  { -0.850651f,  0.525731f,  0.000000f },
  { -0.864188f,  0.442863f,  0.238856f },
  { -0.716567f,  0.681718f,  0.147621f },
  { -0.688191f,  0.587785f,  0.425325f },
  { -0.500000f,  0.809017f,  0.309017f },
  { -0.238856f,  0.864188f,  0.442863f },
  { -0.425325f,  0.688191f,  0.587785f },
  { -0.716567f,  0.681718f, -0.147621f },
  { -0.500000f,  0.809017f, -0.309017f },
  { -0.525731f,  0.850651f,  0.000000f },
  {  0.000000f,  0.850651f, -0.525731f },
  { -0.238856f,  0.864188f, -0.442863f },
  {  0.000000f,  0.955423f, -0.295242f },
  { -0.262866f,  0.951056f, -0.162460f },
  {  0.000000f,  1.000000f,  0.000000f },
  {  0.000000f,  0.955423f,  0.295242f },
  { -0.262866f,  0.951056f,  0.162460f },
  {  0.238856f,  0.864188f,  0.442863f },
  {  0.262866f,  0.951056f,  0.162460f },
  {  0.500000f,  0.809017f,  0.309017f },
  {  0.238856f,  0.864188f, -0.442863f },
  {  0.262866f,  0.951056f, -0.162460f },
  {  0.500000f,  0.809017f, -0.309017f },
  {  0.850651f,  0.525731f,  0.000000f },
  {  0.716567f,  0.681718f,  0.147621f },
  {  0.716567f,  0.681718f, -0.147621f },
  {  0.525731f,  0.850651f,  0.000000f },
  {  0.425325f,  0.688191f,  0.587785f },
  {  0.864188f,  0.442863f,  0.238856f },
  {  0.688191f,  0.587785f,  0.425325f },
  {  0.809017f,  0.309017f,  0.500000f },
  {  0.681718f,  0.147621f,  0.716567f },
  {  0.587785f,  0.425325f,  0.688191f },
  {  0.955423f,  0.295242f,  0.000000f },
  {  1.000000f,  0.000000f,  0.000000f },
  {  0.951056f,  0.162460f,  0.262866f },
  {  0.850651f, -0.525731f,  0.000000f },
  {  0.955423f, -0.295242f,  0.000000f },
  {  0.864188f, -0.442863f,  0.238856f },
  {  0.951056f, -0.162460f,  0.262866f },
  {  0.809017f, -0.309017f,  0.500000f },
  {  0.681718f, -0.147621f,  0.716567f },
  {  0.850651f,  0.000000f,  0.525731f },
  {  0.864188f,  0.442863f, -0.238856f },
  {  0.809017f,  0.309017f, -0.500000f },
  {  0.951056f,  0.162460f, -0.262866f },
  {  0.525731f,  0.000000f, -0.850651f },
  {  0.681718f,  0.147621f, -0.716567f },
  {  0.681718f, -0.147621f, -0.716567f },
  {  0.850651f,  0.000000f, -0.525731f },
  {  0.809017f, -0.309017f, -0.500000f },
  {  0.864188f, -0.442863f, -0.238856f },
  {  0.951056f, -0.162460f, -0.262866f },
  {  0.147621f,  0.716567f, -0.681718f },
  {  0.309017f,  0.500000f, -0.809017f },
  {  0.425325f,  0.688191f, -0.587785f },
  {  0.442863f,  0.238856f, -0.864188f },
  {  0.587785f,  0.425325f, -0.688191f },
  {  0.688191f,  0.587785f, -0.425325f },
  { -0.147621f,  0.716567f, -0.681718f },
  { -0.309017f,  0.500000f, -0.809017f },
  {  0.000000f,  0.525731f, -0.850651f },
  { -0.525731f,  0.000000f, -0.850651f },
  { -0.442863f,  0.238856f, -0.864188f },
  { -0.295242f,  0.000000f, -0.955423f },
  { -0.162460f,  0.262866f, -0.951056f },
  {  0.000000f,  0.000000f, -1.000000f },
  {  0.295242f,  0.000000f, -0.955423f },
  {  0.162460f,  0.262866f, -0.951056f },
  { -0.442863f, -0.238856f, -0.864188f },
  { -0.309017f, -0.500000f, -0.809017f },
  { -0.162460f, -0.262866f, -0.951056f },
  {  0.000000f, -0.850651f, -0.525731f },
  { -0.147621f, -0.716567f, -0.681718f },
  {  0.147621f, -0.716567f, -0.681718f },
  {  0.000000f, -0.525731f, -0.850651f },
  {  0.309017f, -0.500000f, -0.809017f },
  {  0.442863f, -0.238856f, -0.864188f },
  {  0.162460f, -0.262866f, -0.951056f },
  {  0.238856f, -0.864188f, -0.442863f },
  {  0.500000f, -0.809017f, -0.309017f },
  {  0.425325f, -0.688191f, -0.587785f },
  {  0.716567f, -0.681718f, -0.147621f },
  {  0.688191f, -0.587785f, -0.425325f },
  {  0.587785f, -0.425325f, -0.688191f },
  {  0.000000f, -0.955423f, -0.295242f },
  {  0.000000f, -1.000000f,  0.000000f },
  {  0.262866f, -0.951056f, -0.162460f },
  {  0.000000f, -0.850651f,  0.525731f },
  {  0.000000f, -0.955423f,  0.295242f },
  {  0.238856f, -0.864188f,  0.442863f },
  {  0.262866f, -0.951056f,  0.162460f },
  {  0.500000f, -0.809017f,  0.309017f },
  {  0.716567f, -0.681718f,  0.147621f },
  {  0.525731f, -0.850651f,  0.000000f },
  { -0.238856f, -0.864188f, -0.442863f },
  { -0.500000f, -0.809017f, -0.309017f },
  { -0.262866f, -0.951056f, -0.162460f },
  { -0.850651f, -0.525731f,  0.000000f },
  { -0.716567f, -0.681718f, -0.147621f },
  { -0.716567f, -0.681718f,  0.147621f },
  { -0.525731f, -0.850651f,  0.000000f },
  { -0.500000f, -0.809017f,  0.309017f },
  { -0.238856f, -0.864188f,  0.442863f },
  { -0.262866f, -0.951056f,  0.162460f },
  { -0.864188f, -0.442863f,  0.238856f },
  { -0.809017f, -0.309017f,  0.500000f },
  { -0.688191f, -0.587785f,  0.425325f },
  { -0.681718f, -0.147621f,  0.716567f },
  { -0.442863f, -0.238856f,  0.864188f },
  { -0.587785f, -0.425325f,  0.688191f },
  { -0.309017f, -0.500000f,  0.809017f },
  { -0.147621f, -0.716567f,  0.681718f },
  { -0.425325f, -0.688191f,  0.587785f },
  { -0.162460f, -0.262866f,  0.951056f },
  {  0.442863f, -0.238856f,  0.864188f },
  {  0.162460f, -0.262866f,  0.951056f },
  {  0.309017f, -0.500000f,  0.809017f },
  {  0.147621f, -0.716567f,  0.681718f },
  {  0.000000f, -0.525731f,  0.850651f },
  {  0.425325f, -0.688191f,  0.587785f },
  {  0.587785f, -0.425325f,  0.688191f },
  {  0.688191f, -0.587785f,  0.425325f },
  { -0.955423f,  0.295242f,  0.000000f },
  { -0.951056f,  0.162460f,  0.262866f },
  { -1.000000f,  0.000000f,  0.000000f },
  { -0.850651f,  0.000000f,  0.525731f },
  { -0.955423f, -0.295242f,  0.000000f },
  { -0.951056f, -0.162460f,  0.262866f },
  { -0.864188f,  0.442863f, -0.238856f },
  { -0.951056f,  0.162460f, -0.262866f },
  { -0.809017f,  0.309017f, -0.500000f },
  { -0.864188f, -0.442863f, -0.238856f },
  { -0.951056f, -0.162460f, -0.262866f },
  { -0.809017f, -0.309017f, -0.500000f },
  { -0.681718f,  0.147621f, -0.716567f },
  { -0.681718f, -0.147621f, -0.716567f },
  { -0.850651f,  0.000000f, -0.525731f },
  { -0.688191f,  0.587785f, -0.425325f },
  { -0.587785f,  0.425325f, -0.688191f },
  { -0.425325f,  0.688191f, -0.587785f },
  { -0.425325f, -0.688191f, -0.587785f },
  { -0.587785f, -0.425325f, -0.688191f },
  { -0.688191f, -0.587785f, -0.425325f }
};

};
/**************************************************************************/

void MD2Object_GL_Rnr::Draw(RnrDriver* rd)
{
  MD2Object& M = *mMD2Object;
  if(M.m_akVertex == 0)
    return;

  // Get Triangles & Texture coords
  const MD2Object::MD2Header&       rkHeader   = M.Header();
  const MD2Object::Vector3*         akVertices = M.Vertices();
  const Int_t*                      akNormals  = M.Normals();
  const MD2Object::MD2Triangle*     akTriangle = M.Triangles();
  const MD2Object::MD2TextureCoord* akTexture  = M.TextureCoords();

  float sfTime = M.mFrameTime;
  if(sfTime > rkHeader.iNumFrames - 1)
    sfTime = rkHeader.iNumFrames - 1;
  int iCurrFrame = (int) TMath::Floor(sfTime);
  int iNextFrame = iCurrFrame + 1;
  if(iNextFrame >= rkHeader.iNumFrames)
    iNextFrame = iCurrFrame;

  sfTime -= iCurrFrame;

  rd->GL()->Color(M.mColor);

  // Render triangles: W ~ vertex-weigts, T ~ normal-weights
  const float W0 = 1 - sfTime, T0 = M.mNormFac*W0;
  const float W1 = sfTime,     T1 = M.mNormFac*W1;

  const float SWF =  1.0 / rkHeader.iSkinWidth;
  const float SHF = -1.0 / rkHeader.iSkinHeight; // -1 ~ invert texture

  glBegin(GL_TRIANGLES);
  for (int iIndex = 0; iIndex < rkHeader.iNumTriangles; iIndex++) {

    int iLookUp1, iLookUp2;

    // Vertex 0
    iLookUp1 = akTriangle[iIndex].iVertexIndex[0]+iCurrFrame*rkHeader.iNumVertices;
    iLookUp2 = akTriangle[iIndex].iVertexIndex[0]+iNextFrame*rkHeader.iNumVertices;
    glTexCoord2f(SWF*akTexture[akTriangle[iIndex].iTextureIndex[0]].sU,
		 SHF*akTexture[akTriangle[iIndex].iTextureIndex[0]].sV);
    glNormal3f(T0*QNorms[akNormals[iLookUp1]].x + T1*QNorms[akNormals[iLookUp2]].x,
	       T0*QNorms[akNormals[iLookUp1]].y + T1*QNorms[akNormals[iLookUp2]].y,
	       T0*QNorms[akNormals[iLookUp1]].z + T1*QNorms[akNormals[iLookUp2]].z);
    glVertex3f(W0*akVertices[iLookUp1].x + W1*akVertices[iLookUp2].x,
	       W0*akVertices[iLookUp1].y + W1*akVertices[iLookUp2].y,
	       W0*akVertices[iLookUp1].z + W1*akVertices[iLookUp2].z);

    // Vertex 2 -- inversion !!!
    iLookUp1 = akTriangle[iIndex].iVertexIndex[2]+iCurrFrame*rkHeader.iNumVertices;
    iLookUp2 = akTriangle[iIndex].iVertexIndex[2]+iNextFrame*rkHeader.iNumVertices;
    glTexCoord2f(SWF*akTexture[akTriangle[iIndex].iTextureIndex[2]].sU,
		 SHF*akTexture[akTriangle[iIndex].iTextureIndex[2]].sV);
    glNormal3f(T0*QNorms[akNormals[iLookUp1]].x + T1*QNorms[akNormals[iLookUp2]].x,
	       T0*QNorms[akNormals[iLookUp1]].y + T1*QNorms[akNormals[iLookUp2]].y,
	       T0*QNorms[akNormals[iLookUp1]].z + T1*QNorms[akNormals[iLookUp2]].z);
    glVertex3f(W0*akVertices[iLookUp1].x + W1*akVertices[iLookUp2].x,
	       W0*akVertices[iLookUp1].y + W1*akVertices[iLookUp2].y,
	       W0*akVertices[iLookUp1].z + W1*akVertices[iLookUp2].z);

    // Vertex 1
    iLookUp1 = akTriangle[iIndex].iVertexIndex[1]+iCurrFrame*rkHeader.iNumVertices;
    iLookUp2 = akTriangle[iIndex].iVertexIndex[1]+iNextFrame*rkHeader.iNumVertices;
    glTexCoord2f(SWF*akTexture[akTriangle[iIndex].iTextureIndex[1]].sU,
		 SHF*akTexture[akTriangle[iIndex].iTextureIndex[1]].sV);
    glNormal3f(T0*QNorms[akNormals[iLookUp1]].x + T1*QNorms[akNormals[iLookUp2]].x,
	       T0*QNorms[akNormals[iLookUp1]].y + T1*QNorms[akNormals[iLookUp2]].y,
	       T0*QNorms[akNormals[iLookUp1]].z + T1*QNorms[akNormals[iLookUp2]].z);
    glVertex3f(W0*akVertices[iLookUp1].x + W1*akVertices[iLookUp2].x,
	       W0*akVertices[iLookUp1].y + W1*akVertices[iLookUp2].y,
	       W0*akVertices[iLookUp1].z + W1*akVertices[iLookUp2].z);
  }
  glEnd();
}
