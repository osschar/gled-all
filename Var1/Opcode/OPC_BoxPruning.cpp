//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Contains code for box pruning.
 *	\file		IceBoxPruning.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
//----------------------------------------------------------------------

/*
//----------------------------------------------------------------------

You could use a complex sweep-and-prune as implemented in I-Collide.
You could use a complex hashing scheme as implemented in V-Clip or
  recently in ODE it seems.
You could use a "Recursive Dimensional Clustering" algorithm as
  implemented in GPG2.

Or you could use this.

Faster ? I don't know. Probably not. It would be a shame. But who knows ?
Easier ? Definitely. Enjoy the sheer simplicity.
//----------------------------------------------------------------------
*/


//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

using namespace Opcode;

inline_ void FindRunningIndex(udword& index, float* array, udword* sorted,
                              int last, float max)
{
  int First=index;
  while (First<=last)
    {
      index = (First+last)>>1;

      if (max>array[sorted[index]])	First	= index+1;
      else				last	= index-1;
    }
}
// ### could be log(n) !
// and maybe use cmp integers

// InsertionSort has better coherence, RadixSort is better for one-shot queries.
#define PRUNING_SORTER	RadixSort
//#define PRUNING_SORTER	InsertionSort

// Static for coherence
static PRUNING_SORTER* gCompletePruningSorter   = 0;
static PRUNING_SORTER* gBipartitePruningSorter0 = 0;
static PRUNING_SORTER* gBipartitePruningSorter1 = 0;
inline_ PRUNING_SORTER* GetCompletePruningSorter()
{
  if (!gCompletePruningSorter)	gCompletePruningSorter = new PRUNING_SORTER;
  return gCompletePruningSorter;
}
inline_ PRUNING_SORTER* GetBipartitePruningSorter0()
{
  if (!gBipartitePruningSorter0)	gBipartitePruningSorter0 = new PRUNING_SORTER;
  return gBipartitePruningSorter0;
}
inline_ PRUNING_SORTER* GetBipartitePruningSorter1()
{
  if (!gBipartitePruningSorter1)	gBipartitePruningSorter1 = new PRUNING_SORTER;
  return gBipartitePruningSorter1;
}
void ReleasePruningSorters()
{
  DELETESINGLE(gBipartitePruningSorter1);
  DELETESINGLE(gBipartitePruningSorter0);
  DELETESINGLE(gCompletePruningSorter);
}


//----------------------------------------------------------------------
/**
 *	Bipartite box pruning. Returns a list of overlapping pairs of
 *	boxes, each box of the pair belongs to a different set.
 *	\param		nb0	[in] number of boxes in the first set
 *	\param		array0	[in] array of boxes for the first set
 *	\param		nb1	[in] number of boxes in the second set
 *	\param		array1	[in] array of boxes for the second set
 *	\param		pairs	[out] array of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
//----------------------------------------------------------------------
bool Opcode::BipartiteBoxPruning(udword nb0, const AABB** array0,
                                 udword nb1, const AABB** array1,
                                 Pairs& pairs, const Axes& axes)
{
  // Checkings
  if (!nb0 || !array0 || !nb1 || !array1) return false;

  // Catch axes
  udword Axis0 = axes.mAxis0;
  udword Axis1 = axes.mAxis1;
  udword Axis2 = axes.mAxis2;

  // Allocate some temporary data
  float* MinPosList0 = new float[nb0];
  float* MinPosList1 = new float[nb1];

  // 1) Build main lists using the primary axis
  for (udword i=0;i<nb0;i++)	MinPosList0[i] = array0[i]->GetMin(Axis0);
  for (udword i=0;i<nb1;i++)	MinPosList1[i] = array1[i]->GetMin(Axis0);

  // 2) Sort the lists
  PRUNING_SORTER* RS0 = GetBipartitePruningSorter0();
  PRUNING_SORTER* RS1 = GetBipartitePruningSorter1();
  const udword* Sorted0 = RS0->Sort(MinPosList0, nb0).GetRanks();
  const udword* Sorted1 = RS1->Sort(MinPosList1, nb1).GetRanks();

  // 3) Prune the lists
  udword Index0, Index1;

  const udword* const LastSorted0 = &Sorted0[nb0];
  const udword* const LastSorted1 = &Sorted1[nb1];
  const udword* RunningAddress0 = Sorted0;
  const udword* RunningAddress1 = Sorted1;

  while (RunningAddress1<LastSorted1 && Sorted0<LastSorted0)
  {
    Index0 = *Sorted0++;

    while (RunningAddress1<LastSorted1 && MinPosList1[*RunningAddress1]<MinPosList0[Index0])	RunningAddress1++;

    const udword* RunningAddress2_1 = RunningAddress1;

    while (RunningAddress2_1<LastSorted1 && MinPosList1[Index1 = *RunningAddress2_1++]<=array0[Index0]->GetMax(Axis0))
    {
      if (array0[Index0]->Intersect(*array1[Index1], Axis1))
      {
        if (array0[Index0]->Intersect(*array1[Index1], Axis2))
        {
          pairs.AddPair(Index0, Index1);
        }
      }
    }
  }

  //----

  while (RunningAddress0<LastSorted0 && Sorted1<LastSorted1)
  {
    Index0 = *Sorted1++;

    while (RunningAddress0<LastSorted0 && MinPosList0[*RunningAddress0]<=MinPosList1[Index0])	RunningAddress0++;

    const udword* RunningAddress2_0 = RunningAddress0;

    while (RunningAddress2_0<LastSorted0 && MinPosList0[Index1 = *RunningAddress2_0++]<=array1[Index0]->GetMax(Axis0))
    {
      if (array0[Index1]->Intersect(*array1[Index0], Axis1))
      {
        if (array0[Index1]->Intersect(*array1[Index0], Axis2))
        {
          pairs.AddPair(Index1, Index0);
        }
      }

    }
  }

  DELETEARRAY(MinPosList1);
  DELETEARRAY(MinPosList0);

  return true;
}

#define ORIGINAL_VERSION
//#define JOAKIM

//----------------------------------------------------------------------
/**
 *	Complete box pruning. Returns a list of overlapping pairs of
 *	boxes, each box of the pair belongs to the same set.
 *	\param		nb	[in] number of boxes
 *	\param		array	[in] array of boxes
 *	\param		pairs	[out] array of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
//----------------------------------------------------------------------
bool Opcode::CompleteBoxPruning(udword nb, const AABB** array,
                                Pairs& pairs, const Axes& axes)
{
  // Checkings
  if (!nb || !array) return false;

  // Catch axes
  udword Axis0 = axes.mAxis0;
  udword Axis1 = axes.mAxis1;
  udword Axis2 = axes.mAxis2;

#ifdef ORIGINAL_VERSION
  // Allocate some temporary data
  //	float* PosList = new float[nb];
  float* PosList = new float[nb+1];

  // 1) Build main list using the primary axis
  for (udword i=0;i<nb;i++)	PosList[i] = array[i]->GetMin(Axis0);
  PosList[nb++] = MAX_FLOAT;

  // 2) Sort the list
  PRUNING_SORTER* RS = GetCompletePruningSorter();
  const udword* Sorted = RS->Sort(PosList, nb).GetRanks();

  // 3) Prune the list
  const udword* const LastSorted = &Sorted[nb];
  const udword* RunningAddress = Sorted;
  udword Index0, Index1;
  while (RunningAddress<LastSorted && Sorted<LastSorted)
  {
    Index0 = *Sorted++;

    // while (RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);
    while (PosList[*RunningAddress++]<PosList[Index0]);

    if (RunningAddress<LastSorted)
    {
      const udword* RunningAddress2 = RunningAddress;

      // while (RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=array[Index0]->GetMax(Axis0))
      while (PosList[Index1 = *RunningAddress2++]<=array[Index0]->GetMax(Axis0))
      {
        // if (Index0!=Index1)
        //   {
        if (array[Index0]->Intersect(*array[Index1], Axis1))
        {
          if (array[Index0]->Intersect(*array[Index1], Axis2))
          {
            pairs.AddPair(Index0, Index1);
          }
        }
        //   }
      }
    }
  }

  DELETEARRAY(PosList);
#endif

#ifdef JOAKIM
  // Allocate some temporary data
  //	float* PosList = new float[nb];
  float* MinList = new float[nb+1];

  // 1) Build main list using the primary axis
  for (udword i=0;i<nb;i++)	MinList[i] = array[i]->GetMin(Axis0);
  MinList[nb] = MAX_FLOAT;

  // 2) Sort the list
  PRUNING_SORTER* RS = GetCompletePruningSorter();
  udword* Sorted = RS->Sort(MinList, nb+1).GetRanks();

  // 3) Prune the list
  //	const udword* const LastSorted = &Sorted[nb];
  //	const udword* const LastSorted = &Sorted[nb-1];
  const udword* RunningAddress = Sorted;
  udword Index0, Index1;

  // while (RunningAddress<LastSorted && Sorted<LastSorted)
  // while (RunningAddress<LastSorted)
  // while (Sorted<LastSorted)
  while (RunningAddress<&Sorted[nb])
  {
    // Index0 = *Sorted++;
    Index0 = *RunningAddress++;

    // while (RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);
    // while (PosList[*RunningAddress++]<PosList[Index0]);
    // RunningAddress = Sorted;
    // if (RunningAddress<LastSorted)
    {
      const udword* RunningAddress2 = RunningAddress;

      // while (RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=array[Index0]->GetMax(Axis0))

      // float CurrentMin = array[Index0]->GetMin(Axis0);
      float CurrentMax = array[Index0]->GetMax(Axis0);

      while (MinList[Index1 = *RunningAddress2] <= CurrentMax)
        // while (PosList[Index1 = *RunningAddress] <= CurrentMax)
      {
        // if (Index0!=Index1)
        //   {
        if (array[Index0]->Intersect(*array[Index1], Axis1))
        {
          if (array[Index0]->Intersect(*array[Index1], Axis2))
          {
            pairs.AddPair(Index0, Index1);
          }
        }
        //   }

        RunningAddress2++;
        // RunningAddress++;
      }
    }
  }

  DELETEARRAY(MinList);
#endif

  return true;
}


//______________________________________________________________________________
//
// class:  BipartiteBoxPruner
// author: Matevz Tadel
// date:   Jan 26, 27 2008
//
// Management of two full box-pruning structures with functions to perform
// various queries on them.
//______________________________________________________________________________


Opcode::BipartiteBoxPruner::BipartiteBoxPruner() : mAxes(AXES_XZY)
{
  for (int l=0; l<2; ++l)
  {
    mN[l] = 0;
    mBoxList[l] = 0;
    mUserData[l] = 0;
    mMinPosList[l] = 0;
    mSorter[l] = 0;
  }
}

Opcode::BipartiteBoxPruner::~BipartiteBoxPruner()
{
  for (int l=0; l<2; ++l)  ClearList(l);
}

void Opcode::BipartiteBoxPruner::ClearList(int l)
{
  mN[l] = 0;
  delete [] mBoxList[l];    mBoxList[l]    = 0;
  delete [] mUserData[l];   mUserData[l]   = 0;
  delete [] mMinPosList[l]; mMinPosList[l] = 0;
  delete mSorter[l];        mSorter[l]     = 0;
}

void Opcode::BipartiteBoxPruner::InitList(int l, int n)
{
  ClearList(l);
  mN[l]          = n;
  mBoxList[l]    = new const AABB* [n];
  mUserData[l]   = new void* [n];
  mMinPosList[l] = new float [n+1]; // CompletePruning requires extra max_float at the end.
  mSorter[l]     = new RadixSort;
}

void Opcode::BipartiteBoxPruner::Sort(int l)
{
  // Sort the list l. This fills the min-pos-list and calls radix sort.

  // 1) Build min list using the primary axis
  mMaxXExtent[l] = 0;
  for (udword i=0; i<mN[l]; ++i)
  {
    float m = mBoxList[l][i]->GetMin(mAxes.mAxis0);
    mMinPosList[l][i] = m;
    m = mBoxList[l][i]->GetMax(mAxes.mAxis0) - m;
    if (m > mMaxXExtent[l]) mMaxXExtent[l] = m;
  }
  mMinPosList[l][mN[l]] = MAX_FLOAT;

  // 2) Sort the list
  mSorter[l]->Sort(mMinPosList[l], mN[l] + 1);
}

void Opcode::BipartiteBoxPruner::BipartitePruning(Pairs& pairs, int l0, int l1)
{
  // Perform bipartite box-pruning with lists l0 and l1, that is, the resulting
  // pairs will have the first element from l0 and he second from l1.
  //
  // The lists must be sorted previously.

  udword Axis0 = mAxes.mAxis0;
  udword Axis1 = mAxes.mAxis1;
  udword Axis2 = mAxes.mAxis2;

  const AABB** array0 = mBoxList[l0];
  const AABB** array1 = mBoxList[l1];

  float* MinPosList0 = mMinPosList[l0];
  float* MinPosList1 = mMinPosList[l1];

  const udword* Sorted0 = mSorter[l0]->GetRanks();
  const udword* Sorted1 = mSorter[l1]->GetRanks();

  const udword* const LastSorted0 = &Sorted0[mN[l0]];
  const udword* const LastSorted1 = &Sorted1[mN[l1]];
  const udword* RunningAddress0 = Sorted0;
  const udword* RunningAddress1 = Sorted1;

  udword Index0, Index1;

  while (RunningAddress1 < LastSorted1 && Sorted0 < LastSorted0)
  {
    Index0 = *Sorted0++;

    while (RunningAddress1 < LastSorted1 && MinPosList1[*RunningAddress1] < MinPosList0[Index0])
      RunningAddress1++;

    const udword* RunningAddress2_1 = RunningAddress1;

    while (RunningAddress2_1 < LastSorted1 && MinPosList1[Index1 = *RunningAddress2_1++] <= array0[Index0]->GetMax(Axis0))
    {
      if (array0[Index0]->Intersect(*array1[Index1], Axis1))
      {
        if (array0[Index0]->Intersect(*array1[Index1], Axis2))
        {
          pairs.AddPair(Index0, Index1);
        }
      }
    }
  }

  //----

  while (RunningAddress0 < LastSorted0 && Sorted1 < LastSorted1)
  {
    Index0 = *Sorted1++;

    while (RunningAddress0 < LastSorted0 && MinPosList0[*RunningAddress0] <= MinPosList1[Index0])
      RunningAddress0++;

    const udword* RunningAddress2_0 = RunningAddress0;

    while (RunningAddress2_0 < LastSorted0 && MinPosList0[Index1 = *RunningAddress2_0++] <= array1[Index0]->GetMax(Axis0))
    {
      if (array0[Index1]->Intersect(*array1[Index0], Axis1))
      {
        if (array0[Index1]->Intersect(*array1[Index0], Axis2))
        {
          pairs.AddPair(Index1, Index0);
        }
      }
    }
  }
}

void Opcode::BipartiteBoxPruner::CompletePruning(Pairs& pairs, int l)
{
  // Perform complete box-pruning on the list l itself.
  //
  // The list must be sorted before.

  udword Axis0 = mAxes.mAxis0;
  udword Axis1 = mAxes.mAxis1;
  udword Axis2 = mAxes.mAxis2;

  const   AABB** array   = mBoxList[l];

         float*  PosList = mMinPosList[l];
  const udword*  Sorted  = mSorter[l]->GetRanks();

  const udword* const LastSorted     = &Sorted[mN[l] + 1]; // Watch this, points one past extra MAX_FLOAT.
  const udword*       RunningAddress =  Sorted;

  udword Index0, Index1;
  while (RunningAddress < LastSorted && Sorted < LastSorted)
  {
    Index0 = *Sorted++;

    while (PosList[*RunningAddress++] < PosList[Index0]);

    if (RunningAddress < LastSorted)
    {
      const udword* RunningAddress2 = RunningAddress;

      while (PosList[Index1 = *RunningAddress2++] <= array[Index0]->GetMax(Axis0))
      {
        if (array[Index0]->Intersect(*array[Index1], Axis1))
        {
          if (array[Index0]->Intersect(*array[Index1], Axis2))
          {
            pairs.AddPair(Index0, Index1);
          }
        }
      }
    }
  }
}

int Opcode::BipartiteBoxPruner::FindNearestSmallerMinPos(int l, float value, int below, int above)
{
  // Find index of the box whose min-pos is the closest to 'value' but
  // smaller. Returns integer in range [-1, list_len - 1].
  //
  // List must be sorted.
  //
  // 'below' and 'above' can be specified to limit the search to a
  // given range.

  if (below < -1)                  below = -1;
  if (above <  0 || above > mN[l]) above = mN[l];

  const udword*  Sorted  = mSorter[l]->GetRanks();

  while (above - below > 1)
  {
    const int middle = (above + below) / 2;
    if (mMinPosList[l][Sorted[middle]] < value)
      below = middle;
    else
      above = middle;
  }
  return below;
}

void Opcode::BipartiteBoxPruner::SinglePruning(Container& hits, int l, const AABB& box)
{
  // Collect overlaps of given box with elements of list l.
  //
  // The list must be sorted.
  //
  // This is not terrifically efficient, as all we know is that
  // elements of the list with min > box_max can not intersect the
  // box. We still need to iterate over all the others.
  //
  // To speed this up we also store the max X bounding-box extent of
  // all list elements during sorting. Thus we can begin checks at
  // box_min - max_x_extent and end them at the upper limit discussed
  // in previous paragraph.

  udword Axis0 = mAxes.mAxis0;
  udword Axis1 = mAxes.mAxis1;
  udword Axis2 = mAxes.mAxis2;

  const   AABB** array   = mBoxList[l];

  udword fi = FindNearestSmallerMinPos(l, box.GetMin(Axis0) - mMaxXExtent[l]) + 1;
  if (fi == mN[l])
    return;
  udword li = FindNearestSmallerMinPos(l, box.GetMax(Axis0), fi) + 1;

  const udword* Sorted     = mSorter[l]->GetRanks() + fi;
  const udword* LastSorted = mSorter[l]->GetRanks() + li;

  while (Sorted < LastSorted)
  {
    udword Index = *Sorted++;

    // Check the main axis last as it is most likely for boxes to
    // intersect there. Most of tests will fail, so let them fail
    // soon.
    if (box.Intersect(*array[Index], Axis1) &&
        box.Intersect(*array[Index], Axis2) &&
        box.Intersect(*array[Index], Axis0))
    {
      hits.Add(Index);
    }
  }
}


/******************************************************************************/
/******************************************************************************/

//----------------------------------------------------------------------
// Brute-force versions are kept:
// - to check the optimized versions return the correct list of intersections
// - to check the speed of the optimized code against the brute-force one
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Brute-force bipartite box pruning. Returns a list of
 *	overlapping pairs of boxes, each box of the pair belongs to a
 *	different set.
 *	\param		nb0	[in] number of boxes in the first set
 *	\param		array0	[in] array of boxes for the first set
 *	\param		nb1	[in] number of boxes in the second set
 *	\param		array1	[in] array of boxes for the second set
 *	\param		pairs	[out] array of overlapping pairs
 *	\return		true if success.
 */
//----------------------------------------------------------------------
bool Opcode::BruteForceBipartiteBoxTest(udword nb0, const AABB** array0,
                                        udword nb1, const AABB** array1,
                                        Pairs& pairs)
{
  // Checkings
  if (!nb0 || !array0 || !nb1 || !array1) return false;

  // Brute-force nb0*nb1 overlap tests
  for (udword i=0;i<nb0;i++)
    {
      for (udword j=0;j<nb1;j++)
        {
          if (array0[i]->Intersect(*array1[j]))	pairs.AddPair(i, j);
        }
    }
  return true;
}

//----------------------------------------------------------------------
/**
 *	Complete box pruning. Returns a list of overlapping pairs of
 *	boxes, each box of the pair belongs to the same set.
 *	\param		nb	[in] number of boxes
 *	\param		array	[in] array of boxes
 *	\param		pairs	[out] array of overlapping pairs
 *	\return		true if success.
 */
//----------------------------------------------------------------------
bool Opcode::BruteForceCompleteBoxTest(udword nb, const AABB** array, Pairs& pairs)
{
  // Checkings
  if (!nb || !array)	return false;

  // Brute-force n(n-1)/2 overlap tests
  for (udword i=0;i<nb;i++)
    {
      for (udword j=i+1;j<nb;j++)
        {
          if (array[i]->Intersect(*array[j]))	pairs.AddPair(i, j);
        }
    }
  return true;
}
