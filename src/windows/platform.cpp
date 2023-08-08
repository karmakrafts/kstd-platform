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
 * @since 08/08/2023
 */

#ifdef PLATFORM_WINDOWS

#include "kstd/platform/platform.hpp"

namespace kstd::platform {
    auto get_platform() noexcept -> Platform {
        return Platform::WINDOWS;
    }

    auto get_last_error() noexcept -> std::string {
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
    }

    auto get_page_size() noexcept -> usize {
        SYSTEM_INFO info {};
        ::GetSystemInfo(&info);
        return static_cast<usize>(info.dwPageSize);
    }
}// namespace kstd::platform

#endif// PLATFORM_WINDOWS