//----------------------------------------------------------------------
/**
 *	Contains a simple container class.
 *	\file		IceContainer.cpp
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains a list of 32-bits values.
 *	Use this class when you need to store an unknown number of values. The list is automatically
 *	resized and can contains 32-bits entities (dwords or floats)
 *
 *	\class		Container
 *	\author		Pierre Terdiman
 *	\version	1.0
 *	\date		08.15.98
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

using namespace Opcode;

// Static members
#ifdef CONTAINER_STATS
udword Container::mNbContainers = 0;
udword Container::mUsedRam = 0;
#endif

//----------------------------------------------------------------------
/**
 *	Constructor. No entries allocated there.
 */
//----------------------------------------------------------------------
Container::Container() : mMaxNbEntries(0), mCurNbEntries(0), mEntries(0), mGrowthFactor(2.0f)
{
#ifdef CONTAINER_STATS
  mNbContainers++;
  mUsedRam+=sizeof(Container);
#endif
}

//----------------------------------------------------------------------
/**
 *	Constructor. Also allocates a given number of entries.
 */
//----------------------------------------------------------------------
Container::Container(udword size, float growth_factor) : mMaxNbEntries(0), mCurNbEntries(0), mEntries(0), mGrowthFactor(growth_factor)
{
#ifdef CONTAINER_STATS
  mNbContainers++;
  mUsedRam+=sizeof(Container);
#endif
  SetSize(size);
}

//----------------------------------------------------------------------
/**
 *	Copy constructor.
 */
//----------------------------------------------------------------------
Container::Container(const Container& object) : mMaxNbEntries(0), mCurNbEntries(0), mEntries(0), mGrowthFactor(2.0f)
{
#ifdef CONTAINER_STATS
  mNbContainers++;
  mUsedRam+=sizeof(Container);
#endif
  *this = object;
}

//----------------------------------------------------------------------
/**
 *	Destructor.	Frees everything and leaves.
 */
//----------------------------------------------------------------------
Container::~Container()
{
  Empty();
#ifdef CONTAINER_STATS
  mNbContainers--;
  mUsedRam-=GetUsedRam();
#endif
}

//----------------------------------------------------------------------
/**
 *	Clears the container. All stored values are deleted, and it frees used ram.
 *	\see		Reset()
 *	\return		Self-Reference
 */
//----------------------------------------------------------------------
Container& Container::Empty()
{
#ifdef CONTAINER_STATS
  mUsedRam-=mMaxNbEntries*sizeof(udword);
#endif
  DELETEARRAY(mEntries);
  mCurNbEntries = mMaxNbEntries = 0;
  return *this;
}

//----------------------------------------------------------------------
/**
 *	Resizes the container.
 *	\param		needed	[in] assume the container can be added at least "needed" values
 *	\return		true if success.
 */
//----------------------------------------------------------------------
bool Container::Resize(udword needed)
{
#ifdef CONTAINER_STATS
  // Subtract previous amount of bytes
  mUsedRam-=mMaxNbEntries*sizeof(udword);
#endif

  // Get more entries
  mMaxNbEntries = mMaxNbEntries ? udword(float(mMaxNbEntries)*mGrowthFactor) : 2;	// Default nb Entries = 2
  if(mMaxNbEntries<mCurNbEntries + needed)	mMaxNbEntries = mCurNbEntries + needed;

  // Get some bytes for new entries
  udword*	NewEntries = new udword[mMaxNbEntries];
  CHECKALLOC(NewEntries);

#ifdef CONTAINER_STATS
  // Add current amount of bytes
  mUsedRam+=mMaxNbEntries*sizeof(udword);
#endif

  // Copy old data if needed
  if(mCurNbEntries)	CopyMemory(NewEntries, mEntries, mCurNbEntries*sizeof(udword));

  // Delete old data
  DELETEARRAY(mEntries);

  // Assign new pointer
  mEntries = NewEntries;

  return true;
}

//----------------------------------------------------------------------
/**
 *	Sets the initial size of the container. If it already contains something, it's discarded.
 *	\param		nb		[in] Number of entries
 *	\return		true if success
 */
//----------------------------------------------------------------------
bool Container::SetSize(udword nb)
{
  // Make sure it's empty
  Empty();

  // Checkings
  if(!nb)	return false;

  // Initialize for nb entries
  mMaxNbEntries = nb;

  // Get some bytes for new entries
  mEntries = new udword[mMaxNbEntries];
  CHECKALLOC(mEntries);

#ifdef CONTAINER_STATS
  // Add current amount of bytes
  mUsedRam+=mMaxNbEntries*sizeof(udword);
#endif
  return true;
}

//----------------------------------------------------------------------
/**
 *	Refits the container and get rid of unused bytes.
 *	\return		true if success
 */
//----------------------------------------------------------------------
bool Container::Refit()
{
#ifdef CONTAINER_STATS
  // Subtract previous amount of bytes
  mUsedRam-=mMaxNbEntries*sizeof(udword);
#endif

  // Get just enough entries
  mMaxNbEntries = mCurNbEntries;
  if(!mMaxNbEntries)	return false;

  // Get just enough bytes
  udword*	NewEntries = new udword[mMaxNbEntries];
  CHECKALLOC(NewEntries);

#ifdef CONTAINER_STATS
  // Add current amount of bytes
  mUsedRam+=mMaxNbEntries*sizeof(udword);
#endif

  // Copy old data
  CopyMemory(NewEntries, mEntries, mCurNbEntries*sizeof(udword));

  // Delete old data
  DELETEARRAY(mEntries);

  // Assign new pointer
  mEntries = NewEntries;

  return true;
}

//----------------------------------------------------------------------
/**
 *	Checks whether the container already contains a given value.
 *	\param		entry			[in] the value to look for in the container
 *	\param		location		[out] a possible pointer to store the entry location
 *	\see		Add(udword entry)
 *	\see		Add(float entry)
 *	\see		Empty()
 *	\return		true if the value has been found in the container, else false.
 */
//----------------------------------------------------------------------
bool Container::Contains(udword entry, udword* location) const
{
  // Look for the entry
  for(udword i=0;i<mCurNbEntries;i++)
    {
      if(mEntries[i]==entry)
        {
          if(location)	*location = i;
          return true;
        }
    }
  return false;
}

//----------------------------------------------------------------------
/**
 *	Deletes an entry. If the container contains such an entry, it's removed.
 *	\param		entry		[in] the value to delete.
 *	\return		true if the value has been found in the container, else false.
 *	\warning	This method is arbitrary slow (O(n)) and should be used carefully. Insertion order is not preserved.
 */
//----------------------------------------------------------------------
bool Container::Delete(udword entry)
{
  // Look for the entry
  for(udword i=0;i<mCurNbEntries;i++)
    {
      if(mEntries[i]==entry)
        {
          // Entry has been found at index i. The strategy is to copy the last current entry at index i, and decrement the current number of entries.
          DeleteIndex(i);
          return true;
        }
    }
  return false;
}

//----------------------------------------------------------------------
/**
 *	Deletes an entry, preserving the insertion order. If the container contains such an entry, it's removed.
 *	\param		entry		[in] the value to delete.
 *	\return		true if the value has been found in the container, else false.
 *	\warning	This method is arbitrary slow (O(n)) and should be used carefully.
 */
//----------------------------------------------------------------------
bool Container::DeleteKeepingOrder(udword entry)
{
  // Look for the entry
  for(udword i=0;i<mCurNbEntries;i++)
    {
      if(mEntries[i]==entry)
        {
          // Entry has been found at index i.
          // Shift entries to preserve order. You really should use a linked list instead.
          mCurNbEntries--;
          for(udword j=i;j<mCurNbEntries;j++)
            {
              mEntries[j] = mEntries[j+1];
            }
          return true;
        }
    }
  return false;
}

//----------------------------------------------------------------------
/**
 *	Gets the next entry, starting from input one.
 *	\param		entry		[in/out] On input, the entry to look for. On output, the next entry
 *	\param		find_mode	[in] wrap/clamp
 *	\return		Self-Reference
 */
//----------------------------------------------------------------------
Container& Container::FindNext(udword& entry, FindMode find_mode)
{
  udword Location;
  if(Contains(entry, &Location))
    {
      Location++;
      if(Location==mCurNbEntries)	Location = find_mode==FIND_WRAP ? 0 : mCurNbEntries-1;
      entry = mEntries[Location];
    }
  return *this;
}

//----------------------------------------------------------------------
/**
 *	Gets the previous entry, starting from input one.
 *	\param		entry		[in/out] On input, the entry to look for. On output, the previous entry
 *	\param		find_mode	[in] wrap/clamp
 *	\return		Self-Reference
 */
//----------------------------------------------------------------------
Container& Container::FindPrev(udword& entry, FindMode find_mode)
{
  udword Location;
  if(Contains(entry, &Location))
    {
      Location--;
      if(Location==0xffffffff)	Location = find_mode==FIND_WRAP ? mCurNbEntries-1 : 0;
      entry = mEntries[Location];
    }
  return *this;
}

//----------------------------------------------------------------------
/**
 *	Gets the ram used by the container.
 *	\return		the ram used in bytes.
 */
//----------------------------------------------------------------------
udword Container::GetUsedRam() const
{
  return sizeof(Container) + mMaxNbEntries * sizeof(udword);
}

void Container::operator=(const Container& object)
{
  SetSize(object.GetNbEntries());
  CopyMemory(mEntries, object.GetEntries(), mMaxNbEntries*sizeof(udword));
  mCurNbEntries = mMaxNbEntries;
}
