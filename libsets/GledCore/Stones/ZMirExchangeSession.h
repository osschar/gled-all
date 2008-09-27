// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef __CINT__
#ifndef GledCore_ZMirExchangeSession_H
#define GledCore_ZMirExchangeSession_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

template<class T>
class ZMES_map : private hash_map<UInt_t, void*> {

  typedef hash_map<UInt_t, void*>           base;
  typedef hash_map<UInt_t, void*>::iterator iter;

  GMutex	m_mutex;
  UInt_t	m_key;
public:
  ZMES_map() : m_key(0) {}

  UInt_t insert(T* md) {
    m_mutex.Lock();
    UInt_t key = ++m_key;
    base::insert(pair<UInt_t,void*>(key, md));
    m_mutex.Unlock();
    return key;
  }

  void insert(UInt_t key, T* md) {
    m_mutex.Lock();
    base::insert(pair<UInt_t,void*>(key, md));
    m_mutex.Unlock();
  }

  T* retrieve(UInt_t key) {
    m_mutex.Lock();
    T* ret = 0;
    iter i = base::find(key);
    if(i != base::end()) ret = static_cast<T*>(i->second);
    m_mutex.Unlock();
    return ret;
  }

  void remove(UInt_t key) {
    m_mutex.Lock();
    iter i = base::find(key);
    if(i != base::end()) { delete static_cast<T*>(i->second); base::erase(i); }
    m_mutex.Unlock();
  }
};

#endif
#endif
