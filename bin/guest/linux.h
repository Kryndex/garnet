// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GARNET_BIN_GUEST_LINUX_H_
#define GARNET_BIN_GUEST_LINUX_H_

#include <zircon/types.h>

zx_status_t setup_linux(const uintptr_t addr,
                        const size_t size,
                        const uintptr_t first_page,
                        const int fd,
                        const char* initrd_path,
                        const char* cmdline,
                        uintptr_t* guest_ip,
                        uintptr_t* boot_ptr);

#endif  // GARNET_BIN_GUEST_LINUX_H_
