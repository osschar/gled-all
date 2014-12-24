//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains code for OPCODE models.
 *	\file		OPC_Model.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_MODEL_H__
#define __OPC_MODEL_H__

class OPCODE_API Model : public BaseModel
{
public:
  // Constructor/Destructor
  Model();
  virtual ~Model();

  //----------------------------------------------------------------------
  /**
   *	Builds a collision model.
   *	\param		create		[in] model creation structure
   *	\return		true if success
   */
  //----------------------------------------------------------------------
  override(BaseModel) bool Build(const OPCODECREATE& create);

#ifdef __MESHMERIZER_H__
  //----------------------------------------------------------------------
  /**
   *	Gets the collision hull.
   *	\return		the collision hull if it exists
   */
  //----------------------------------------------------------------------
  const	CollisionHull* GetHull() const { return mHull; }
#endif // __MESHMERIZER_H__

  //----------------------------------------------------------------------
  /**
   *	Gets the number of bytes used by the tree.
   *	\return		amount of bytes used
   */
  //----------------------------------------------------------------------
  override(BaseModel)	udword	GetUsedBytes()	const;

private:
#ifdef __MESHMERIZER_H__
  CollisionHull*	mHull;	//!< Possible convex hull
#endif // __MESHMERIZER_H__
  // Internal methods
  void	Release();
};

#endif //__OPC_MODEL_H__
