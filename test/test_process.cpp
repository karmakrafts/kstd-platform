// Copyright 2023 Karma Krafts & associates
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @author Alexander Hinze
 * @since 03/07/2023
 */

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <iostream>
#include <kstd/platform/process.hpp>

TEST(kstd_platform_Process, TestFind) {
}

TEST(kstd_platform_Process, TestCurrent) {
    auto proc = kstd::platform::Process::get_current();

    const auto id = proc.get_id();
    std::cout << fmt::format("Current PID: {}\n", id);
    ASSERT_NE(id, 0);
}

TEST(kstd_platform_Process, TestGetPath) {
    auto proc = kstd::platform::Process::get_current();

    auto path_result = proc.get_path();
    ASSERT_TRUE(path_result);
    ASSERT_TRUE(!path_result->empty());

    std::cout << fmt::format("Current Path: {}\n", path_result->c_str());
}

TEST(kstd_platform_Process, TestOpenHandle) {
}