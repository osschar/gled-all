// $Header$

#include "TA_SubUnit.h"
#include "TA_Unit.h"
#include "TA_SubUnit.c7"
#include "TA_TextureContainer.h"
#include <Glasses/ZQueen.h>

ClassImp(TA_SubUnit)

struct o3dVert { long x, y, z; };

struct o3dPrim {
  long Unknown_0;
  long NumberOfVertexIndexes;
  long Always_0;
  long OffsetToVertexIndexArray;
  long OffsetToTextureName;
  long Unknown_1;
  long Unknown_2;
  long Unknown_3;
};

struct o3dInfo {
  long VersionSignature;
  long NumberOfVertexes;
  long NumberOfPrimitives;
  long UnknownFlag;
  long XFromParent;
  long YFromParent;
  long ZFromParent;
  long OffsetToObjectName;
  long Always_0;
  long OffsetToVertexArray;
  long OffsetToPrimitiveArray;
  long OffsetToSiblingObject;
  long OffsetToChildObject;
};

void
_TA_Prim::calc_normal(float* v0, float* v1, float* v2) {
  float d0[3], d1[3];
  d0[0] = v1[0] - v0[0]; d1[0] = v2[0] - v0[0];
  d0[1] = v1[1] - v0[1]; d1[1] = v2[1] - v0[1];
  d0[2] = v1[2] - v0[2]; d1[2] = v2[2] - v0[2];
  fNormal[0] = d0[1]*d1[2] - d0[2]*d1[1];
  fNormal[1] = d0[2]*d1[0] - d0[0]*d1[2];
  fNormal[2] = d0[0]*d1[1] - d0[1]*d1[0];

  float s = sqrt(fNormal[0]*fNormal[0] + fNormal[1]*fNormal[1] + fNormal[2]*fNormal[2]);
  fNormal[0] /= s; fNormal[1] /= s; fNormal[2] /= s;
}

/**************************************************************************/

namespace {
  float SAFR = 1e-6;
}

void
TA_SubUnit::build(TA_Unit* u, o3dInfo* i, char* data)
{
  // Build from raw data.
  // Transform to Gled coordinate system: x-forward, y-left, z-up.

  mUnit = u; mInfo = i;
  mName = (char*)(data + mInfo->OffsetToObjectName);
  mXoff = -SAFR * mInfo->ZFromParent;
  mYoff = -SAFR * mInfo->XFromParent;
  mZoff =  SAFR * mInfo->YFromParent;
  MoveLF(1, mXoff); MoveLF(2, mYoff); MoveLF(3, mZoff);

  // Vertex Array
  mNVert = mInfo->NumberOfVertexes; pVert  = new float[3*mNVert];
  for(int i=0; i<mNVert; ++i) {
    int o = 3*i;
    o3dVert* v = (o3dVert*)(data + mInfo->OffsetToVertexArray + i*sizeof(o3dVert));
    pVert[o] = -v->z*SAFR; pVert[o+1] = -v->x*SAFR; pVert[o+2] = v->y*SAFR;
  }

  // Primitives & Normals
  mNPrim = mInfo->NumberOfPrimitives;
  o3dPrim* p_beg = (o3dPrim*)(data + mInfo->OffsetToPrimitiveArray);
  for(int i=0; i<mNPrim; ++i) {
    o3dPrim* p = p_beg + i;
    unsigned short* x = (unsigned short*)(data + p->OffsetToVertexIndexArray);
    char* tname = p->OffsetToTextureName ? data + p->OffsetToTextureName : 0;
    _TA_Prim ta_prim(tname, p->NumberOfVertexIndexes);
    if(p->NumberOfVertexIndexes >= 3) {
      ta_prim.calc_normal(&pVert[3*x[0]], &pVert[3*x[1]], &pVert[3*x[2]]);
    }
    for(int j=0; j<ta_prim.fN; ++j) ta_prim.fIndices[j] = x[j];
    mPrims.push_back(ta_prim);
  }

  // Family business
  if(mInfo->OffsetToChildObject) {
    o3dInfo* ci = (o3dInfo*)(data + mInfo->OffsetToChildObject);
    do {
      TA_SubUnit* child = new TA_SubUnit;
      mQueen->CheckIn(child);
      Add(child);
      child->build(mUnit, ci, data);
      ci = ci->OffsetToSiblingObject ?
	(o3dInfo*)(data + ci->OffsetToSiblingObject) : 0;
    } while(ci);
  }
  Stamp();
}
