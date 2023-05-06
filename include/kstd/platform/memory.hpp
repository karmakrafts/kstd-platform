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

#pragma once

#ifdef PLATFORM_APPLE
    #include <malloc/malloc.h>
#else

    #include <malloc.h>

#endif

#ifndef PLATFORM_WINDOWS

    #include <sys/mman.h>

#endif

#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>
#include "kstd/types.hpp"

namespace kstd::platform {
    // @formatter:off
    [[nodiscard]] inline auto allocate_aligned(usize size, usize alignment) noexcept -> void* {
        #ifdef PLATFORM_WINDOWS
        return _aligned_malloc(size, alignment);
        #else
        auto* memory = aligned_alloc(alignment, size + sizeof(usize));
        *reinterpret_cast<usize*>(memory) = size; // Store usable size before user data
        return reinterpret_cast<u8*>(memory) + sizeof(usize);
        #endif
    } // @formatter:on

    // @formatter:off
    template<typename T, typename... ARGS>
    [[nodiscard]] inline auto allocate(ARGS&& ... args) noexcept -> T* {
        static_assert(!std::is_void<T>::value, "Type must not be void");
        constexpr auto alignment = alignof(T);
        constexpr auto size = sizeof(T);
        #ifdef PLATFORM_WINDOWS
        auto* memory = reinterpret_cast<T*>(_aligned_malloc(size, alignment));
        new (memory) T(std::forward<ARGS>(args)...);
        return memory;
        #else
        auto* memory = aligned_alloc(alignment, size + sizeof(usize));
        *reinterpret_cast<usize*>(memory) = size; // Store usable size before user data
        auto* user_ptr = reinterpret_cast<T*>(reinterpret_cast<u8*>(memory) + sizeof(usize));
        new (user_ptr) T(std::forward<ARGS>(args)...);
        return user_ptr;
        #endif
    } // @formatter:on

    // @formatter:off
    inline auto free_aligned(void* memory) noexcept -> void {
        #ifdef PLATFORM_WINDOWS
        _aligned_free(memory);
        #else
        free(reinterpret_cast<u8*>(memory) - sizeof(usize));
        #endif
    } // @formatter:on

    // @formatter:off
    template<typename T>
    inline auto free(T* memory) noexcept -> void {
        static_assert(!std::is_void<T>::value, "Type must not be void");
        memory->~T();
        free_aligned(memory);
    } // @formatter:on

    // @formatter:off
    [[nodiscard]] inline auto realloc_aligned(void* memory, usize size, usize alignment) noexcept -> void* {
        #ifdef PLATFORM_WINDOWS
        return _aligned_realloc(memory, size, alignment);
        #else
        auto* new_memory = allocate_aligned(size, alignment);
        std::memcpy(new_memory, memory, *reinterpret_cast<usize*>(reinterpret_cast<u8*>(memory) - sizeof(usize)));
        free_aligned(memory);
        return new_memory;
        #endif
    } // @formatter:on
}