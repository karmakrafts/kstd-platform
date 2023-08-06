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

#else

#include <fcntl.h>
#include <unistd.h>


#endif

#include <fmt/format.h>
#include <string>

namespace kstd::platform {
#if defined(PLATFORM_WINDOWS)
    using NativeFileHandle = HANDLE;
    using NativeModuleHandle = HMODULE;
    using NativeOffset = isize;
#elif defined(PLATFORM_APPLE)
    using NativeFileHandle = i32;
    using NativeModuleHandle = void*;
    using NativeOffset = off_t;
#else
    using NativeFileHandle = i32;
    using NativeModuleHandle = void*;
#ifdef CPU_64_BIT
    using NativeOffset = __off64_t;
#else
    using NativeOffset = __off_t;
#endif
#endif

#ifdef PLATFORM_WINDOWS
    constexpr usize max_path = MAX_PATH;
    constexpr NativeFileHandle invalid_file_handle = INVALID_HANDLE_VALUE;
#else
    constexpr usize max_path = PATH_MAX;
    constexpr NativeFileHandle invalid_file_handle = -1;
#endif

    constexpr NativeModuleHandle invalid_module_handle = nullptr;// Placeholder for now

    enum class Platform : u8 {
        WINDOWS,
        LINUX,
        MACOS
    };

    [[nodiscard]] inline auto get_platform() noexcept -> Platform {
#if defined(PLATFORM_WINDOWS)
        return Platform::WINDOWS;
#elif defined(PLATFORM_APPLE)
        return Platform::MACOS;
#else
        return Platform::LINUX;
#endif
    }

    [[nodiscard]] inline auto get_platform_name(Platform platform) noexcept -> std::string {
        switch(platform) {
            case Platform::WINDOWS: return "Windows";
            case Platform::MACOS: return "MacOS";
            case Platform::LINUX: return "Linux";
            default: return "Unknown";
        }
    }

    [[nodiscard]] inline auto get_last_error() noexcept -> std::string {
#ifdef PLATFORM_WINDOWS
        const auto error_code = ::GetLastError();

        if(error_code == 0) {
            return "";
        }

        LPWSTR buffer = nullptr;
        constexpr auto lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
        const auto new_length = ::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                error_code, lang_id, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
        auto message = utils::to_mbs({buffer, new_length});
        LocalFree(buffer);

        return fmt::format("ERROR 0x{:X}: {}", error_code, message);
#else
        return fmt::format("ERROR 0x{:X}: {}", errno, strerror(errno));
#endif
    }

    [[nodiscard]] inline auto get_page_size() noexcept -> usize {
#ifdef PLATFORM_WINDOWS
        SYSTEM_INFO info {};
        ::GetSystemInfo(&info);
        return static_cast<usize>(info.dwPageSize);
#else
        return static_cast<usize>(::sysconf(_SC_PAGESIZE));
#endif
    }
}// namespace kstd::platform