// Opcode 1.1: ray-AABB overlap tests based on Woo's code
// Opcode 1.2: ray-AABB overlap tests based on the separating axis theorem
//
// The point of intersection is not computed anymore. The distance to impact is not needed anymore
// since we now have two different queries for segments or rays.

//----------------------------------------------------------------------
/**
 *	Computes a segment-AABB overlap test using the separating axis theorem. Segment is cached within the class.
 *	\param		center	[in] AABB center
 *	\param		extents	[in] AABB extents
 *	\return		true on overlap
 */
//----------------------------------------------------------------------
inline_ bool RayCollider::SegmentAABBOverlap(const Point& center, const Point& extents)
{
  // Stats
  mNbRayBVTests++;

  float Dx = mData2.x - center.x;  if(fabsf(Dx) > extents.x + mFDir.x)  return false;
  float Dy = mData2.y - center.y;  if(fabsf(Dy) > extents.y + mFDir.y)  return false;
  float Dz = mData2.z - center.z;  if(fabsf(Dz) > extents.z + mFDir.z)  return false;

  float f;
  f = mData.y * Dz - mData.z * Dy;  if(fabsf(f) > extents.y*mFDir.z + extents.z*mFDir.y)  return false;
  f = mData.z * Dx - mData.x * Dz;  if(fabsf(f) > extents.x*mFDir.z + extents.z*mFDir.x)  return false;
  f = mData.x * Dy - mData.y * Dx;  if(fabsf(f) > extents.x*mFDir.y + extents.y*mFDir.x)  return false;

  return true;
}

//----------------------------------------------------------------------
/**
 *	Computes a ray-AABB overlap test using the separating axis theorem. Ray is cached within the class.
 *	\param		center	[in] AABB center
 *	\param		extents	[in] AABB extents
 *	\return		true on overlap
 */
//----------------------------------------------------------------------
inline_ bool RayCollider::RayAABBOverlap(const Point& center, const Point& extents)
{
  // Stats
  mNbRayBVTests++;

  // float Dx = mOrigin.x - center.x;  if(fabsf(Dx) > extents.x && Dx*mDir.x>=0.0f)  return false;
  // float Dy = mOrigin.y - center.y;  if(fabsf(Dy) > extents.y && Dy*mDir.y>=0.0f)  return false;
  // float Dz = mOrigin.z - center.z;  if(fabsf(Dz) > extents.z && Dz*mDir.z>=0.0f)  return false;

  float Dx = mOrigin.x - center.x;  if(GREATER(Dx, extents.x) && Dx*mDir.x>=0.0f)  return false;
  float Dy = mOrigin.y - center.y;  if(GREATER(Dy, extents.y) && Dy*mDir.y>=0.0f)  return false;
  float Dz = mOrigin.z - center.z;  if(GREATER(Dz, extents.z) && Dz*mDir.z>=0.0f)  return false;

  // float Dx = mOrigin.x - center.x;  if(GREATER(Dx, extents.x) && ((SIR(Dx)-1)^SIR(mDir.x))>=0.0f)  return false;
  // float Dy = mOrigin.y - center.y;  if(GREATER(Dy, extents.y) && ((SIR(Dy)-1)^SIR(mDir.y))>=0.0f)  return false;
  // float Dz = mOrigin.z - center.z;  if(GREATER(Dz, extents.z) && ((SIR(Dz)-1)^SIR(mDir.z))>=0.0f)  return false;

  float f;
  f = mDir.y * Dz - mDir.z * Dy;  if(fabsf(f) > extents.y*mFDir.z + extents.z*mFDir.y)  return false;
  f = mDir.z * Dx - mDir.x * Dz;  if(fabsf(f) > extents.x*mFDir.z + extents.z*mFDir.x)  return false;
  f = mDir.x * Dy - mDir.y * Dx;  if(fabsf(f) > extents.x*mFDir.y + extents.y*mFDir.x)  return false;

  return true;
}
