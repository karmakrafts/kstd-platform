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

#pragma once

#include <cstdlib>
#include <cstring>
#include <kstd/types.hpp>
#include <memory>

#ifdef PLATFORM_APPLE
#include <malloc/malloc.h>
#else

#include <malloc.h>

#endif

#ifndef PLATFORM_WINDOWS
#include <sys/mman.h>
#endif

namespace kstd::platform {
    [[nodiscard]] inline auto get_usable_size(void* memory) noexcept -> usize {
#if defined(PLATFORM_WINDOWS)
        return _msize(memory);
#elif defined(PLATFORM_APPLE)
        return malloc_size(memory);
#else
        return malloc_usable_size(memory) - sizeof(void*);
#endif
    }

    [[nodiscard]] inline auto alloc_aligned(usize size, usize alignment) noexcept -> void* {
#ifdef PLATFORM_WINDOWS
        return _aligned_malloc(size, alignment);
#else
        auto* memory = aligned_alloc(alignment, size + sizeof(usize));
        *reinterpret_cast<usize*>(memory) = size;// Store usable size before user data
        return reinterpret_cast<u8*>(memory) + sizeof(usize);
#endif
    }

    [[nodiscard]] inline auto free_aligned(void* memory) noexcept {
#ifdef PLATFORM_WINDOWS
        _aligned_free(memory);
#else
        free(reinterpret_cast<u8*>(memory) - sizeof(usize));
#endif
    }

    [[nodiscard]] inline auto realloc_aligned(void* memory, usize old_size, usize size, usize alignment) noexcept
            -> void* {
#ifdef PLATFORM_WINDOWS
        return _aligned_realloc(memory, size, alignment);
#else
        auto* new_memory = alloc_aligned(size, alignment);
        std::memcpy(new_memory, memory, old_size);
        free_aligned(memory);
        return new_memory;
#endif
    }
}// namespace kstd::platform