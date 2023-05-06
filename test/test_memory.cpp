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
#include <kstd/platform/memory.hpp>

TEST(kstd_platform, TestAllocAligned) {
    auto* memory = reinterpret_cast<kstd::i32*>(kstd::platform::allocate_aligned(sizeof(kstd::i32), kstd::platform::get_min_alignment()));
    ASSERT_TRUE(memory != nullptr);

    *memory = 1337;
    ASSERT_EQ(*memory, 1337);

    kstd::platform::free_aligned(memory);
}

TEST(kstd_platform, TestReAllocAligned) {
    auto* memory = reinterpret_cast<kstd::i32*>(kstd::platform::allocate_aligned(sizeof(kstd::i32), kstd::platform::get_min_alignment()));
    ASSERT_TRUE(memory != nullptr);

    *memory = 1337;
    ASSERT_EQ(*memory, 1337);

    memory = reinterpret_cast<kstd::i32*>(kstd::platform::realloc_aligned(memory, sizeof(kstd::i32) << 1, kstd::platform::get_min_alignment()));
    ASSERT_TRUE(memory != nullptr);

    *memory = 444444;
    *(memory + 1) = 1337;
    ASSERT_EQ(*memory, 444444);
    ASSERT_EQ(*(memory + 1), 1337);

    kstd::platform::free_aligned(memory);
}

TEST(kstd_platform, TestAlloc) {
    auto* memory = kstd::platform::allocate<kstd::i32>();
    *memory = 1337;
    ASSERT_EQ(*memory, 1337);
    kstd::platform::free(memory);
}