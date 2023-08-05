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
 * @since 02/07/2023
 */

#include <gtest/gtest.h>
#include <kstd/libc.hpp>
#include <kstd/platform/memory.hpp>

constexpr auto foo_alignment = sizeof(void*);

struct alignas(foo_alignment) Foo final {
    kstd::u16 x;
    kstd::u16 y;
};

TEST(kstd_platform, test_get_usable_size) {
    constexpr auto size = sizeof(void*) << 1;
    auto* memory = kstd::libc::malloc(size);// NOLINT
    ASSERT_TRUE(memory != nullptr);
    ASSERT_EQ(kstd::platform::mm::get_usable_size(memory), size);
    kstd::libc::free(memory);// NOLINT
}

TEST(kstd_platform, test_allocate_aligned) {
    constexpr auto size = sizeof(Foo);
    auto* memory = reinterpret_cast<Foo*>(kstd::platform::mm::alloc_aligned(size, foo_alignment));// NOLINT
    ASSERT_TRUE(memory != nullptr);

    memory->x = 1337;
    memory->y = 69;
    ASSERT_EQ(memory->x, 1337);
    ASSERT_EQ(memory->y, 69);

    kstd::platform::mm::free_aligned(memory);
}

TEST(kstd_platform, test_realloc_aligned) {
    constexpr auto size = sizeof(Foo);
    auto* memory = reinterpret_cast<Foo*>(kstd::platform::mm::alloc_aligned(size, foo_alignment));// NOLINT
    ASSERT_TRUE(memory != nullptr);

    memory->x = 1337;
    memory->y = 69;
    ASSERT_EQ(memory->x, 1337);
    ASSERT_EQ(memory->y, 69);

    constexpr auto new_size = size << 1;
    memory =
            reinterpret_cast<Foo*>(kstd::platform::mm::realloc_aligned(memory, size, new_size, foo_alignment));// NOLINT
    ASSERT_TRUE(memory != nullptr);

    ASSERT_EQ(memory->x, 1337);
    ASSERT_EQ(memory->y, 69);

    ++memory;// NOLINT
    memory->x = 444;
    memory->y = 222;
    ASSERT_EQ(memory->x, 444);
    ASSERT_EQ(memory->y, 222);

    --memory;// NOLINT
    ASSERT_EQ(memory->x, 1337);
    ASSERT_EQ(memory->y, 69);

    kstd::platform::mm::free_aligned(memory);
}