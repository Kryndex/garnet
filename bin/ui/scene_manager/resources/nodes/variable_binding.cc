// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "garnet/bin/ui/scene_manager/resources/nodes/variable_binding.h"

namespace scene_manager {

template <scenic::Value::Tag VT, typename T>
TypedVariableBinding<VT, T>::TypedVariableBinding(
    fxl::RefPtr<TypedVariable<VT, T>> variable,
    std::function<void(T value)> on_value_changed_callback)
    : variable_(variable),
      on_value_changed_callback_(on_value_changed_callback) {
  on_value_changed_callback_(variable_->value());
  variable_->AddListener(this);
}

template <scenic::Value::Tag VT, typename T>
TypedVariableBinding<VT, T>::~TypedVariableBinding() {
  variable_->RemoveListener(this);
}

template <scenic::Value::Tag VT, typename T>
void TypedVariableBinding<VT, T>::OnVariableValueChanged(
    TypedVariable<VT, T>* v) {
  on_value_changed_callback_(v->value());
}

template class TypedVariableBinding<scenic::Value::Tag::VECTOR3, escher::vec3>;
template class TypedVariableBinding<scenic::Value::Tag::QUATERNION,
                                    escher::quat>;

}  // namespace scene_manager
