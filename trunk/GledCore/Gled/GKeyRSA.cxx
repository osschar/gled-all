// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// GKeyRSA
//
// Example of trivial use:
//
//    GKeyRSA k1, k2;
//    TBuffer b(TBuffer::kWrite);
//    try {
//       k1.ReadPubKey(PubKeyFile(mSaturnIdentity));
//       k1.GenerateSecret();
//       k1.SendSecret(b);
//
//       b.SetReadMode();
//       b.SetBufferOffset(0);
//
//       k2.ReadPrivKey(PrivKeyFile(mSaturnIdentity));
//       k2.ReceiveSecret(b);
//       if(k1.MatchSecrets(k2)) {
// 	cout <<"Secret comparison successful\n";
//       } else {
// 	cout <<"Secret comparison failed\n";
//       }
//     }
//     catch(string exc) {
//       cout << "encode test failed: "<< exc << endl;
//     }
//

#include "GKeyRSA.h"
#include <Gled/GThread.h>

#include <TBuffer.h>
#include <TRandom2.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

ClassImp(GKeyRSA)

/**************************************************************************/

bool    GKeyRSA::init_done = false;
GMutex* GKeyRSA::s_ssl_mutexen = 0;

void GKeyRSA::init_ssl()
{
  if(init_done) return;
  SSL_library_init();
  SSL_load_error_strings();

  int nl = CRYPTO_num_locks();
  //printf("GKeyRSA::init_ssl setting up %d locks\n", nl);
  s_ssl_mutexen = new GMutex[nl];
  
  CRYPTO_set_locking_callback(ssl_locker_foo);
  CRYPTO_set_id_callback(ssl_id_foo);

  init_done = true;
}

void GKeyRSA::ssl_locker_foo(int mode, int n, const char *file, int line)
{
  if(mode & CRYPTO_LOCK) {
    //printf("GKeyRSA::ssl_locker locking %d\n", n);
    s_ssl_mutexen[n].Lock();
  } else {
    //printf("GKeyRSA::ssl_locker unlocking %d\n", n);
    s_ssl_mutexen[n].Unlock();
  }
}

unsigned long GKeyRSA::ssl_id_foo()
{
  return GThread::RawSelf();
}

/**************************************************************************/
/**************************************************************************/

bool GKeyRSA::check_error()
{
  mSslError = ERR_get_error();
  return mSslError;
}

const char* GKeyRSA::error_string()
{
  return ERR_error_string(mSslError, 0);
}

/**************************************************************************/
/**************************************************************************/

GKeyRSA::GKeyRSA()
{
  mSecret = 0;
  mSecretLen = 0;

  bIsPrivate = false;
  pKey = 0;
  mSslError = 0;
}

GKeyRSA::~GKeyRSA()
{
  delete pKey;
  delete [] mSecret;
}

/**************************************************************************/

void GKeyRSA::ReadPubKey(const char* file)
{
  string _eh("GKeyRSA::ReadPubKey ");

  if(pKey) { delete pKey; bIsPrivate = false; }
  FILE* fp = fopen(file, "r");
  if(!fp) throw(_eh + "file '" + file + "' not found");
  pKey = PEM_read_RSA_PUBKEY(fp, 0, 0, 0);
  fclose(fp);
  if(check_error()) {
    pKey = 0;
    throw(_eh + error_string());
  }
}

void GKeyRSA::ReadPrivKey(const char* file)
{
  string _eh("GKeyRSA::ReadPrivKey ");

  if(pKey) { delete pKey; bIsPrivate = false; }
  FILE* fp = fopen(file, "r");
  if(!fp) throw(_eh + "file " + file + " not found");
  pKey = PEM_read_RSAPrivateKey(fp, 0, 0, 0);
  fclose(fp);
  if(check_error()) {
    pKey = 0;
    throw(_eh + error_string());
  }
  bIsPrivate = true;
}

/**************************************************************************/

void GKeyRSA::GenerateSecret()
{
  static string _eh("GKeyRSA::GenerateSecret ");

  assert(pKey);

  UInt_t s;
  FILE* fp = fopen("/dev/random", "r");
  fread(&s, sizeof(UInt_t), 1, fp);
  fclose(fp);
  TRandom2 rnd(s);

  int modlen = RSA_size(pKey);
  mSecretLen = modlen - 41 - 1;
  if(mSecret) delete mSecret;
  mSecret    = new unsigned char[mSecretLen];
  for(int i=0; i<mSecretLen; ++i) {
    mSecret[i] = (unsigned char)(255.99*rnd.Rndm());
  }

}

void GKeyRSA::SendSecret(TBuffer& b)
{
  static string _eh("GKeyRSA::SendSecret ");

  int modlen = RSA_size(pKey);
  unsigned char enc_secret[modlen];
  Int_t lenmsg = RSA_public_encrypt(mSecretLen, mSecret, enc_secret,
				    pKey, RSA_PKCS1_OAEP_PADDING);

  if(check_error())
    throw(_eh + error_string());

  b << lenmsg;
  b.WriteFastArray((char*)enc_secret, lenmsg);
}

void GKeyRSA::ReceiveSecret(TBuffer& b)
{
  static string _eh("GKeyRSA::ReceiveSecret ");

  assert(pKey && bIsPrivate);

  Int_t lenmsg;
  b >> lenmsg;
  unsigned char enc_secret[lenmsg];
  unsigned char rec_secret[lenmsg];
  b.ReadFastArray((char*)enc_secret, lenmsg);
  mSecretLen = RSA_private_decrypt(lenmsg, enc_secret, rec_secret,
				   pKey, RSA_PKCS1_OAEP_PADDING);
  if(check_error())
    throw(_eh + error_string());

  if(mSecret) delete mSecret;
  mSecret    = new unsigned char[mSecretLen];
  for(int i=0; i<mSecretLen; ++i) mSecret[i] = rec_secret[i];
}

bool GKeyRSA::MatchSecrets(const GKeyRSA& a)
{
  if(mSecretLen != a.mSecretLen) return false;
  for(int i=0; i<mSecretLen; ++i) {
    if(mSecret[i] != a.mSecret[i]) return false;
  }
  return true;
}

/**************************************************************************/

void GKeyRSA::StreamPubKey(TBuffer& b)
{
  if(b.IsReading()) {

    if(pKey) delete pKey;
    pKey = RSA_new();
    bIsPrivate = false;

    Int_t ln, le;
    b >> ln >> le;

    char bn[ln], be[le];
    b.ReadFastArray(bn, ln);
    b.ReadFastArray(be, le);

    pKey->n = BN_mpi2bn((unsigned char *)bn, ln, 0);
    pKey->e = BN_mpi2bn((unsigned char *)be, le, 0);

  } else {

    Int_t ln, le;
    ln = BN_bn2mpi(pKey->n, NULL);
    le = BN_bn2mpi(pKey->e, NULL);
    b << ln << le;

    char bn[ln], be[le];
    BN_bn2mpi(pKey->n, (unsigned char*)bn);
    BN_bn2mpi(pKey->e, (unsigned char*)be);
    b.WriteFastArray(bn, ln);
    b.WriteFastArray(be, le);
  }
}
