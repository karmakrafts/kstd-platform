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

#pragma once

#include "platform.hpp"
#include <filesystem>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>

#ifdef PLATFORM_WINDOWS
#include <processthreadsapi.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace kstd::platform {
#ifdef PLATFORM_WINDOWS
    using NativeProcessId = DWORD;
    using NativeProcessHandle = HANDLE;
    static inline const NativeProcessId invalid_process_id = static_cast<NativeProcessId>(-1);
    static inline const NativeProcessHandle invalid_process_handle = INVALID_HANDLE_VALUE;
#else
    using NativeProcessId = pid_t;
    using NativeProcessHandle = pid_t;
    static inline const NativeProcessId invalid_process_id = -1;
    static inline const NativeProcessHandle invalid_process_handle = -1;
#endif

    class Process final {
        NativeProcessId _pid;
        NativeProcessHandle _handle;

        public:
        Process(const Process& other);
        Process(Process&& other) noexcept;
        Process() noexcept;

        Process(NativeProcessId pid);

        ~Process() noexcept;

        auto operator=(const Process& other) -> Process&;
        auto operator=(Process&& other) noexcept -> Process&;

        [[nodiscard]] static auto get_current() noexcept -> Result<Process>;

        [[nodiscard]] auto get_path() const noexcept -> Result<std::filesystem::path>;

        [[nodiscard]] constexpr auto get_pid() const noexcept -> NativeProcessId {
            return _pid;
        }
    };
}// namespace kstd::platform