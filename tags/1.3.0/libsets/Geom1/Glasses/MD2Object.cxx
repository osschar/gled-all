// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MD2Object
//
// Quake2 MD2 object -- minimal implementation.
// Snached from: http://www.moelhave.dk/2002/06/quake-2-md2-model-loader/
//
// Issues:
// 1. Triangles are back-facing ("fixed" in rendering).
// 2. Some models have inverted normals (fixed via mNormFac variable, set to -1).
//
//
// Should check some more models

#include "MD2Object.h"
#include "MD2Object.c7"

#include <fstream>

ClassImp(MD2Object);

/**************************************************************************/

void MD2Object::_init()
{
  mFrameTime = 0;
  mMaxFrame  = 0;
  mNormFac   = 1;

  m_akFrames = 0;
  m_akVertex = 0;
  m_akNormal = 0;
  m_akTriangle = 0;
  m_akTextureCoords = 0;
}


void MD2Object::_clear_data()
{
  delete m_akFrames; m_akFrames = 0;
  delete m_akVertex; m_akVertex = 0;
  delete m_akNormal; m_akNormal = 0;
  delete m_akTriangle; m_akTriangle = 0;
  delete m_akTextureCoords; m_akTextureCoords = 0;
}

/**************************************************************************/

void MD2Object::Load()
{
  // Loads the file given as parameter

  static const Exc_t _eh("MD2Object::Load ");

  //Load file

  std::ifstream kFile;
  kFile.open(mFile, std::ios::binary | std::ios::in);
  if (!kFile) {
    kFile.close();
    throw(_eh + "failed opening the file.");
  }

  //Loading header

  kFile.read((char *)&m_kHeader, sizeof(MD2Header));

  //Check magic number

  if (m_kHeader.iMagic != 0x32504449 ||m_kHeader.iVersion != 8) {
    kFile.close();
    throw(_eh + "wrong magic number.");
  }

  _clear_data();

  //Load the frames

  kFile.seekg(m_kHeader.iOffsetFrames, std::ios::beg); //seek to start

  m_akFrames = new char[m_kHeader.iFrameSize * m_kHeader.iNumFrames];
  kFile.read((char *)m_akFrames, m_kHeader.iFrameSize * m_kHeader.iNumFrames);

  //Read in to vertex array

  m_akVertex = new Vector3[m_kHeader.iNumFrames*m_kHeader.iNumVertices];
  m_akNormal = new Int_t  [m_kHeader.iNumFrames*m_kHeader.iNumVertices];
  for (int i = 0; i < m_kHeader.iNumFrames; i++) {

      for (int j = 0; j < m_kHeader.iNumVertices; j++) {

	  int iVertexIndex = i*m_kHeader.iNumVertices + j;
	  int iFrameIndex  = i*m_kHeader.iFrameSize;
	  m_akVertex[iVertexIndex].x = (float)(((MD2Frame*)&m_akFrames[iFrameIndex])->akVertices[j].abVertex[0]);
	  m_akVertex[iVertexIndex].y = (float)(((MD2Frame*)&m_akFrames[iFrameIndex])->akVertices[j].abVertex[1]);
	  m_akVertex[iVertexIndex].z = (float)(((MD2Frame*)&m_akFrames[iFrameIndex])->akVertices[j].abVertex[2]);

	  m_akVertex[iVertexIndex].x *= ((MD2Frame*)&m_akFrames[iFrameIndex])->afScale[0];
	  m_akVertex[iVertexIndex].y *= ((MD2Frame*)&m_akFrames[iFrameIndex])->afScale[1];
	  m_akVertex[iVertexIndex].z *= ((MD2Frame*)&m_akFrames[iFrameIndex])->afScale[2];

	  //scale them:

	  m_akVertex[iVertexIndex].x += ((MD2Frame*)&m_akFrames[iFrameIndex])->afTranslate[0];
	  m_akVertex[iVertexIndex].y += ((MD2Frame*)&m_akFrames[iFrameIndex])->afTranslate[1];
	  m_akVertex[iVertexIndex].z += ((MD2Frame*)&m_akFrames[iFrameIndex])->afTranslate[2];

	  m_akNormal[iVertexIndex] = ((MD2Frame*)&m_akFrames[iFrameIndex])->akVertices[j].abLightNormalIndex;
      }

  }

  //Read triangles

  m_akTriangle = new MD2Triangle[m_kHeader.iNumTriangles];
  kFile.seekg(m_kHeader.iOffsetTriangles, std::ios::beg);
  kFile.read((char *)m_akTriangle, m_kHeader.iNumTriangles * sizeof(MD2Triangle));


  //Read texture coordinates

  m_akTextureCoords = new MD2TextureCoord[m_kHeader.iNumTexCoords];
  kFile.seekg(m_kHeader.iOffsetTexCoords, std::ios::beg);
  kFile.read((char *)m_akTextureCoords, sizeof(MD2TextureCoord)*m_kHeader.iNumTexCoords);


  //Build bounding volume
  // m_kModelBound.ComputeFromData(m_akVertex, m_kHeader.iNumVertices);

  kFile.close();

  mMaxFrame = m_kHeader.iNumFrames;
  Stamp(FID());
}

/**************************************************************************/
