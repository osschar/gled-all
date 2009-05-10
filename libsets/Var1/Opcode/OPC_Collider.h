//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains base collider class.
 *	\file		OPC_Collider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Include Guard
#ifndef __OPC_COLLIDER_H__
#define __OPC_COLLIDER_H__

enum CollisionFlag
  {
    OPC_FIRST_CONTACT      = (1<<0), //!< Report all contacts (false) or only first one (true)
    OPC_TEMPORAL_COHERENCE = (1<<1), //!< Use temporal coherence or not
    OPC_CONTACT            = (1<<2), //!< Final contact status after a collision query
    OPC_TEMPORAL_HIT       = (1<<3), //!< There has been an early exit due to temporal coherence
    OPC_NO_PRIMITIVE_TESTS = (1<<4), //!< Keep or discard primitive-bv tests in leaf nodes (volume-mesh queries)

    OPC_CONTACT_FOUND      = OPC_FIRST_CONTACT | OPC_CONTACT,
    OPC_TEMPORAL_CONTACT   = OPC_TEMPORAL_HIT | OPC_CONTACT,

    OPC_FORCE_DWORD        = 0x7fffffff
  };

class OPCODE_API Collider
{
public:
  // Constructor / Destructor
  Collider();
  virtual ~Collider();

  // Collision report

  //----------------------------------------------------------------------
  /**
   *	Gets the last collision status after a collision query.
   *	\return		true if a collision occured
   */
  //----------------------------------------------------------------------
  bool	GetContactStatus() const { return mFlags & OPC_CONTACT; }

  //----------------------------------------------------------------------
  /**
   *	Gets the "first contact" mode.
   *	\return		true if "first contact" mode is on
   */
  //----------------------------------------------------------------------
  bool	FirstContactEnabled() const { return mFlags & OPC_FIRST_CONTACT; }

  //----------------------------------------------------------------------
  /**
   *	Gets the temporal coherence mode.
   *	\return		true if temporal coherence is on
   */
  //----------------------------------------------------------------------
  bool	TemporalCoherenceEnabled() const { return mFlags & OPC_TEMPORAL_COHERENCE; }

  //----------------------------------------------------------------------
  /**
   *	Checks a first contact has already been found.
   *	\return		true if a first contact has been found and we can stop a query
   */
  //----------------------------------------------------------------------
  bool	ContactFound() const { return (mFlags&OPC_CONTACT_FOUND)==OPC_CONTACT_FOUND; }

  //----------------------------------------------------------------------
  /**
   *	Checks there's been an early exit due to temporal coherence;
   *	\return		true if a temporal hit has occured
   */
  //----------------------------------------------------------------------
  bool	TemporalHit() const { return mFlags & OPC_TEMPORAL_HIT; }

  //----------------------------------------------------------------------
  /**
   *	Checks primitive tests are enabled;
   *	\return		true if primitive tests must be skipped
   */
  //----------------------------------------------------------------------
  bool	SkipPrimitiveTests() const { return mFlags & OPC_NO_PRIMITIVE_TESTS; }

  // Settings

  //----------------------------------------------------------------------
  /**
   *	Reports all contacts (false) or first contact only (true)
   *	\param		flag		[in] true for first contact, false for all contacts
   *	\see		SetTemporalCoherence(bool flag)
   *	\see		ValidateSettings()
   */
  //----------------------------------------------------------------------
  void	SetFirstContact(bool flag)
  {
    if(flag)	mFlags |=  OPC_FIRST_CONTACT;
    else	mFlags &= ~OPC_FIRST_CONTACT;
  }

  //----------------------------------------------------------------------
  /**
   *	Enable/disable temporal coherence.
   *	\param		flag		[in] true to enable temporal coherence, false to discard it
   *	\see		SetFirstContact(bool flag)
   *	\see		ValidateSettings()
   */
  //----------------------------------------------------------------------
  void	SetTemporalCoherence(bool flag)
  {
    if(flag)	mFlags |=  OPC_TEMPORAL_COHERENCE;
    else	mFlags &= ~OPC_TEMPORAL_COHERENCE;
  }

  //----------------------------------------------------------------------
  /**
   *	Enable/disable primitive tests.
   *	\param		flag		[in] true to enable primitive tests, false to discard them
   */
  //----------------------------------------------------------------------
  void	SetPrimitiveTests(bool flag)
  {
    if(!flag)	mFlags |=  OPC_NO_PRIMITIVE_TESTS;
    else	mFlags &= ~OPC_NO_PRIMITIVE_TESTS;
  }

  //----------------------------------------------------------------------
  /**
   *	Validates current settings. You should call this method after all the settings / callbacks have been defined for a collider.
   *	\return		0 if everything is ok, else a string describing the problem
   */
  //----------------------------------------------------------------------
  virtual const char* ValidateSettings() = 0;

protected:
  udword		mFlags;		//!< Bit flags
  const	BaseModel*	mCurrentModel;	//!< Current model for collision query (owner of touched faces)
  // User mesh interface
  const	MeshInterface*	mIMesh;		//!< User-defined mesh interface

  // Internal methods
  //----------------------------------------------------------------------
  /**
   *	Setups current collision model
   *	\param		model	[in] current collision model
   *	\return		true if success
   */
  //----------------------------------------------------------------------
  bool	Setup(const BaseModel* model)
  {
    // Keep track of current model
    mCurrentModel = model;
    if(!mCurrentModel)	return false;

    mIMesh = model->GetMeshInterface();
    return mIMesh!=0;
  }

  //----------------------------------------------------------------------
  /**
   *	Initializes a query
   */
  //----------------------------------------------------------------------
  virtual void	InitQuery() { mFlags &= ~OPC_TEMPORAL_CONTACT; }
};

#endif // __OPC_COLLIDER_H__
