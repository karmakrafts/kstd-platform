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
#define LIB_C_NAME "msvcrt.dll"
#elif defined(PLATFORM_APPLE)
#define LIB_C_NAME "libSystem.dylib"
#else
#define LIB_C_NAME "libc.so.6"
#endif

TEST(kstd_platform_DynamicLibrary, TestLoadUnload) {
    auto lib = kstd::platform::DynamicLib(LIB_C_NAME);
    ASSERT_NO_THROW(lib.load().get());
    ASSERT_NO_THROW(lib.unload().get());
}

TEST(kstd_platform_DynamicLibrary, TestCallFunction) {
    auto lib = kstd::platform::DynamicLib(LIB_C_NAME);
    ASSERT_NO_THROW(lib.load().get());

    auto result = lib.get_function<kstd::i32, const char*, const char*>("printf");
    ASSERT_TRUE(result);
    (*result)("%s", "Hello World!\n");

    ASSERT_NO_THROW(lib.unload().get());
}