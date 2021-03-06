/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "eden/fs/testharness/FakePrivHelper.h"

#include <folly/Conv.h>
#include <folly/File.h>
#include "eden/fs/testharness/FakeFuse.h"

using std::runtime_error;
using std::string;

namespace facebook {
namespace eden {

FakePrivHelper::FakePrivHelper() {}

void FakePrivHelper::registerMount(
    AbsolutePathPiece mountPath,
    std::shared_ptr<FakeFuse> fuse) {
  auto ret = mounts_.emplace(mountPath.stringPiece().str(), std::move(fuse));
  if (!ret.second) {
    throw std::range_error(
        folly::to<string>("mount ", mountPath, " already defined"));
  }
}

folly::File FakePrivHelper::fuseMount(folly::StringPiece mountPath) {
  auto iter = mounts_.find(mountPath.str());
  if (iter == mounts_.end()) {
    throw std::range_error(folly::to<string>(
        "got request to create FUSE mount ",
        mountPath,
        ", but no test FUSE endpoint defined for this path"));
  }
  auto& fakeFuse = iter->second;
  if (fakeFuse->isStarted()) {
    throw std::runtime_error(folly::to<string>(
        "got request to create FUSE mount ",
        mountPath,
        ", but this mount is already running"));
  }

  return fakeFuse->start();
}

void FakePrivHelper::fuseUnmount(folly::StringPiece /* mountPath */) {
  throw runtime_error("FakePrivHelper::fuseUnmount() not implemented");
}

void FakePrivHelper::bindMount(
    folly::StringPiece /* clientPath */,
    folly::StringPiece /* mountPath */) {
  throw runtime_error("FakePrivHelper::bindMount() not implemented");
}

void FakePrivHelper::fuseTakeoverShutdown(folly::StringPiece /* mountPath */) {
  throw runtime_error("FakePrivHelper::fuseTakeoverShutdown() not implemented");
}

void FakePrivHelper::fuseTakeoverStartup(
    folly::StringPiece /* mountPath */,
    const std::vector<std::string>& /* bindMounts */) {
  throw runtime_error("FakePrivHelper::fuseTakeoverStartup() not implemented");
}

int FakePrivHelper::stop() {
  return 0;
}

} // namespace eden
} // namespace facebook
