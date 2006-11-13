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
 *	\file		OPC_VolumeCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_VOLUMECOLLIDER_H__
#define __OPC_VOLUMECOLLIDER_H__

struct OPCODE_API VolumeCache
{
  VolumeCache() : Model(0) {}
  ~VolumeCache() {}

  Container        TouchedPrimitives;  //!< Indices of touched primitives
  const BaseModel* Model;	       //!< Owner
};


class OPCODE_API VolumeCollider : public Collider
{
public:
  // Constructor / Destructor
  VolumeCollider();
  virtual ~VolumeCollider() = 0;

  // Collision report

  //----------------------------------------------------------------------
  /**
   *	Gets the number of touched primitives after a collision query.
   *	\see		GetContactStatus()
   *	\see		GetTouchedPrimitives()
   *	\return		the number of touched primitives
   */
  //----------------------------------------------------------------------
  udword GetNbTouchedPrimitives() const { return mTouchedPrimitives ? mTouchedPrimitives->GetNbEntries() : 0;	}

  //----------------------------------------------------------------------
  /**
   *	Gets the list of touched primitives after a collision query.
   *	\see		GetContactStatus()
   *	\see		GetNbTouchedPrimitives()
   *	\return		the list of touched primitives (primitive indices)
   */
  //----------------------------------------------------------------------
  const	udword*	GetTouchedPrimitives() const { return mTouchedPrimitives ? mTouchedPrimitives->GetEntries() : 0; }

  // Stats

  //----------------------------------------------------------------------
  /**
   *	Stats: gets the number of Volume-BV overlap tests after a collision query.
   *	\see		GetNbVolumePrimTests()
   *	\return		the number of Volume-BV tests performed during last query
   */
  //----------------------------------------------------------------------
  udword GetNbVolumeBVTests() const { return mNbVolumeBVTests; }

  //----------------------------------------------------------------------
  /**
   *	Stats: gets the number of Volume-Triangle overlap tests after a collision query.
   *	\see		GetNbVolumeBVTests()
   *	\return		the number of Volume-Triangle tests performed during last query
   */
  //----------------------------------------------------------------------
  udword GetNbVolumePrimTests() const { return mNbVolumePrimTests; }

  // Settings

  //----------------------------------------------------------------------
  /**
   *	Validates current settings. You should call this method after all the settings / callbacks have been defined for a collider.
   *	\return		0 if everything is ok, else a string describing the problem
   */
  //----------------------------------------------------------------------
  override(Collider)	const char*	ValidateSettings();

protected:
  // Touched primitives
  Container*	mTouchedPrimitives;	//!< List of touched primitives

  // Dequantization coeffs
  Point		mCenterCoeff;
  Point		mExtentsCoeff;
  // Stats
  udword	mNbVolumeBVTests;	//!< Number of Volume-BV tests
  udword	mNbVolumePrimTests;	//!< Number of Volume-Primitive tests

  // Internal methods
  void	_Dump(const AABBCollisionNode* node);
  void	_Dump(const AABBNoLeafNode* node);
  void	_Dump(const AABBQuantizedNode* node);
  void	_Dump(const AABBQuantizedNoLeafNode* node);

  //----------------------------------------------------------------------
  /**
   *	Initializes a query
   */
  //----------------------------------------------------------------------
  override(Collider) inline_	void	InitQuery()
  {
    // Reset stats & contact status
    mNbVolumeBVTests	= 0;
    mNbVolumePrimTests	= 0;
    Collider::InitQuery();
  }

  inline_ bool IsCacheValid(VolumeCache& cache)
  {
    // We're going to do a volume-vs-model query.
    if(cache.Model!=mCurrentModel)
      {
        // Cached list was for another model so we can't keep it
        // Keep track of new owner and reset cache
        cache.Model = mCurrentModel;
        return false;
      }
    else
      {
        // Same models, no problem
        return true;
      }
  }
};

#endif // __OPC_VOLUMECOLLIDER_H__
