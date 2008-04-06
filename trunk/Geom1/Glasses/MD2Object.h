// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_MD2Object_H
#define Geom1_MD2Object_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class MD2Object : public ZNode
{
  MAC_RNR_FRIENDS(MD2Object);

public:
  struct MD2Header
  {
    int iMagic;
    int iVersion;
    int iSkinWidth;
    int iSkinHeight;
    int iFrameSize;
    int iNumSkins;
    int iNumVertices;
    int iNumTexCoords;
    int iNumTriangles;
    int iNumGlCommands;
    int iNumFrames;
    int iOffsetSkins;
    int iOffsetTexCoords;
    int iOffsetTriangles;
    int iOffsetFrames;
    int iOffsetGlCommands;
    int iOffsetEnd;
  };

  struct MD2Triangle
  {
    short iVertexIndex[3];
    short iTextureIndex[3];
  };

  struct MD2Vertex
  {
    unsigned char abVertex[3];
    unsigned char abLightNormalIndex;
  };

  struct MD2TextureCoord
  {
    short sU,sV;
  };

  struct MD2CommandVertex
  {
    float fU;
    float fV;
    int iVertexIndex;
  };

  struct MD2Frame
  {
    float afScale[3];
    float afTranslate[3];
    char  acName[16];
    MD2Vertex akVertices[1];
  };

  struct Vector3 {
    float x,y,z;
  };

private:
  void _init();
  void _clear_data();

protected:
  TString	mFile;		// X{GS}   7 Filor()
  Float_t       mFrameTime;     // X{GS}   7 Value(-range=>[0,1000,1,100], -join=>1)
  Int_t         mMaxFrame;      // X{G}    7 ValOut()
  Float_t       mNormFac;       // X{GS}   7 Value(-range=>[-10,10,1,100], -join=>1)
  ZColor	mColor;		// X{PGS}  7 ColorButt()

public:
  MD2Object(const Text_t* n="MD2Object", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~MD2Object() {}

  void Load(); // X{E} 7 MCWButt()

  const Vector3*          Vertices()      const { return m_akVertex; }
  const Int_t*            Normals()       const { return m_akNormal; }
  const MD2Header&        Header()        const { return m_kHeader; }
  const MD2Triangle*      Triangles()     const { return m_akTriangle; }
  const MD2TextureCoord*  TextureCoords() const { return m_akTextureCoords; }

  char      *m_akFrames; //!
  MD2Header  m_kHeader;  //!

  //indices

  Vector3*           m_akVertex;         //!
  Int_t*             m_akNormal;         //!
  MD2Triangle*       m_akTriangle;       //!
  MD2TextureCoord*   m_akTextureCoords;  //!
  MD2CommandVertex** m_akTriangleFans;   //!
  MD2CommandVertex** m_akTriangleStrips; //!

  UInt_t m_uiNumFans;   //!
  UInt_t m_uiNumStrips; //!

#include "MD2Object.h7"
  ClassDef(MD2Object, 1);
}; // endclass MD2Object


#endif
