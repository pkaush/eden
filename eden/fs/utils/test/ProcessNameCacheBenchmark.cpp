/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/synchronization/Baton.h>
#include "eden/fs/utils/ProcessNameCache.h"

using namespace facebook::eden;

/**
 * A high but realistic amount of contention.
 */
constexpr size_t kThreadCount = 4;

BENCHMARK(ProcessNameCache_repeatedly_add_self, iters) {
  folly::BenchmarkSuspender suspender;

  ProcessNameCache processNameCache;
  std::vector<std::thread> threads;
  std::array<folly::Baton<>, kThreadCount> batons;

  size_t remainingIterations = iters;
  size_t totalIterations = 0;
  for (size_t i = 0; i < kThreadCount; ++i) {
    size_t remainingThreads = kThreadCount - i;
    size_t assignedIterations = remainingIterations / remainingThreads;
    remainingIterations -= assignedIterations;
    totalIterations += assignedIterations;
    threads.emplace_back([&processNameCache,
                          baton = &batons[i],
                          assignedIterations,
                          myPid = getpid()] {
      baton->wait();
      for (size_t j = 0; j < assignedIterations; ++j) {
        processNameCache.add(myPid);
      }
    });
  }

  CHECK_EQ(totalIterations, iters);

  suspender.dismiss();

  // Now wake the threads.
  for (auto& baton : batons) {
    baton.post();
  }

  // Wait until they're done.
  for (auto& thread : threads) {
    thread.join();
  }
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}