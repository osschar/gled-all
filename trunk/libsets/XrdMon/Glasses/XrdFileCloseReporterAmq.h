// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFileCloseReporterAmq_H
#define XrdMon_XrdFileCloseReporterAmq_H

#include <Glasses/XrdFileCloseReporter.h>

#include <cms/ExceptionListener.h>

namespace cms
{
  class ConnectionFactory;
  class Connection;
  class Session;
  class Destination;
  class MessageProducer;
}


class XrdFileCloseReporterAmq : public XrdFileCloseReporter,
				public cms::ExceptionListener
{
  MAC_RNR_FRIENDS(XrdFileCloseReporterAmq);

private:
  void _init();

protected:
  TString       mAmqHost;     // X{GS} 7 Textor(-width=>24)
  UShort_t      mAmqPort;     // X{GS} 7 Value()
  TString       mAmqUser;     // X{GS} 7 Textor()
  TString       mAmqPswd;     // X{GS} 7 Textor()
  TString       mAmqTopic;    // X{GS} 7 Textor()
  Bool_t        bLeakAmqObjs; // X{GS} 7 Bool()
  Bool_t        bPersistent;  // X{GS} 7 Bool()

  cms::ConnectionFactory   *mConnFac;    //!
  cms::Connection          *mConn;       //!
  cms::Session             *mSess;       //!
  cms::Destination         *mDest;       //!
  cms::MessageProducer     *mProd;       //!
  Bool_t                    bConnClosed; //!

  UInt_t mAmqMaxMsgQueueLen;        // X{GS} 7 Value(-range=>[1, 10000000, 1])
  Int_t  mAmqReconnectWaitSec;      // X{GS} 7 Value(-range=>[1, 100,   1])
  Int_t  mAmqReconnectWaitSecMax;   // X{GS} 7 Value(-range=>[1, 10000, 1])
  Int_t  mAmqAutoReconnectSec;      // X{GS} 7 Value(-range=>[0, 10000000, 1])

  Int_t  mAmqTotalConnectSuccessCount; //! X{G} 7 ValOut()
  Int_t  mAmqTotalConnectFailCount;    //! X{G} 7 ValOut()
  Int_t  mAmqTotalAutoReconnectCount; //! X{G} 7 ValOut()
  Int_t  mAmqCurrentConnectFailCount;  //! X{G} 7 ValOut()
  Int_t  mAmqSendMessageFailCount;     //! X{G} 7 ValOut()
  Bool_t bAmqConnected;                //! X{G} 7 BoolOut()

  GThread    *mAmqThread;              //!
  GCondition  mAmqCond;                //!
  lStr_t      mAmqMsgQueue;            //!

  static void* tl_AmqHandler(XrdFileCloseReporterAmq* fcr_amq);
  static void  cu_AmqHandler(XrdFileCloseReporterAmq* fcr_amq);

  void AmqHandler();
  void amq_connect();
  void amq_disconnect();

  virtual void ReportLoopInit();
  virtual void ReportFileClosed(FileUserServer& fus);
  virtual void ReportLoopFinalize();

public:
  XrdFileCloseReporterAmq(const Text_t* n="XrdFileCloseReporterAmq", const Text_t* t=0);
  virtual ~XrdFileCloseReporterAmq();

  virtual void onException(const cms::CMSException &e);

#include "XrdFileCloseReporterAmq.h7"
  ClassDef(XrdFileCloseReporterAmq, 1);
}; // endclass XrdFileCloseReporterAmq

#endif
