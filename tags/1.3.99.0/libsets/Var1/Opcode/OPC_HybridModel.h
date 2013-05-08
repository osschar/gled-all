//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains code for hybrid models.
 *	\file		OPC_HybridModel.h
 *	\author		Pierre Terdiman
 *	\date		May, 18, 2003
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_HYBRIDMODEL_H__
#define __OPC_HYBRIDMODEL_H__

//! Leaf descriptor
struct LeafTriangles
{
  udword	Data;		//!< Packed data

  //----------------------------------------------------------------------
  /**
   *	Gets number of triangles in the leaf.
   *	\return		number of triangles N, with 0 < N <= 16
   */
  //----------------------------------------------------------------------
  udword  GetNbTriangles() const { return (Data & 15)+1; }

  //----------------------------------------------------------------------
  /**
   *	Gets triangle index for this leaf. Indexed model's array of indices
   *    retrieved with HybridModel::GetIndices()
   *	\return		triangle index
   */
  //----------------------------------------------------------------------
  udword GetTriangleIndex() const         { return Data>>4; }
  void   SetData(udword nb, udword index) { ASSERT(nb>0 && nb<=16); nb--; Data = (index<<4)|(nb&15); }
};



class OPCODE_API HybridModel : public BaseModel
{
public:
  // Constructor/Destructor
  HybridModel();
  virtual ~HybridModel();

  //----------------------------------------------------------------------
  /**
   *	Builds a collision model.
   *	\param		create		[in] model creation structure
   *	\return		true if success
   */
  //----------------------------------------------------------------------
  override(BaseModel)	bool	Build(const OPCODECREATE& create);

  //----------------------------------------------------------------------
  /**
   *	Gets the number of bytes used by the tree.
   *	\return		amount of bytes used
   */
  //----------------------------------------------------------------------
  override(BaseModel)	udword	GetUsedBytes() const;

  //----------------------------------------------------------------------
  /**
   *	Refits the collision model. This can be used to handle dynamic meshes.
   *    Usage is:
   *	1. modify your mesh vertices (keep the topology constant!)
   *	2. refit the tree (call this method)
   *	\return		true if success
   */
  //----------------------------------------------------------------------
  override(BaseModel) bool Refit();

  //----------------------------------------------------------------------
  /**
   *	Gets array of triangles.
   *	\return		array of triangles
   */
  //----------------------------------------------------------------------
  const LeafTriangles* GetLeafTriangles() const { return mTriangles; }

  //----------------------------------------------------------------------
  /**
   *	Gets array of indices.
   *	\return		array of indices
   */
  //----------------------------------------------------------------------
  const udword* GetIndices() const { return mIndices; }

private:
  udword		mNbLeaves;	//!< Number of leaf nodes in the model
  LeafTriangles*	mTriangles;	//!< Array of mNbLeaves leaf descriptors
  udword		mNbPrimitives;	//!< Number of primitives in the model
  udword*		mIndices;	//!< Array of primitive indices

  // Internal methods
  void	Release();
};

#endif // __OPC_HYBRIDMODEL_H__
