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

namespace kstd::platform::file {
    class FileHandle final {
        NativeFileHandle _value;

        public:
        KSTD_DEFAULT_MOVE_COPY(FileHandle, FileHandle)

        FileHandle(NativeFileHandle value) noexcept :// NOLINT
                _value(value) {
        }

        FileHandle() noexcept :
                _value(invalid_file_handle) {
        }

        ~FileHandle() noexcept = default;

        [[nodiscard]] inline operator NativeFileHandle() const noexcept {// NOLINT
            return _value;
        }

        [[nodiscard]] inline auto operator==(const FileHandle& other) const noexcept -> bool {
            return _value == other._value;
        }

        [[nodiscard]] inline auto operator!=(const FileHandle& other) const noexcept -> bool {
            return _value != other._value;
        }

        [[nodiscard]] inline auto is_valid() const noexcept -> bool {
            return _value != invalid_file_handle;
        }
    };
}// namespace kstd::platform::file