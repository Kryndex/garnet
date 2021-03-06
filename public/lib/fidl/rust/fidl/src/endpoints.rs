// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Wrapper types for the endpoints of a connection.

use std::marker::PhantomData;

use zircon;
use {FidlService, Encodable, Decodable, EncodableNullable, DecodableNullable, encode_handle,
    decode_handle, EncodeBuf, DecodeBuf, EncodableType, Result};

/// The `Client` end of a FIDL connection.
#[derive(Eq, PartialEq, Hash)]
pub struct ClientEnd<T> {
    inner: zircon::Channel,
    phantom: PhantomData<T>,
}

impl<T> ClientEnd<T> {
    /// Create a new client from the provided channel.
    pub fn new(inner: zircon::Channel) -> Self {
        ClientEnd { inner, phantom: PhantomData }
    }

    /// Extract the inner channel.
    pub fn into_channel(self) -> zircon::Channel {
        self.inner
    }
}

impl<T> zircon::AsHandleRef for ClientEnd<T> {
    fn as_handle_ref(&self) -> zircon::HandleRef {
        self.inner.as_handle_ref()
    }
}

impl<T> From<ClientEnd<T>> for zircon::Handle {
    fn from(client: ClientEnd<T>) -> zircon::Handle {
        client.into_channel().into()
    }
}

impl<T> From<zircon::Handle> for ClientEnd<T> {
    fn from(handle: zircon::Handle) -> Self {
        ClientEnd {
            inner: handle.into(),
            phantom: PhantomData,
        }
    }
}

impl<T: FidlService> ::std::fmt::Debug for ClientEnd<T> {
    fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        write!(f, "ClientEnd(name={}, version={}, channel={:?})", T::NAME, T::VERSION, self.inner)
    }
}

impl<T> zircon::HandleBased for ClientEnd<T> {}

impl<T> Encodable for ClientEnd<T> {
    fn encode(self, buf: &mut EncodeBuf, base: usize, offset: usize) {
        encode_handle(self.into(), buf, base, offset);
    }
    fn encodable_type() -> EncodableType {
        EncodableType::Handle
    }
    fn size() -> usize {
        4
    }
}

impl<T> EncodableNullable for ClientEnd<T> {
    type NullType = u32;
    fn null_value() -> u32 { !0u32 }
}

impl<T> Decodable for ClientEnd<T> {
    fn decode(buf: &mut DecodeBuf, base: usize, offset: usize) -> Result<Self> {
        decode_handle(buf, base, offset).map(From::from)
    }
}

impl<T> DecodableNullable for ClientEnd<T> {}


/// The `Server` end of a FIDL connection.
#[derive(Eq, PartialEq, Hash)]
pub struct ServerEnd<T> {
    inner: zircon::Channel,
    phantom: PhantomData<T>,
}

impl<T> ServerEnd<T> {
    /// Create a new `ServerEnd` from the provided channel.
    pub fn new(inner: zircon::Channel) -> ServerEnd<T> {
        ServerEnd { inner, phantom: PhantomData }
    }

    /// Extract the inner channel.
    pub fn into_channel(self) -> ::zircon::Channel {
        self.inner
    }
}

impl<T> zircon::AsHandleRef for ServerEnd<T> {
    fn as_handle_ref(&self) -> zircon::HandleRef {
        self.inner.as_handle_ref()
    }
}

impl<T> From<ServerEnd<T>> for zircon::Handle {
    fn from(server: ServerEnd<T>) -> zircon::Handle {
        server.into_channel().into()
    }
}

impl<T> From<zircon::Handle> for ServerEnd<T> {
    fn from(handle: zircon::Handle) -> Self {
        ServerEnd {
            inner: handle.into(),
            phantom: PhantomData,
        }
    }
}

impl<T: FidlService> ::std::fmt::Debug for ServerEnd<T> {
    fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        write!(f, "ServerEnd(name={}, version={}, channel={:?})", T::NAME, T::VERSION, self.inner)
    }
}

impl<T> zircon::HandleBased for ServerEnd<T> {}

impl<T> Encodable for ServerEnd<T> {
    fn encode(self, buf: &mut EncodeBuf, base: usize, offset: usize) {
        encode_handle(self.into(), buf, base, offset);
    }
    fn encodable_type() -> EncodableType {
        EncodableType::Handle
    }
    fn size() -> usize {
        4
    }
}

impl<T> EncodableNullable for ServerEnd<T> {
    type NullType = u32;
    fn null_value() -> u32 { !0u32 }
}

impl<T> Decodable for ServerEnd<T> {
    fn decode(buf: &mut DecodeBuf, base: usize, offset: usize) -> Result<Self> {
        decode_handle(buf, base, offset).map(From::from)
    }
}

impl<T> DecodableNullable for ServerEnd<T> {}
