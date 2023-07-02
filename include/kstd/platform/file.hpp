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

#include <filesystem>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>
#include <kstd/types.hpp>

#include "file_handle.hpp"
#include "platform.hpp"

namespace kstd::platform {
    enum class FileMode : u8 {
        READ,
        WRITE,
        READ_WRITE
    };

    class File final {
        std::filesystem::path _path;
        FileMode _mode;
        FileHandle _handle;

#ifdef PLATFORM_WINDOWS
        SECURITY_ATTRIBUTES _security_attribs {};
#endif

        public:
        KSTD_NO_MOVE_COPY(File)

        explicit File(std::filesystem::path path, FileMode mode) noexcept;

        ~File() noexcept = default;

        [[nodiscard]] auto open() noexcept -> Result<void>;

        [[nodiscard]] auto close() noexcept -> Result<void>;

        [[nodiscard]] auto get_size() const noexcept -> Result<usize>;

        [[nodiscard]] auto resize(usize size) const noexcept -> Result<void>;

        [[nodiscard]] auto set_executable(bool is_executable = true) const noexcept -> Result<void>;

        [[nodiscard]] auto is_executable() const noexcept -> Result<bool>;

        [[nodiscard]] inline auto get_path() const noexcept -> const std::filesystem::path& {
            return _path;
        };

        [[nodiscard]] inline auto get_mode() const noexcept -> FileMode {
            return _mode;
        }

        [[nodiscard]] inline auto get_handle() const noexcept -> FileHandle {
            return _handle;
        }

        [[nodiscard]] inline auto is_open() const noexcept -> bool {
            return _handle.is_valid();
        }

        [[nodiscard]] inline auto is_directory() const noexcept -> bool {
            return std::filesystem::is_directory(_path);
        }

        [[nodiscard]] inline auto exists() const noexcept -> bool {
            return std::filesystem::exists(_path);
        }

#ifdef PLATFORM_WINDOWS

        [[nodiscard]] inline auto get_security_attribs() noexcept -> SECURITY_ATTRIBUTES& {
            return _security_attribs;
        }

#endif
    };
}// namespace kstd::platform