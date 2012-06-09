// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GQueue_H
#define GledCore_GQueue_H

#include "GCondition.h"

template <typename TT>
class GQueue
{
  list<TT*>   mQueue;
  GCondition  mCondition;

public:
  GQueue(GMutex::Init_e e=GMutex::fast) : mCondition(e) {}
  ~GQueue() {}

  void PushBack(TT* el);
  TT*  PopFront();
  TT*  PopFrontTimedWait(GTime time);

  void ClearQueue();
  void ClearQueueDecRefCount();

  void Lock()   { mCondition.Lock();   }
  void Unlock() { mCondition.Unlock(); }
};

//------------------------------------------------------------------------------

template <typename TT>
void GQueue<TT>::PushBack(TT* el)
{
  GMutexHolder _lck(mCondition);
  mQueue.push_back(el);
  mCondition.Signal();
}

template <typename TT>
TT* GQueue<TT>::PopFront()
{
  GMutexHolder _lck(mCondition);
  if (mQueue.empty())
  {
    mCondition.Wait();
    if (mQueue.empty())
      return 0;
  }
  TT *el = mQueue.front();
  mQueue.pop_front();
  return el;
}

template <typename TT>
TT* GQueue<TT>::PopFrontTimedWait(GTime time)
{
  GMutexHolder _lck(mCondition);
  if (mQueue.empty())
  {
    mCondition.TimedWait(time);
    if (mQueue.empty())
      return 0;
  }
  TT *el = mQueue.front();
  mQueue.pop_front();
  return el;
}

template <typename TT>
void GQueue<TT>::ClearQueue()
{
  GMutexHolder _lck(mCondition);
  mQueue.clear();
}

template <typename TT>
void GQueue<TT>::ClearQueueDecRefCount()
{
  GMutexHolder _lck(mCondition);
  while (! mQueue.empty())
  {
    mQueue.front()->DecRefCount();
    mQueue.pop_front();
  }
}

//==============================================================================

template<typename TT>
class GQueueSet
{
  typedef GQueue<TT>                   Queue_t;
  typedef set<Queue_t*>                spQueue_t;
  typedef typename spQueue_t::iterator spQueue_i;

  spQueue_t      mQueueSet;
  mutable GMutex mSetMutex;

public:
  GQueueSet() {}

  Int_t GetSetSize() const;

  void RegisterQueue  (Queue_t* q);
  void UnregisterQueue(Queue_t* q);

  void DeliverToQueues(TT* el);
};

//------------------------------------------------------------------------------

template <typename TT>
Int_t GQueueSet<TT>::GetSetSize() const
{
  GMutexHolder _lck(mSetMutex);
  return mQueueSet.size();
}

template <typename TT>
void GQueueSet<TT>::RegisterQueue(Queue_t* q)
{
  GMutexHolder _lck(mSetMutex);
  mQueueSet.insert(q);
}

template <typename TT>
void GQueueSet<TT>::UnregisterQueue(Queue_t* q)
{
  GMutexHolder _lck(mSetMutex);
  mQueueSet.erase(q);
}

template <typename TT>
void GQueueSet<TT>::DeliverToQueues(TT* el)
{
  GMutexHolder _lck(mSetMutex);
  if ( ! mQueueSet.empty())
  {
    el->IncRefCount(mQueueSet.size());
    for (spQueue_i i = mQueueSet.begin(); i != mQueueSet.end(); ++i)
    {
      (*i)->PushBack(el);
    }
  }
  else
  {
    delete el;
  }
}

#endif
