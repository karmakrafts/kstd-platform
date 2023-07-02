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

#include <kstd/defaults.hpp>

#include "platform.hpp"

namespace kstd::platform {
    class FileHandle final {
        KSTD_FILE_HANDLE_TYPE _value;

        public:
        KSTD_DEFAULT_MOVE_COPY(FileHandle)

        explicit FileHandle(KSTD_FILE_HANDLE_TYPE value) noexcept :
                _value(value) {
        }

        FileHandle() noexcept :
                _value(KSTD_INVALID_FILE_HANDLE) {
        }

        ~FileHandle() noexcept = default;

        [[nodiscard]] inline operator KSTD_FILE_HANDLE_TYPE() const noexcept {// NOLINT
            return _value;
        }

        [[nodiscard]] inline auto operator==(const FileHandle& other) const noexcept -> bool {
            return _value == other._value;
        }

        [[nodiscard]] inline auto operator!=(const FileHandle& other) const noexcept -> bool {
            return _value != other._value;
        }

        [[nodiscard]] inline auto is_valid() const noexcept -> bool {
#ifdef PLATFORM_WINDOWS
            return _value != nullptr && _value != INVALID_HANDLE_VALUE;
#else
            return _value != -1;
#endif
        }
    };
}// namespace kstd::platform