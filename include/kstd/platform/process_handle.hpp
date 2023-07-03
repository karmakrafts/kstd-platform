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

#include <kstd/defaults.hpp>
#include <kstd/types.hpp>

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
#else
    using NativeProcessId = pid_t;
    using NativeProcessHandle = pid_t;
#endif

    class ProcessHandle final {
        NativeProcessHandle _value;

        public:
        KSTD_DEFAULT_MOVE_COPY(ProcessHandle)

        ProcessHandle(NativeProcessHandle value) noexcept :
                _value(value) {
        }

#ifdef PLATFORM_WINDOWS
        ~ProcessHandle() noexcept {
            ::CloseHandle(_value);// On windows, we need to drop the handle ourselves
        }
#else
        ~ProcessHandle() noexcept = default;
#endif

        [[nodiscard]] inline operator NativeProcessHandle() const noexcept {// NOLINT
            return _value;
        }

        [[nodiscard]] inline auto operator==(const ProcessHandle& other) const noexcept -> bool {
            return _value == other._value;
        }

        [[nodiscard]] inline auto operator!=(const ProcessHandle& other) const noexcept -> bool {
            return _value != other._value;
        }
    };
}// namespace kstd::platform