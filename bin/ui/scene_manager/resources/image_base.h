// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GARNET_BIN_UI_SCENE_MANAGER_RESOURCES_IMAGE_BASE_H_
#define GARNET_BIN_UI_SCENE_MANAGER_RESOURCES_IMAGE_BASE_H_

#include "garnet/bin/ui/scene_manager/engine/session.h"
#include "garnet/bin/ui/scene_manager/resources/resource.h"

namespace scene_manager {

// Abstract superclass for Image and ImagePipe.
class ImageBase : public Resource {
 public:
  static const ResourceTypeInfo kTypeInfo;

  // Returns the image that should currently be presented. Can be null.
  virtual const escher::ImagePtr& GetEscherImage() = 0;

 protected:
  ImageBase(Session* session,
            scenic::ResourceId id,
            const ResourceTypeInfo& type_info);
};

using ImageBasePtr = fxl::RefPtr<ImageBase>;

}  // namespace scene_manager

#endif  // GARNET_BIN_UI_SCENE_MANAGER_RESOURCES_IMAGE_BASE_H_
