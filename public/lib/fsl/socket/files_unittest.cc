// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/fsl/socket/files.h"

#include <fcntl.h>
#include <zx/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"
#include "lib/fxl/files/file.h"
#include "lib/fxl/files/scoped_temp_dir.h"
#include "lib/fsl/socket/strings.h"
#include "lib/fsl/tasks/message_loop.h"

namespace fsl {
namespace {

TEST(SocketAndFile, CopyToFileDescriptor) {
  files::ScopedTempDir tmp_dir;
  std::string tmp_file;
  tmp_dir.NewTempFile(&tmp_file);
  MessageLoop message_loop;

  fxl::UniqueFD destination(open(tmp_file.c_str(), O_WRONLY));
  EXPECT_TRUE(destination.is_valid());

  bool success;
  CopyToFileDescriptor(
      fsl::WriteStringToSocket("Hello"), std::move(destination),
      message_loop.async(),
      [&message_loop, &success](bool success_value, fxl::UniqueFD fd) {
        success = success_value;
        message_loop.PostQuitTask();
      });
  message_loop.Run();

  EXPECT_TRUE(success);
  std::string content;
  EXPECT_TRUE(files::ReadFileToString(tmp_file, &content));
  EXPECT_EQ("Hello", content);
}

TEST(SocketAndFile, CopyFromFileDescriptor) {
  files::ScopedTempDir tmp_dir;
  std::string tmp_file;
  tmp_dir.NewTempFile(&tmp_file);
  MessageLoop message_loop;

  files::WriteFile(tmp_file, "Hello", 5);
  fxl::UniqueFD source(open(tmp_file.c_str(), O_RDONLY));
  EXPECT_TRUE(source.is_valid());

  zx::socket socket1, socket2;
  EXPECT_EQ(ZX_OK, zx::socket::create(0u, &socket1, &socket2));

  bool success;
  CopyFromFileDescriptor(
      std::move(source), std::move(socket1), message_loop.async(),
      [&message_loop, &success](bool success_value, fxl::UniqueFD fd) {
        success = success_value;
        message_loop.PostQuitTask();
      });
  message_loop.Run();

  EXPECT_TRUE(success);
  std::string content;
  EXPECT_TRUE(fsl::BlockingCopyToString(std::move(socket2), &content));
  EXPECT_EQ("Hello", content);
}

}  // namespace
}  // namespace fsl
