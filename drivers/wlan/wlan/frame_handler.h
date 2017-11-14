// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "mac_frame.h"

#include "lib/wlan/fidl/wlan_mlme.fidl-common.h"

#include <ddk/protocol/wlan.h>
#include <zircon/types.h>

#define WLAN_DECL_VIRT_FUNC_HANDLE(methodName, args...) \
    virtual zx_status_t methodName(args) { return ZX_OK; }

#define WLAN_DECL_FUNC_HANDLE_MLME(methodName, mlmeMsgType) \
    WLAN_DECL_VIRT_FUNC_HANDLE(methodName, const mlmeMsgType&)

#define WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(methodName, mlmeMsgType)          \
    zx_status_t HandleFrameInternal(Method& method, const mlmeMsgType& msg) { \
        if (ShouldDropMlmeMessage(method)) { return ZX_ERR_STOP; }            \
        methodName(msg);                                                      \
        return ZX_OK;                                                         \
    }

#define WLAN_DECL_FUNC_HANDLE_MGMT(mgmtFrameType)                                      \
    WLAN_DECL_VIRT_FUNC_HANDLE(Handle##mgmtFrameType, const MgmtFrame<mgmtFrameType>&, \
                               const wlan_rx_info_t&)

#define WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(mgmtFrameType)                 \
    zx_status_t HandleFrameInternal(const MgmtFrame<mgmtFrameType>& frame, \
                                    const wlan_rx_info_t& info) {          \
        if (ShouldDropMgmtFrame(*frame.hdr)) { return ZX_ERR_STOP; }       \
        Handle##mgmtFrameType(frame, info);                                \
        return ZX_OK;                                                      \
    }

#define WLAN_DECL_VIRT_FUNC_HANDLE_DATA(methodName, args...) \
    WLAN_DECL_VIRT_FUNC_HANDLE(Handle##methodName, args, const wlan_rx_info_t&)

namespace wlan {

// FrameHandler provides frame handling capabilities. It is not thread safe.
// By default the FrameHandler will ignore every frame. If the component wishes to handle a
// specific frame it must override the corresponding method, for example HandleBeacon(...). The
// FrameHandler also offers to provide custom logic for dropping frames in a centralized manner
// by overriding ShouldDropXYZ(...). Using these methods for dropping results in two benefits,
// (1) frame filtering logic is in one single place rather than being spread out through
// various HandleXYZ(...) methods, and
// (2) dropped frames will not be forwarded to forwarding targets.
// Children are targets which frames are automatically forwarded to.
// Frames are only forwarded to the children if they were not dropped by the parent, and
// the parent did not fail processing the frame itself.
// Frame processing errors returned by children are logged but have no effect onto their
// siblings or the parent.
class FrameHandler {
   public:
    FrameHandler() {}
    virtual ~FrameHandler() {}

    template <typename... Args> zx_status_t HandleFrame(Args&&... args) {
        auto status = HandleFrameInternal(std::forward<Args>(args)...);
        // Do not forward frame if it was dropped.
        if (status == ZX_ERR_STOP) { return ZX_OK; }
        // Do not forward frame if processing failed.
        if (status != ZX_OK) { return status; }

        // Forward frame to all children.
        uint16_t i = 0;
        for (auto child = children_.begin(); child != children_.end();) {
            if (auto handler = child->lock()) {
                status = handler->HandleFrame(std::forward<Args>(args)...);
                // Log when a child failed processing a frame, but proceed.
                if (status != ZX_OK) {
                    debugfhandler("child %u failed handling frame: %d\n", i, status);
                }
                child++;
                i++;
            } else {
                // Handler expired, remove from list.
                child = children_.erase(child);
            }
        }

        return ZX_OK;
    }

    void AddChildHandler(std::weak_ptr<FrameHandler> ptr) { children_.push_back(ptr); }

   protected:
    // Ethernet frame handlers.
    virtual bool ShouldDropEthFrame(const BaseFrame<EthernetII>& frame) { return false; }
    WLAN_DECL_VIRT_FUNC_HANDLE(HandleEthFrame, const BaseFrame<EthernetII>& frame)

    // Service Message handlers.
    virtual bool ShouldDropMlmeMessage(Method& method) { return false; }
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeScanReq, ScanRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeJoinReq, JoinRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeAuthReq, AuthenticateRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeDeauthReq, DeauthenticateRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeAssocReq, AssociateRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeEapolReq, EapolRequest)
    WLAN_DECL_FUNC_HANDLE_MLME(HandleMlmeSetKeysReq, SetKeysRequest)

    // Data frame handlers.
    virtual bool ShouldDropDataFrame(const DataFrameHeader& hdr) { return false; }
    WLAN_DECL_VIRT_FUNC_HANDLE_DATA(NullDataFrame, const DataFrameHeader&)
    WLAN_DECL_VIRT_FUNC_HANDLE_DATA(DataFrame, const DataFrame<LlcHeader>&)

    // Management frame handlers.
    virtual bool ShouldDropMgmtFrame(const MgmtFrameHeader& hdr) { return false; }
    WLAN_DECL_FUNC_HANDLE_MGMT(Beacon)
    WLAN_DECL_FUNC_HANDLE_MGMT(ProbeResponse)
    WLAN_DECL_FUNC_HANDLE_MGMT(Authentication)
    WLAN_DECL_FUNC_HANDLE_MGMT(Deauthentication)
    WLAN_DECL_FUNC_HANDLE_MGMT(AssociationResponse)
    WLAN_DECL_FUNC_HANDLE_MGMT(Disassociation)
    WLAN_DECL_FUNC_HANDLE_MGMT(AddBaRequestFrame)

   private:
    // Internal Service Message handlers.
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeScanReq, ScanRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeJoinReq, JoinRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeAuthReq, AuthenticateRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeDeauthReq, DeauthenticateRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeAssocReq, AssociateRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeEapolReq, EapolRequest)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MLME(HandleMlmeSetKeysReq, SetKeysRequest)

    // Internal Management frame handlers.
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(Beacon)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(ProbeResponse)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(Authentication)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(Deauthentication)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(AssociationResponse)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(Disassociation)
    WLAN_DECL_FUNC_INTERNAL_HANDLE_MGMT(AddBaRequestFrame)

    zx_status_t HandleFrameInternal(const BaseFrame<EthernetII>& frame) {
        if (ShouldDropEthFrame(frame)) { return ZX_ERR_NOT_SUPPORTED; }
        HandleEthFrame(frame);
        return ZX_OK;
    }

    zx_status_t HandleFrameInternal(const DataFrameHeader& hdr, const wlan_rx_info_t& rxinfo) {
        if (ShouldDropDataFrame(hdr)) { return ZX_ERR_NOT_SUPPORTED; }
        HandleNullDataFrame(hdr, rxinfo);
        return ZX_OK;
    }

    zx_status_t HandleFrameInternal(const DataFrame<LlcHeader>& frame,
                                    const wlan_rx_info_t& rxinfo) {
        if (ShouldDropDataFrame(*frame.hdr)) { return ZX_ERR_NOT_SUPPORTED; }
        HandleDataFrame(frame, rxinfo);
        return ZX_OK;
    }

    std::vector<std::weak_ptr<FrameHandler>> children_;
};

}  // namespace wlan