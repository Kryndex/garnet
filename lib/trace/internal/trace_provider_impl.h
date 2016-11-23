// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_TRACING_LIB_TRACE_INTERNAL_TRACE_PROVIDER_IMPL_H_
#define APPS_TRACING_LIB_TRACE_INTERNAL_TRACE_PROVIDER_IMPL_H_

#include <memory>
#include <string>
#include <vector>

#include <mx/eventpair.h>
#include <mx/vmo.h>

#include "apps/tracing/lib/trace/settings.h"
#include "apps/tracing/services/trace_provider.fidl.h"
#include "apps/tracing/services/trace_registry.fidl.h"
#include "lib/fidl/cpp/bindings/binding.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/weak_ptr.h"

namespace tracing {
namespace internal {

class TraceProviderImpl : public TraceProvider {
 public:
  TraceProviderImpl(TraceRegistryPtr registry, const TraceSettings& settings);
  ~TraceProviderImpl() override;

 private:
  // |TraceProvider|
  void Start(mx::vmo buffer,
             mx::eventpair fence,
             ::fidl::Array<::fidl::String> categories) override;
  void Stop() override;

  void StartPendingTrace();
  void FinishedTrace();

  TraceRegistryPtr registry_;
  fidl::Binding<TraceProvider> binding_;

  enum class State { kStarted, kStopping, kStopped };
  State state_ = State::kStopped;

  struct PendingTrace {
    mx::vmo buffer;
    mx::eventpair fence;
    std::vector<std::string> enabled_categories;
  };
  std::unique_ptr<PendingTrace> pending_trace_;

  ftl::WeakPtrFactory<TraceProviderImpl> weak_ptr_factory_;

  FTL_DISALLOW_COPY_AND_ASSIGN(TraceProviderImpl);
};

}  // namespace internal
}  // namespace tracing

#endif  // APPS_TRACING_LIB_TRACE_INTERNAL_TRACE_PROVIDER_IMPL_H_
