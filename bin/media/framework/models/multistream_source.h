// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "garnet/bin/media/framework/models/node.h"
#include "garnet/bin/media/framework/models/stage.h"
#include "garnet/bin/media/framework/packet.h"

namespace media {

// Stage for |MultistreamSource|.
class MultistreamSourceStage : public Stage {
 public:
  ~MultistreamSourceStage() override {}
};

// Synchronous source of packets for multiple streams.
class MultistreamSource : public Node<MultistreamSourceStage> {
 public:
  ~MultistreamSource() override {}

  // Flushes media state.
  virtual void Flush(){};

  // Returns the number of streams the source produces.
  virtual size_t stream_count() const = 0;

  // Gets a packet for the stream indicated via stream_index_out. This call
  // should always produce a packet until end-of-stream. The caller is
  // responsible for releasing the packet.
  virtual PacketPtr PullPacket(size_t* stream_index_out) = 0;
};

}  // namespace media
