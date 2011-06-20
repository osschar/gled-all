// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SRefCounted.h"

//__________________________________________________________________________
//
// A reference-counted stone.
// Note that the ref-count is not serialized -- so rebuild it in
// post-streaming function when necessary.
// When zero ref count is reached in DecRefCount(),
//   virtual void OnZeroRefCount();
// is called. Default action is to destroy the object.

ClassImp(SRefCounted);

SRefCounted::SRefCounted() : mRefCount(0)
{}

SRefCounted::~SRefCounted()
{}

void SRefCounted::IncRefCount()
{
  ++mRefCount;
}

void SRefCounted::DecRefCount()
{
  --mRefCount;
  if (mRefCount <= 0)
    OnZeroRefCount();
}

void SRefCounted::OnZeroRefCount()
{
  delete this;
}


//__________________________________________________________________________
//
// A reference-counted stone without virtual functions, all functions inline.
// Note that the ref-count is not serialized -- so rebuild it in
// post-streaming function when necessary.
// When zero ref count is reached in DecRefCount() the object is destructed.

ClassImp(SRefCountedNV);
