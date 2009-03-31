//----------------------------------------------------------------------
/**
 *	Contains code for random generators.
 *	\file		IceRandom.cpp
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

using namespace Opcode;

void SRand(udword seed)
{
  srand(seed);
}

udword Rand()
{
  return rand();
}


static BasicRandom gRandomGenerator(42);

udword GetRandomIndex(udword max_index)
{
  // We don't use rand() since it's limited to RAND_MAX
  udword Index = gRandomGenerator.Randomize();
  return Index % max_index;
}

