// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_NETSTACK_C_NETCONFIG_H_
#define LIB_NETSTACK_C_NETCONFIG_H_

// This is a temporary API to access network configuration information. It will
// be replaced by a FIDL interface to the network stack.

// clang-format off

#include <sys/socket.h>

#include <zircon/device/ioctl.h>
#include <zircon/device/ioctl-wrapper.h>
#include <zircon/types.h>

__BEGIN_CDECLS

#define NETC_IFNAME_SIZE 16
#define NETC_HWADDR_SIZE 8
#define NETC_IF_INFO_MAX 16

typedef struct {
  char name[NETC_IFNAME_SIZE]; // null-terminated
  struct sockaddr_storage addr;
  struct sockaddr_storage netmask;
  struct sockaddr_storage broadaddr;
  uint32_t flags;
  uint16_t index;
  uint16_t hwaddr_len;
  uint8_t hwaddr[NETC_HWADDR_SIZE];
} netc_if_info_t;

#define NETC_IFF_UP 0x1

typedef struct {
  uint32_t n_info;
  netc_if_info_t info[NETC_IF_INFO_MAX];
} netc_get_if_info_t;

#define IOCTL_NETC_GET_IF_INFO \
    IOCTL(IOCTL_KIND_DEFAULT, IOCTL_FAMILY_NETCONFIG, 0)

// Get if info
// ssize_t ioctl_netc_get_if_info(int fd, netc_get_if_info_t* get_if_info)
IOCTL_WRAPPER_OUT(ioctl_netc_get_if_info, IOCTL_NETC_GET_IF_INFO, netc_get_if_info_t);

__END_CDECLS

#endif  // LIB_NETSTACK_C_NETCONFIG_H_
