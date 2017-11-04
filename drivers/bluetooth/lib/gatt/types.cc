// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "types.h"

#include "lib/fxl/logging.h"

namespace btlib {
namespace gatt {

Service::Service(bool primary, const common::UUID& type)
    : primary_(primary), type_(type) {}

Characteristic::Characteristic(IdType id,
                               const common::UUID& type,
                               uint8_t properties,
                               uint16_t extended_properties,
                               const att::AccessRequirements& read_permissions,
                               const att::AccessRequirements& write_permissions)
    : id_(id),
      type_(type),
      properties_(properties),
      extended_properties_(extended_properties),
      read_permissions_(read_permissions),
      write_permissions_(write_permissions) {}

Descriptor::Descriptor(IdType id,
                       const common::UUID& type,
                       const att::AccessRequirements& read_permissions,
                       const att::AccessRequirements& write_permissions)
    : id_(id),
      type_(type),
      read_permissions_(read_permissions),
      write_permissions_(write_permissions) {}

}  // namespace gatt
}  // namespace btlib
