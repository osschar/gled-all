// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GKeyRSA_H
#define GledCore_GKeyRSA_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

class TBuffer;

typedef struct rsa_st RSA;

class GKeyRSA {

  friend class Gled;

private:
  static void init_ssl();
  static bool init_done;

  static GMutex*       s_ssl_mutexen;
  static void          ssl_locker_foo(int, int, const char*, int);
  static unsigned long ssl_id_foo();

  int			mSecretLen;
  unsigned char*	mSecret;

protected:

  Bool_t	bIsPrivate;	// X{G}

  RSA*		pKey;

  unsigned long mSslError;
  bool          check_error();
  const char*   error_string();

public:
  GKeyRSA();
  virtual ~GKeyRSA();

  void ReadPubKey(const char* file);
  void ReadPrivKey(const char* file);

  void GenerateSecret();
  void SendSecret(TBuffer& b);
  void ReceiveSecret(TBuffer& b);
  bool MatchSecrets(const GKeyRSA& a);

  void StreamPubKey(TBuffer& b);

#include "GKeyRSA.h7"
  ClassDef(GKeyRSA, 0)
}; // endclass GKeyRSA

#endif
