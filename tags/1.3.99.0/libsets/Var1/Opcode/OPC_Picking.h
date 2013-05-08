//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains code to perform "picking".
 *	\file		OPC_Picking.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_PICKING_H__
#define __OPC_PICKING_H__

#ifdef OPC_RAYHIT_CALLBACK

enum CullMode
  {
    CULLMODE_NONE = 0,
    CULLMODE_CW   = 1,
    CULLMODE_CCW  = 2
  };

typedef CullMode (*CullModeCallback)(udword triangle_index, void* user_data);

OPCODE_API bool SetupAllHits(RayCollider& collider, CollisionFaces& contacts);
OPCODE_API bool SetupClosestHit(RayCollider& collider, CollisionFace& closest_contact);
OPCODE_API bool SetupShadowFeeler(RayCollider& collider);
OPCODE_API bool SetupInOutTest(RayCollider& collider);

OPCODE_API bool Picking(CollisionFace& picked_face, const Ray& world_ray,
                        const Model& model, const Matrix4x4* world,
                        float min_dist, float max_dist, const Point& view_point,
                        CullModeCallback callback, void* user_data);
#endif

#endif //__OPC_PICKING_H__
