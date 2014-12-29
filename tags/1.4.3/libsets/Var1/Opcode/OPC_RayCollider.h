//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains code for a ray collider.
 *	\file		OPC_RayCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_RAYCOLLIDER_H__
#define __OPC_RAYCOLLIDER_H__

class OPCODE_API CollisionFace
{
public:
  //! Constructor
  CollisionFace() {}
  //! Destructor
  ~CollisionFace() {}

  udword	mFaceID;	//!< Index of touched face
  float		mDistance;	//!< Distance from collider to hitpoint
  float		mU, mV;		//!< Impact barycentric coordinates
};

class OPCODE_API CollisionFaces : private Container
{
public:
  //! Constructor
  CollisionFaces() {}
  //! Destructor
  ~CollisionFaces() {}

  udword		GetNbFaces() const { return GetNbEntries()>>2; }
  const CollisionFace*	GetFaces()   const { return (const CollisionFace*)GetEntries(); }
  const CollisionFace&  operator[](int i) const { return GetFaces()[i]; }
  // MT eventually could add:
  // int  ClosestFaceIndex();
  // void SortFacesByDistance();

  void	Reset()		{ Container::Reset(); }

  void	AddFace(const CollisionFace& face) { Add(face.mFaceID).Add(face.mDistance).Add(face.mU).Add(face.mV); }
};

#ifdef OPC_RAYHIT_CALLBACK
//----------------------------------------------------------------------
/**
 *	User-callback, called by OPCODE to record a hit.
 *	\param		hit			[in] current hit
 *	\param		user_data	[in] user-defined data from SetCallback()
 */
//----------------------------------------------------------------------
typedef void	(*HitCallback)	(const CollisionFace& hit, void* user_data);
#endif

class OPCODE_API RayCollider : public Collider
{
public:
  // Constructor / Destructor
  RayCollider();
  virtual ~RayCollider();

  //----------------------------------------------------------------------
  /**
   *	Generic stabbing query for generic OPCODE models. After the call, access the results:
   *	- with GetContactStatus()
   *	- in the user-provided destination array
   *
   *	\param		world_ray	[in] stabbing ray in world space
   *	\param		model		[in] Opcode model to collide with
   *	\param		world		[in] model's world matrix, or 0
   *	\param		cache		[in] a possibly cached face index, or 0
   *	\return		true if success
   *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
   */
  //----------------------------------------------------------------------
  bool	Collide(const Ray& world_ray, const Model& model, const Matrix4x4* world=0, udword* cache=0);
  //
  bool	Collide(const Ray& world_ray, const AABBTree* tree, Container& box_indices);

  std::string CollideInfo(bool status, const Ray& ray);

  // Settings

#ifndef OPC_RAYHIT_CALLBACK
  //----------------------------------------------------------------------
  /**
   *	Settings: enable or disable "closest hit" mode.
   *	\param		flag		[in] true to report closest hit only
   *	\see		SetCulling(bool flag)
   *	\see		SetMaxDist(float max_dist)
   *	\see		SetDestination(StabbedFaces* sf)
   */
  //----------------------------------------------------------------------
  void	SetClosestHit(bool flag) { mClosestHit = flag; }
#endif

  //----------------------------------------------------------------------
  /**
   *	Settings: enable or disable backface culling.
   *	\param		flag		[in] true to enable backface culling
   *	\see		SetClosestHit(bool flag)
   *	\see		SetMaxDist(float max_dist)
   *	\see		SetDestination(StabbedFaces* sf)
   */
  //----------------------------------------------------------------------
  void	SetCulling(bool flag) { mCulling = flag; }

  //----------------------------------------------------------------------
  /**
   *	Settings: sets the higher distance bound.
   *	\param		max_dist	[in] higher distance bound. Default = maximal value, for ray queries (else segment)
   *	\see		SetClosestHit(bool flag)
   *	\see		SetCulling(bool flag)
   *	\see		SetDestination(StabbedFaces* sf)
   */
  //----------------------------------------------------------------------
  void	SetMaxDist(float max_dist=MAX_FLOAT)	{ mMaxDist = max_dist; }

#ifdef OPC_RAYHIT_CALLBACK
  void	SetHitCallback(HitCallback cb)		{ mHitCallback = cb; }
  void	SetUserData(void* user_data)		{ mUserData = user_data; }
#else
  //----------------------------------------------------------------------
  /**
   *	Settings: sets the destination array for stabbed faces.
   *	\param		cf			[in] destination array, filled during queries
   *	\see		SetClosestHit(bool flag)
   *	\see		SetCulling(bool flag)
   *	\see		SetMaxDist(float max_dist)
   */
  //----------------------------------------------------------------------
  void SetDestination(CollisionFaces* cf) { mStabbedFaces = cf; }
  CollisionFaces* GetDestination() { return mStabbedFaces; }
#endif
  // Stats
  //----------------------------------------------------------------------
  /**
   *	Stats: gets the number of Ray-BV overlap tests after a collision query.
   *	\see		GetNbRayPrimTests()
   *	\see		GetNbIntersections()
   *	\return		the number of Ray-BV tests performed during last query
   */
  //----------------------------------------------------------------------
  udword GetNbRayBVTests() const { return mNbRayBVTests; }

  //----------------------------------------------------------------------
  /**
   *	Stats: gets the number of Ray-Triangle overlap tests after a collision query.
   *	\see		GetNbRayBVTests()
   *	\see		GetNbIntersections()
   *	\return		the number of Ray-Triangle tests performed during last query
   */
  //----------------------------------------------------------------------
  udword GetNbRayPrimTests() const { return mNbRayPrimTests; }

  // In-out test
  //----------------------------------------------------------------------
  /**
   *	Stats: gets the number of intersection found after a collision query.
   *           Can be used for in/out tests.
   *	\see		GetNbRayBVTests()
   *	\see		GetNbRayPrimTests()
   *	\return		the number of valid intersections during last query
   */
  //----------------------------------------------------------------------
  udword GetNbIntersections() const { return mNbIntersections; }

  //----------------------------------------------------------------------
  /**
   *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
   *	\return		0 if everything is ok, else a string describing the problem
   */
  //----------------------------------------------------------------------
  override(Collider)	const char*	ValidateSettings();

protected:
  // Ray in local space
  Point		mOrigin;	//!< Ray origin
  Point		mDir;		//!< Ray direction (normalized)
  Point		mFDir;		//!< fabsf(mDir)
  Point		mData, mData2;

  // Stabbed faces
  CollisionFace	mStabbedFace;	//!< Current stabbed face
#ifdef OPC_RAYHIT_CALLBACK
  HitCallback	mHitCallback;	//!< Callback used to record a hit
  void*		mUserData;	//!< User-defined data
#else
  CollisionFaces* mStabbedFaces;//!< List of stabbed faces
#endif

  // Stats
  udword	mNbRayBVTests;	//!< Number of Ray-BV tests
  udword	mNbRayPrimTests;//!< Number of Ray-Primitive tests
  // In-out test
  udword	mNbIntersections;//!< Number of valid intersections
  // Dequantization coeffs
  Point		mCenterCoeff;
  Point		mExtentsCoeff;
  // Settings
  float		mMaxDist;	//!< Valid segment on the ray
#ifndef OPC_RAYHIT_CALLBACK
  bool		mClosestHit;	//!< Report closest hit only
#endif
  bool		mCulling;	//!< Stab culled faces or not

  // Internal methods
  void	_SegmentStab(const AABBCollisionNode* node);
  void	_SegmentStab(const AABBNoLeafNode* node);
  void	_SegmentStab(const AABBQuantizedNode* node);
  void	_SegmentStab(const AABBQuantizedNoLeafNode* node);
  void	_SegmentStab(const AABBTreeNode* node, Container& box_indices);
  void	_RayStab(const AABBCollisionNode* node);
  void	_RayStab(const AABBNoLeafNode* node);
  void	_RayStab(const AABBQuantizedNode* node);
  void	_RayStab(const AABBQuantizedNoLeafNode* node);
  void	_RayStab(const AABBTreeNode* node, Container& box_indices);

  // Overlap tests
  inline_ bool	RayAABBOverlap(const Point& center, const Point& extents);
  inline_ bool	SegmentAABBOverlap(const Point& center, const Point& extents);
  inline_ bool	RayTriOverlap(const Point& vert0, const Point& vert1, const Point& vert2);
  // Init methods
  bool		InitQuery(const Ray& world_ray, const Matrix4x4* world=0, udword* face_id=0);
};

#endif // __OPC_RAYCOLLIDER_H__
