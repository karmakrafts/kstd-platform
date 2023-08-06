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
 * @since 06/05/2023
 */

#include <gtest/gtest.h>
#include <kstd/platform/dynamic_lib.hpp>

#if defined(PLATFORM_WINDOWS)
constexpr auto lib_name = "msvcrt.dll";
#elif defined(PLATFORM_APPLE)
constexpr auto lib_name = "libSystem.dylib";
#else
constexpr auto lib_name = "libc.so.6";
#endif

TEST(kstd_platform_DynamicLibrary, test_load_unload) {
    auto lib = kstd::platform::DynamicLib(lib_name);
}

TEST(kstd_platform_DynamicLibrary, test_call_function) {
    auto lib = kstd::platform::DynamicLib(lib_name);

    auto result = lib.get_function<kstd::i32, const char*, const char*>("printf");
    ASSERT_TRUE(result);
    (*result)("%s", "Hello World!\n");
}