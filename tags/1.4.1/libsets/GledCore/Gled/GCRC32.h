// $Id$

// Public domain, original author Eric Durbin, 1998.
// Modified by Andrew Hanushevsky, 2007.
// Modified by Matevz Tadel upon inclusion in Gled in 2011.

#ifndef GledCore_GCRC32_H
#define GledCore_GCRC32_H

#include <Rtypes.h>

class GCRC32
{
public:
  enum Init_e { I_Now };

protected:
  UInt_t	mCrc;

  static const UInt_t sInit;
  static const UInt_t sXorOut;

  void process(const UChar_t* p, Int_t len);

public:
  GCRC32() : mCrc(sInit) {}
  GCRC32(const GCRC32& t) : mCrc(t.mCrc) {}

  ~GCRC32() {}

  UInt_t GetCrc() const { return mCrc ^ sXorOut; }
  void   Reset() { mCrc = sInit; }

  template<typename T> GCRC32& Start(T* p, Int_t len)
  { mCrc = sInit; process((UChar_t*) p, len); return *this; }

  template<typename T> GCRC32& Start(T& r)
  { mCrc = sInit; process((UChar_t*) &r, sizeof(T)); return *this; }

  template<typename T> GCRC32& Process(const T* p, Int_t len)
  { process((const UChar_t*) p, len); return *this; }

  template<typename T> GCRC32& Process(T& r)
  { process((UChar_t*) &r, sizeof(T)); return *this; }

  template<typename T> UInt_t Finish(const T* p, Int_t len)
  { process((const UChar_t*) p, len); return GetCrc(); }

  template<typename T> UInt_t Finish(T& r)
  { process((UChar_t*) &r, sizeof(T)); return GetCrc(); }


  static UInt_t CRC32(const UChar_t* p, Int_t len);

  ClassDefNV(GCRC32, 0);
}; // endclass GCRC32

#endif

