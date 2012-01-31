// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_UdpPacketSource_H
#define CmsGridViz_UdpPacketSource_H

#include "Glasses/ZGlass.h"
#include "Gled/GQueue.h"

class ZLog;
class SUdpPacket;


/*

// Abstract base for consumers of UdpPacketSource.

class UdpPacketSink
{
protected:
  typedef GQueue<SUdpPacket> Queue_t;
  Queue_t mUdpPacketQueue;

public:
  virtual ~UdpPacketSink() {}

  // ??? Need this at all ???
  // UdpPacketSource would then be a ZHashList of zglasses,
  // making sure they can be dyn-casted to UdpPacketSink.
  // Then, the sink would get the packet and either do it in 
  // deliverer thread or pass it on to packet queue.
  // This means, we'd loop over list elements all the time.
  // But that's not too bad ... we have list-time stamp.
  //
  // Or ... just register the blody queue to sink. And have a thread, it's not
  // so hard.

  ClassDef(UdpPacketSink, 1);
};

class UdpPacketSinkWithQueue
{
protected:
  typedef GQueue<SUdpPacket> Queue_t;
  Queue_t mUdpPacketQueue;

public:
  virtual ~UdpPacketSinkWithQueue() {}

  // ??? Need this at all ???
  // UdpPacketSource would then be a ZHashList of zglasses,
  // making sure they can be dyn-casted to UdpPacketSink.
  // Then, the sink would get the packet and either do it in 
  // deliverer thread or pass it on to packet queue.

  ClassDef(UdpPacketSinkWithQueue, 1);
};

*/

class UdpPacketSource : public ZGlass
{
  MAC_RNR_FRIENDS(UdpPacketSource);

public:
  typedef GQueue<SUdpPacket>    Queue_t;
  typedef GQueueSet<SUdpPacket> QueueSet_t;

private:
  void _init();

protected:
  ZLink<ZLog>       mLog;          // X{GS} L{}

  QueueSet_t        mConsumerSet;

public:
  UdpPacketSource(const Text_t* n="UdpPacketSource", const Text_t* t=0);
  virtual ~UdpPacketSource();

  void RegisterConsumer  (Queue_t* q);
  void UnregisterConsumer(Queue_t* q);

#include "UdpPacketSource.h7"
  ClassDef(UdpPacketSource, 1);
}; // endclass UdpPacketSource

#endif
