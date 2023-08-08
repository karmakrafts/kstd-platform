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

#include <kstd/types.hpp>
#include <kstd/utils.hpp>

#ifdef PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <sysinfoapi.h>
#include <Winsock2.h>
#include <WS2tcpip.h>

#else

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#endif

#include <fmt/format.h>
#include <string>

namespace kstd::platform {
#if defined(PLATFORM_WINDOWS)
    using NativeFileHandle = HANDLE;
    using NativeModuleHandle = HMODULE;
    using NativeSocketHandle = SOCKET;
    using NativeOffset = isize;
#elif defined(PLATFORM_APPLE)
    using NativeFileHandle = int;
    using NativeModuleHandle = void*;
    using NativeSocketHandle = int;
    using NativeOffset = off_t;
#else
    using NativeFileHandle = int;
    using NativeModuleHandle = void*;
    using NativeSocketHandle = int;
#ifdef CPU_64_BIT
    using NativeOffset = __off64_t;
#else
    using NativeOffset = __off_t;
#endif
#endif

#ifdef PLATFORM_WINDOWS
    static constexpr usize max_path = MAX_PATH;
    static inline const NativeFileHandle invalid_file_handle = INVALID_HANDLE_VALUE;
    static inline const NativeSocketHandle invalid_socket_handle = INVALID_SOCKET;
#else
    static constexpr usize max_path = PATH_MAX;
    static inline const NativeFileHandle invalid_file_handle = -1;
    static inline const NativeSocketHandle invalid_socket_handle = -1;
#endif

    static inline const NativeModuleHandle invalid_module_handle = nullptr;

    enum class Platform : u8 {
        WINDOWS,
        LINUX,
        MACOS
    };

    [[nodiscard]] inline auto get_platform_name(Platform platform) noexcept -> std::string {
        switch(platform) {
            case Platform::WINDOWS: return "Windows";
            case Platform::MACOS: return "MacOS";
            case Platform::LINUX: return "Linux";
            default: return "Unknown";
        }
    }

    [[nodiscard]] auto get_platform() noexcept -> Platform;

    [[nodiscard]] auto get_last_error() noexcept -> std::string;

    [[nodiscard]] auto get_page_size() noexcept -> usize;
}// namespace kstd::platform