// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <deque>
#include <memory>
#include <set>
#include <thread>

#include <dispatcher-pool/dispatcher-execution-domain.h>
#include <dispatcher-pool/dispatcher-wakeup-event.h>
#include <fbl/intrusive_double_list.h>
#include <fbl/ref_counted.h>
#include <fbl/ref_ptr.h>

#include "garnet/bin/media/audio_server/audio_object.h"
#include "garnet/bin/media/audio_server/audio_pipe.h"
#include "garnet/bin/media/audio_server/audio_renderer_impl.h"
#include "garnet/bin/media/audio_server/fwd_decls.h"
#include "lib/fxl/synchronization/mutex.h"
#include "lib/fxl/synchronization/thread_annotations.h"
#include "lib/fxl/tasks/task_runner.h"
#include "lib/fxl/time/time_point.h"

namespace media {
namespace audio {

class AudioDevice : public AudioObject,
                    public fbl::DoublyLinkedListable<fbl::RefPtr<AudioDevice>> {
 public:
  // Wakeup
  //
  // Called from outside the mixing ExecutionDomain to cause an
  // AudioDevice's::OnWakeup handler to run from within the context of the
  // mixing execution domain.
  void Wakeup();

  // Accessors to check to see if this is an AudioInput or AudioOutput
  bool is_input() const { return type() == Type::Input; }
  bool is_output() const { return type() == Type::Output; }

  // Accessors for the current plug state of the device.
  //
  // In addition to publishing and unpublishing streams when codecs are
  // attached/removed to/from hot pluggable buses (such as USB), some codecs
  // have the ability to detect the plugged or unplugged state of external
  // connectors (such as a 3.5mm audio jack).  Drivers can report this
  // plugged/unplugged state as well as the time of the last state change.
  // Currently this information is used in the Audio Server to implement simple
  // routing policies for AudioRenderers and AudioCapturers.
  //
  // plugged   : true when an audio output stream is either hardwired, or
  //             believes that it has something connected to its plug.
  // plug_time : The last time (according to zx_time_get(ZX_CLOCK_MONOTONIC) at
  //             which the plugged/unplugged state of the output stream last
  //             changed.
  bool plugged() const { return plugged_; }
  zx_time_t plug_time() const { return plug_time_; }

 protected:
  friend class fbl::RefPtr<AudioDevice>;
  ~AudioDevice() override;
  AudioDevice(Type type, AudioDeviceManager* manager);

  //////////////////////////////////////////////////////////////////////////////
  //
  // Methods which may be implemented by derived classes to customize behavior.
  //
  //////////////////////////////////////////////////////////////////////////////

  // Init
  //
  // Called during startup on the AudioServer's main message loop thread.  No
  // locks are being held at this point.  Derived classes should allocate their
  // hardware resources and initialize any internal state.  Return
  // MediaResult::OK if everything is good and the output is ready to do work.
  virtual MediaResult Init();

  // Unlink
  //
  // Called from AudioDeviceManager (either directly, or indirectly from
  // Shutdown).  AudioDevice implementations should use this hook to unlink
  // themselves from all of the other audio objects they are currently linked
  // to.
  virtual void Unlink() = 0;

  // Cleanup
  //
  // Called at shutdown on the AudioServer's main message loop thread to allow
  // derived classes to clean up any allocated resources.  All pending
  // processing callbacks have either been nerfed or run till completion.  All
  // audio other objects have been disconnected/unlinked.  No locks are being
  // held.
  virtual void Cleanup();

  //////////////////////////////////////////////////////////////////////////////
  //
  // Methods which may used by derived classes from within the context of a
  // mix_domain_ ExecutionDomain.  Note; since these methods are intended to be
  // called from the within the mix_domain_, callers must be annotated properly
  // to demonstrate that they are executing from within that domain.
  //

  // OnWakeup
  //
  // Called in response to someone from outside the domain poking the
  // mix_wakeup_ WakeupEvent.  At a minimum, the framework will call this once
  // at startup to get the output running.
  virtual void OnWakeup()
      FXL_EXCLUSIVE_LOCKS_REQUIRED(mix_domain_->token()) = 0;

  // ShutdownSelf
  //
  // Kick off the process of shooting ourselves in the head.  Note, after this
  // method has been called, no new callbacks may be scheduled.  As soon as the
  // main message loop finds out about our shutdown request, it will complete
  // the process of shutting us down, unlinking us from our renderers and
  // calling the Cleanup method.
  void ShutdownSelf() FXL_EXCLUSIVE_LOCKS_REQUIRED(mix_domain_->token());

  // Check the shutting down flag.  We are in the process of shutting down when
  // we have become deactivated at the dispatcher framework level.
  inline bool is_shutting_down() const {
    return (!mix_domain_ || mix_domain_->deactivated());
  }

  //////////////////////////////////////////////////////////////////////////////
  //
  // Other methods.
  //

  // UpdatePlugState
  //
  // Called by the audio output manager on the main message loop when it has
  // been notified of a plug state change for the output.  Used to update the
  // internal bookkeeping about the current plugged/unplugged state.  This
  // method may also be used by derived classes during Init to set an initial
  // plug state.
  //
  // Returns true if the plug state has changed, or false otherwise.
  bool UpdatePlugState(bool plugged, zx_time_t plug_time);

  AudioDeviceManager* manager_;
  fxl::Mutex mutex_;

  // State used to manage asynchronous processing using the dispatcher
  // framework.
  fbl::RefPtr<::audio::dispatcher::ExecutionDomain> mix_domain_;
  fbl::RefPtr<::audio::dispatcher::WakeupEvent> mix_wakeup_;

 private:
  // It's always nice when you manager is also your friend.  Seriously though,
  // the AudioDeviceManager gets to call Startup and Shutdown, no one else
  // (including derived classes) should be able to.
  friend class AudioDeviceManager;

  // DeactivateDomain
  //
  // deactivate our execution domain (if it exists) and synchronize with any
  // operations taking place in the domain.
  void DeactivateDomain() FXL_LOCKS_EXCLUDED(mix_domain_->token());

  // Called from the AudioDeviceManager after an output has been created.
  // Gives derived classes a chance to set up hardware, then sets up the
  // machinery needed for scheduling processing tasks and schedules the first
  // processing callback immediately in order to get the process running.
  MediaResult Startup();

  // Called from the AudioDeviceManager on the main message loop
  // thread.  Makes certain that the process of shutdown has started,
  // synchronizes with any processing tasks which were executing at the time,
  // then finishes the shutdown process by unlinking from all renderers and
  // cleaning up all resources.
  void Shutdown();

  // Plug state is protected by the fact that it is only ever accessed on the
  // main message loop thread.
  bool plugged_ = false;
  zx_time_t plug_time_ = 0;

  volatile bool shut_down_ = false;
};

}  // namespace audio
}  // namespace media