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
};

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
    mCondition.Wait();
  TT *el = mQueue.front();
  mQueue.pop_front();
  return el;
}

#endif
