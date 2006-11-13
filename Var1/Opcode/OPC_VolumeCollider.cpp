//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains base volume collider class.
 *	\file		OPC_VolumeCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains the abstract class for volume colliders.
 *
 *	\class		VolumeCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		June, 2, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

using namespace Opcode;

//----------------------------------------------------------------------
/**
 *	Constructor.
 */
//----------------------------------------------------------------------
VolumeCollider::VolumeCollider() :
  mTouchedPrimitives	(0),
  mNbVolumeBVTests	(0),
  mNbVolumePrimTests	(0)
{}

//----------------------------------------------------------------------
/**
 *	Destructor.
 */
//----------------------------------------------------------------------
VolumeCollider::~VolumeCollider()
{
  mTouchedPrimitives = 0;
}

//----------------------------------------------------------------------
/**
 *  Validates current settings. You should call this method after all
 *  the settings / callbacks have been defined for a collider.
 *  \return	0 if everything is ok, else a string describing the problem
 */
//----------------------------------------------------------------------
const char* VolumeCollider::ValidateSettings()
{
  return 0;
}

// Pretty dumb way to dump - to do better - one day...

#define IMPLEMENT_NOLEAFDUMP(type)                      \
  void VolumeCollider::_Dump(const type* node)          \
  {                                                     \
    if(node->HasPosLeaf())  mTouchedPrimitives->Add(node->GetPosPrimitive()); \
    else		   _Dump(node->GetPos());       \
                                                        \
    if(ContactFound()) return;                          \
                                                        \
    if(node->HasNegLeaf())  mTouchedPrimitives->Add(node->GetNegPrimitive()); \
    else		   _Dump(node->GetNeg());       \
  }

#define IMPLEMENT_LEAFDUMP(type)                        \
  void VolumeCollider::_Dump(const type* node)          \
  {                                                     \
    if(node->IsLeaf())                                  \
      {                                                 \
        mTouchedPrimitives->Add(node->GetPrimitive());	\
      }                                                 \
    else                                                \
      {                                                 \
        _Dump(node->GetPos());                          \
                                                        \
        if(ContactFound()) return;                      \
                                                        \
        _Dump(node->GetNeg());                          \
      }                                                 \
  }

IMPLEMENT_NOLEAFDUMP(AABBNoLeafNode)
IMPLEMENT_NOLEAFDUMP(AABBQuantizedNoLeafNode)

IMPLEMENT_LEAFDUMP(AABBCollisionNode)
IMPLEMENT_LEAFDUMP(AABBQuantizedNode)
